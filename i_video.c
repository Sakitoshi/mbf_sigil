// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_video.c,v 1.3 2000-08-12 21:29:28 fraggle Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// DESCRIPTION:
//      DOOM graphics stuff
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: i_video.c,v 1.3 2000-08-12 21:29:28 fraggle Exp $";

#include "z_zone.h"  // memory allocation wrappers -- killough 
#include <stdio.h>
#include <signal.h>
#include <allegro.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <sys/farptr.h> // GB 2014 for VESA access
#include <go32.h>       // GB 2014 for VESA access
//#include <dos.h>      // GB 2014, not needed here
#include "doomstat.h"
#include "v_video.h"
#include "d_main.h"
#include "m_bbox.h"
#include "st_stuff.h"
#include "m_argv.h"
#include "w_wad.h"
#include "r_draw.h"
#include "am_map.h"
#include "m_menu.h"
#include "wi_stuff.h"
#include "m_misc.h"   // for M_DrawText, GB 2014
#include "i_vgavbe.h" // Graphics routines replacements, GB 2014

/////////////////////////////////////////////////////////////////////////////
//
// JOYSTICK                                                  // phares 4/3/98
//
/////////////////////////////////////////////////////////////////////////////

extern int usejoystick;
extern int joystickpresent;

// fraggle: commented these out for allegro 3.12
#if(ALLEGRO_VERSION>=3 && ALLEGRO_SUB_VERSION==0)
extern int joy_x,joy_y;
extern int joy_b1,joy_b2,joy_b3,joy_b4;
#endif
//void poll_joystick(void);

//-----------------------------------------------------------------------------
// I_JoystickEvents() gathers joystick data and creates an event_t for
// later processing by G_Responder().
void I_JoystickEvents()
{
  event_t event;

  if (!joystickpresent || !usejoystick) return;
  poll_joystick(); // Reads the current joystick settings
  event.type = ev_joystick;
  event.data1 = 0;

  // read the button settings
  if (joy_b1) event.data1 |= 1;
  if (joy_b2) event.data1 |= 2;
  if (joy_b3) event.data1 |= 4;
  if (joy_b4) event.data1 |= 8;

  // Read the x,y settings. Convert to -1 or 0 or +1.
  if      (joy_x < 0) event.data2 = -1;
  else if (joy_x > 0) event.data2 = 1;
  else                event.data2 = 0;
  
  if      (joy_y < 0) event.data3 = -1;
  else if (joy_y > 0) event.data3 = 1;
  else                event.data3 = 0;

  // post what you found
  D_PostEvent(&event);
}

//-----------------------------------------------------------------------------
// I_StartFrame
void I_StartFrame (void)
{
  I_JoystickEvents(); // Obtain joystick data                 phares 4/3/98
}

/////////////////////////////////////////////////////////////////////////////
//
// END JOYSTICK                                              // phares 4/3/98
//
/////////////////////////////////////////////////////////////////////////////

// Keyboard routines, By Lee Killough
// Based only a little bit on Chi's v0.2 code
int I_ScanCode2DoomCode (int a)
{
  switch (a)
    {
    default:   return key_ascii_table[a]>8 ? key_ascii_table[a] : a+0x80;
    case 0x7b: return KEYD_PAUSE;
    case 0x0e: return KEYD_BACKSPACE;
    case 0x48: return KEYD_UPARROW;
    case 0x4d: return KEYD_RIGHTARROW;
    case 0x50: return KEYD_DOWNARROW;
    case 0x4b: return KEYD_LEFTARROW;
    case 0x38: return KEYD_LALT;
    case 0x79: return KEYD_RALT;
    case 0x1d:
    case 0x78: return KEYD_RCTRL;
    case 0x36:
    case 0x2a: return KEYD_RSHIFT;
  }
}

//-----------------------------------------------------------------------------
// Automatic caching inverter, so you don't need to maintain two tables.
// By Lee Killough
int I_DoomCode2ScanCode (int a)
{
  static int inverse[256], cache;
  for (;cache<256;cache++)
    inverse[I_ScanCode2DoomCode(cache)]=cache;
  return inverse[a];
}

//-----------------------------------------------------------------------------
// killough 3/22/98: rewritten to use interrupt-driven keyboard queue
void I_GetEvent()
{
  extern int usemouse;   // killough 10/98
  event_t event;
  int tail;

  while ((tail=keyboard_queue.tail) != keyboard_queue.head)
    {
      int k = keyboard_queue.queue[tail];
      keyboard_queue.tail = (tail+1) & (KQSIZE-1);
      event.type = k & 0x80 ? ev_keyup : ev_keydown;
      event.data1 = I_ScanCode2DoomCode(k & 0x7f);
      D_PostEvent(&event);
    }

  if (mousepresent!=-1 && usemouse) // killough 10/98
    {
      static int lastbuttons;
      int xmickeys,ymickeys,buttons=mouse_b;
      get_mouse_mickeys(&xmickeys,&ymickeys);
      if (xmickeys || ymickeys || buttons!=lastbuttons)
        {
          lastbuttons=buttons;
          event.data1=buttons;
          event.data3=-ymickeys;
          event.data2=xmickeys;
          event.type=ev_mouse;
          D_PostEvent(&event);
        }
    }
}

//-----------------------------------------------------------------------------
void I_StartTic(){ I_GetEvent();}

/////////////////////////////////////////////////////////////////////////////
//
// VIDEO ACCESS                                         // Overhauled GB 2014
//
/////////////////////////////////////////////////////////////////////////////

// GB 2014: Make allegro smaller, no graphics services requires:
DECLARE_GFX_DRIVER_LIST()
DECLARE_COLOR_DEPTH_LIST()

//procedures:
extern void ppro_blit(void *, size_t);
extern void pent_blit(void *, size_t);
extern void blast(void *destin, void *src);             // blits to VGA planar memory
extern void ppro_blast(void *destin, void *src);        // same but for PPro CPU
extern void blast_nobar(void *destin, void *src);       // as above but without doing lower part
extern void ppro_blast_nobar(void *destin, void *src);  // as above but without doing lower part

//variables:
extern boolean setsizeneeded;
boolean noblit;
byte *dascreen;
int show_fps;           // GB 2014
int use_vsync;          // killough 2/8/98: controls whether vsync is called
int page_flip;          // killough 8/15/98: enables page flipping
int hires;
int in_graphics_mode;
int in_page_flip, in_hires, linear;
int scroll_offset;
int blackband; // GB 2014: for 640x480 fallback mode
// GB 2014, FPS counter variables:
char fps_string[12];
char mode_string[128];
int  modeswitched=0;
// Disk icon:
int disk_icon;
//static BITMAP *diskflash, *old_data;


//-----------------------------------------------------------------------------
void I_UpdateNoBlit (void){}

//-----------------------------------------------------------------------------
void devparm_proc(int y)
{
      static int lasttic;
      byte *s = screens[0];
	  int i = I_GetTime();
      int tics = i - lasttic;
      lasttic = i;
      if (tics > 20) tics = 20;
      if (in_hires) { // killough 11/98: hires support, GB 2014: added spacing
          for (i=0 ; i<tics*2 ; i+=2) s[(y-1)*SCREENWIDTH*4+i*2] =
						              s[(y-1)*SCREENWIDTH*4+i*2+1] =
						              s[(y-1)*SCREENWIDTH*4+i*2+SCREENWIDTH*2] =
						              s[(y-1)*SCREENWIDTH*4+i*2+SCREENWIDTH*2+1] = 0xff;
          for (    ; i<20*2   ; i+=2) s[(y-1)*SCREENWIDTH*4+i*2] =
						              s[(y-1)*SCREENWIDTH*4+i*2+1] =
						              s[(y-1)*SCREENWIDTH*4+i*2+SCREENWIDTH*2] =
						              s[(y-1)*SCREENWIDTH*4+i*2+SCREENWIDTH*2+1] = 0x0;
        }
      else { for (i=0 ; i<tics*2 ; i+=2) s[(y-1)*SCREENWIDTH + i] = 0xff;
             for (    ; i<20*2   ; i+=2) s[(y-1)*SCREENWIDTH + i] = 0x0;  }
}

//-----------------------------------------------------------------------------
//GB 2014: New procedure to show video mode and frame rate.  
void show_info_proc()
{
	static int fps_counter, fps_starttime, fps_timeout, fps_nextcalculation, fps;

 	if (modeswitched>0) // display video driver after switch
    {
       M_DrawText2(1,0,CR_BLUE,true,mode_string); 
	   if (modeswitched==1)
	   {
	     fps_timeout=I_GetTime()+200; // I_GetTime_RealTime(); same result
         fps_counter=0; // fps counter
	     fps=-1;
		 fps_nextcalculation=-1;
		 modeswitched++;
	   }
       if (I_GetTime()>fps_timeout) modeswitched=0; 
    } 
 	if (show_fps)
 	{
	   static char c,i,cr;
	   int time=I_GetTime(); // I_GetTime_RealTime(); same result
	   if (fps_counter==0) fps_starttime = I_GetTime(); 
       fps_counter++;
       // store a value and/or draw when data is ok:
       if (fps_counter>(TICRATE+10)) 
	   {  
		  if (fps_nextcalculation<time) // in case of a very fast system, this will limit the sampling
		  {
		    fps=(double)((fps_counter-1)*TICRATE)/(time-fps_starttime); // minus 1!, exactly 35 FPS when measeraring for a longer time.
			fps_nextcalculation=time+12; 
		    if (fps>999999) fps=999999; // overflow
            sprintf(fps_string,"%6d",fps);//"FPS:%5d",fps); 

    	         if (fps<10)     {i=0; c=0; }
    	    else if (fps<100)    {i=1; c=0; } 
    	    else if (fps<1000)   {i=2; c=5; } 
    	    else if (fps<10000)  {i=3; c=10;} 
    	    else if (fps<100000) {i=4; c=15;} 
    	    else                 {i=5; c=20;} 

				 if (fps>60)   cr=CR_BLUE;
    		else if (fps>34)   cr=CR_GREEN;
		    else if (fps>20)   cr=CR_GOLD; // CR_ORANGE+CR_YELLOW are like white
			else               cr=CR_RED;

			fps_counter=0; // flush old data
		  }
	   }
       V_DrawRect(0, 309-c, 0, 319, 6, 0x00); 
       if (fps>-1) M_DrawText2(295-i,0,cr,true,fps_string);
	}
}

//-----------------------------------------------------------------------------
void I_FinishUpdate(void)
{ 

   //I_GetTime_RealTime();
  // int a=I_GetTime_RealTime();
  // int b=I_GetTime_RealTime(); 
  // int c=I_GetTime_RealTime();
  // int d=I_GetTime_RealTime();

   int ymax=200, size;
   if (noblit || !in_graphics_mode) return;

 //if (v12_compat)    M_DrawText2(1,10,CR_BLUE,true,"V12_COMPAT");   // debug
 //if (compatibility) M_DrawText2(1,16,CR_BLUE,true,"compatibility");// debug 
 //sprintf(mode_string,"vesa:%d mode:%xh w:%d h:%d banksize:%d BPS:%d",vesa_version,current_mode, screen_w, screen_h, mode_banksize, mode_BPS);
 //sprintf(mode_string,"vesa:%d modehi:%d modehi2:%d modelow:%d",vesa_version, vesa_mode_640x400, vesa_mode_640x480, vesa_mode_320x200);
 //M_DrawText2(1,0,CR_BLUE,true,mode_string); 

   //sprintf(mode_string,"%d",realtic);
   //M_DrawText2(1,0,CR_BLUE,true,mode_string); 

   // GB 2014, FPS counter logic:
   if (show_fps || safeparm) show_info_proc();

   // GB 2014, first part of code to check if statusbar needs to be (re)drawn on screen:
   if ((viewactive || automapactive) && scaledviewheight<200 && !inhelpscreens) ymax=168; 

   // draws little dots on the bottom of the screen:
   if (devparm) devparm_proc(ymax);

   // GB 2014, code to check if statusbar needs to be drawn (e.g. has anything changed, e.g. is dirty?): 
   if (statusbar_dirty>0) {ymax=200; statusbar_dirty--; if (!in_page_flip) statusbar_dirty=0;}
   size = in_hires ? SCREENWIDTH*ymax*4 : SCREENWIDTH*ymax; 

   if (in_page_flip)
      if (!in_hires && (current_mode<256)) // Transfer from system memory to planar 'mode X' video memory:
      {
         if (screen_base_addr==0) // GB 2014, switch between two pages, not four.
         {  // killough 8/15/98: 320x200 Wait-free page-flipping for flicker-free display
            screen_base_addr += 0x4000;             // Move address up one page
            screen_base_addr &= 0xffff;             // Reduce address mod 4 pages
            // Pentium Pros and above need special consideration in the planar multiplexing code, to avoid partial stalls. killough
         } else screen_base_addr=0;
		 dascreen=(byte *) __djgpp_conventional_base + 0xA0000 + screen_base_addr;
         #ifdef CALT // GB 2014: option for C variant of blast function
         if (noasmxparm)                                   blast_C         (dascreen, *screens, ymax);
         else 
         #endif // CALT
         if (cpu_family >= 6 || asmp6parm) {if (ymax==200) ppro_blast      (dascreen, *screens);  // PPro, PII
                                            else           ppro_blast_nobar(dascreen, *screens);}
         else                              {if (ymax==200) blast           (dascreen, *screens);  // Other CPUs, e.g. 486
                                            else           blast_nobar     (dascreen, *screens);}
         outportw(0x3d4, screen_base_addr | 0x0c);              // page flip 
         return;
      } 
      else scroll_offset = scroll_offset ? 0 : screen_h;        // hires hardware page-flipping (VBE 2.0)
   // killough 8/15/98: no page flipping; use other methods:
   else if (use_vsync && !timingdemo && current_mode<256) wait_vsync(); // killough 2/7/98: use vsync() to prevent screen breaks.

   if (!linear || safeparm) 
   {                                                            // note: divide scroll offset /2 to test if pageflipping occurs
      if (current_mode>255) vesa_blitscreen_banked(screens[0], size, scroll_offset); // VESA Banked (slower)
	  else dosmemput(screens[0], SCREENWIDTH*ymax, 0xA0000);    // Mode 13h without nearptr
   }
   else
   {  // 1/16/98 killough: optimization based on CPU type:
      if (current_mode>255) 
    	 dascreen = (byte *) screen_base_addr + scroll_offset*mode_BPS + blackband*mode_BPS; // VESA LFB access
           if (cpu_family >= 6) ppro_blit(dascreen,size);       // PPro, PII
      else if (cpu_family >= 5) pent_blit(dascreen,size);       // Pentium (GB 2014: not used for Cx5x86, but it is a little slower anyways)
      else                      memcpy(dascreen,screens[0],size); // Others
   }

   if (in_page_flip) vesa_set_displaystart(0, scroll_offset, use_vsync); // hires hardware page-flipping (VBE 2.0 Only, Do not waste frames on 1.2)
}

//-----------------------------------------------------------------------------
// I_ReadScreen
void I_ReadScreen(byte *scr)
{
  int size = hires ? SCREENWIDTH*SCREENHEIGHT*4 : SCREENWIDTH*SCREENHEIGHT;
  // 1/18/98 killough: optimized based on CPU type:
       if (cpu_family >= 6) ppro_blit(scr,size); // PPro or PII
  else if (cpu_family >= 5) pent_blit(scr,size); // Pentium
  else    memcpy(scr,*screens,size);             // Others
}

//-----------------------------------------------------------------------------
// killough 10/98: init disk icon
// GB had to disable for now, since allegro drawing routines are no longer used.
// and the new libraries lack a universal read-from and blit-to screen procedure.
static void I_InitDiskFlash(void)
{
/*
  byte temp[32*32];

  if (diskflash)
    {
      destroy_bitmap(diskflash);
      destroy_bitmap(old_data);
    }

  diskflash = create_bitmap_ex(8, 16<<hires, 16<<hires);
  old_data = create_bitmap_ex(8, 16<<hires, 16<<hires);

  V_GetBlock(0, 0, 0, 16, 16, temp);
  V_DrawPatchDirect(0, 0, 0, W_CacheLumpName(M_CheckParm("-cdrom") ?
                                             "STCDROM" : "STDISK", PU_CACHE));
  V_GetBlock(0, 0, 0, 16, 16, diskflash->line[0]);
  V_DrawBlock(0, 0, 0, 16, 16, temp);
*/
}

//-----------------------------------------------------------------------------
// killough 10/98: draw disk icon
void I_BeginRead(void)
{/*
  if (!disk_icon || !in_graphics_mode || noblit || safeparm) // GB 2014: added noblit
    return;

  blit(screen, old_data,
       (SCREENWIDTH-16) << hires,
       scroll_offset + ((SCREENHEIGHT-16)<<hires),
       0, 0, 16 << hires, 16 << hires);

  blit(diskflash, screen, 0, 0, (SCREENWIDTH-16) << hires,
       scroll_offset + ((SCREENHEIGHT-16)<<hires), 16 << hires, 16 << hires);
*/
}

//-----------------------------------------------------------------------------
// killough 10/98: erase disk icon
void I_EndRead(void)
{/*
  if (!disk_icon || !in_graphics_mode || noblit || safeparm) // GB 2014: added noblit
    return;

  blit(old_data, screen, 0, 0, (SCREENWIDTH-16) << hires,
       scroll_offset + ((SCREENHEIGHT-16)<<hires), 16 << hires, 16 << hires);
*/
}

//-----------------------------------------------------------------------------
void I_SetPalette(byte *palette)
{
  int i;

  if (!in_graphics_mode)             // killough 8/11/98
    return;

  if (!timingdemo)
    while (!(inportb(0x3da) & 8));

  outportb(0x3c8,0);
  for (i=0;i<256;i++)
    {
      outportb(0x3c9,gammatable[usegamma][*palette++]>>2);
      outportb(0x3c9,gammatable[usegamma][*palette++]>>2);
      outportb(0x3c9,gammatable[usegamma][*palette++]>>2);
    }

}

//-----------------------------------------------------------------------------
void I_ShutdownGraphics(void)
{
   if (in_graphics_mode)
   {
      //if (!(safeparm && hires)) clear(screen);
      //set_gfx_mode(GFX_TEXT, 0, 0, 0, 0); // Turn off graphics mode
	  set_mode_text();
      in_graphics_mode = 0;
	  vesa_get_screen_base_addr(1); // free mapping, when set before
	  get_vesa_pm_functions(1);
   }
}

//-----------------------------------------------------------------------------
// killough 11/98: New routine, for setting hires and page flipping
// GB 2014: Mostly Rewritten, replaced all allegro graphics dependencies
static void I_InitGraphicsMode(void)
{
  int hiresfail=0;
  char safestring[]="SAFE MODE / ";     
  if (!safeparm) safestring[0]='\0';
  blackband = 0; 

  // PREPARATION AT FIRST RUN
  if (!in_graphics_mode) 
  {
	 in_hires=0;
	 if (vesa_version=-1) // We have not checked it yet
	 {
	    vesa_get_info();  // Get vesa availability and vesa_version; 
        // Look for VESA 320x200; at an unpredictable number, but only with VBE 2.0 services is it worthwhile
		// Also see if 640x400 is there, and 640x480, better do this properly...
	    if (vesa_version>=1) if (vesa_find_modes(safeparm || nolfbparm || vesa_version<2)==0) 
			{vesa_mode_640x400=0x100; vesa_mode_640x480=0x101;} // zero found = bad BIOS?
		// Note: it will set (mode_number | 0x4000) for LFB, when supported. 
        if ((vesa_version>=2) && (!nopmparm)) get_vesa_pm_functions(0);
	 }
	 //vesa_mode_320x200=0; // debugging
  }
 
  // HIGH RESOLUTION - 640x400 or 640x480
  if (hires && !in_hires)  
  {  // GB 2014: Used to just try mode 100h and then 101h, but intel graphics gives trouble if 100h was tried first.
	 if (vesa_version<1) {hiresfail=1;}
	 else if (vesa_mode_640x400>0) 
     {
        if (vesa_set_mode(vesa_mode_640x400)!=-1)      // Init 640x400
		{                       
  		  if (current_mode!=current_mode_info) vesa_get_mode_info(current_mode); 
		  screen_w=640; // Necessary for when mode 13h/X has overwritten them.
 		  screen_h=400;
	 	}
		else hiresfail=1;
     }
	 else if (vesa_mode_640x480>0) 
     {
		if (vesa_set_mode(vesa_mode_640x480)!=-1)      // Init 640x400
		{
  		  if (current_mode!=current_mode_info) vesa_get_mode_info(current_mode); 
		  screen_w=640; // Necessary for when mode 13h/X has overwritten them.
 		  screen_h=480;
 		  blackband=40;                     // color 8 is almost black, zero is black
		}
		else hiresfail=1;
	 }
     else hiresfail=1; 

     if (hiresfail)
     {
		hires = 0;                    // Revert to lowres
        warn_about_changes(S_BADVID); // shows "video mode not supported" at the bottom
        I_InitGraphicsMode();         // Start all over
        return;
     }
  }

  // NORMAL RESOLUTION - 320x200 modes, three options:
  if (!hires) 
  {
	 if (!in_hires && current_mode>255) {}            // Already in VESA 320x200, just page-flip toggle
	 else if (vesa_mode_320x200>0)  
	 {
        if (vesa_set_mode(vesa_mode_320x200)!=-1)
		{
		   if (current_mode!=current_mode_info) vesa_get_mode_info(current_mode);  // Init VESA 320x200
 		   screen_w=320; // Necessary for when mode 13h/X has overwritten it.
		   screen_h=200;
	    }
	    else if (current_mode!=0x13) set_mode_13h();  // Init Mode 13h
	 } 
	 else if (page_flip)                              // Init Mode X
     {                                                // No Mode-X in Windows NT Style OS, check safeparm
        if (!safeparm)
		{
		   if (current_mode!=0x12) set_mode_X();
  	       linear=false; 
		   screen_base_addr=0;
	    }
	    else                                          // Init Mode 13h
	    {                                             // Just one page, Not really VESA LFB, but still linear
           warn_about_changes(S_BADOPT);
           if (current_mode!=0x13) set_mode_13h(); 
	    }
	 }
	 else if (current_mode!=0x13) set_mode_13h();     // Init Mode 13h
   
	 if (current_mode==0x13)                          // Finish Mode 13h
     {
		page_flip=0;                                  
	    linear=true;                                  // Just one page, Not really VESA LFB, but still linear
        dascreen = (byte *) __djgpp_conventional_base + 0xA0000;
	 }
  }

  // CONTINUE TO PREPARE VESA MODES
  if (current_mode>255) 
  {
     int status=0;
	 // Setup Page flipping, optionally combined with Vsync:
     if (use_vsync && !page_flip)      {use_vsync=0; warn_about_changes(S_BADOPT);}
	 if (vesa_version<2 && use_vsync)  {use_vsync=0; warn_about_changes(S_BADOPT);}
	 if (vesa_version<2 && page_flip)  {page_flip=0; warn_about_changes(S_BADOPT);}  // won't work, would just be wasting frames
	 if (vesa_version>=2)
	 {
     	if (in_page_flip && !page_flip) vesa_set_displaystart(0,0,0); // we may still be at the second page, from before
		else if (!in_page_flip && page_flip)                          // we need to check success.                         
		{
		   if (page_flip)  status=vesa_set_displaystart(4,0,0);       // shake screen a little to verify. Voodoo 3: keep too multiples of 4, else fail.
	       if (status==1) {page_flip=use_vsync=0; warn_about_changes(S_BADOPT); } // function failed, cancel use
     	   rest(5);                                                               // show screen shake 
		   vesa_set_displaystart(0,0,0);                                          // reset screen shake, even if fail.
		}
     }
	 // Setup LFB access if available:
	 if (safeparm || nolfbparm || vesa_version<2) linear=false; else linear=mode_LFB; // LFB wanted? and supported?
 	 if (linear) {if (vesa_get_screen_base_addr(0)==1) linear=false;} // Get LFB base address, should be available
     if (blackband &&  linear) vesa_clear_pages_LFB   (2, 0x08);      // may be garbage left in video memory, 
     if (blackband && !linear) vesa_clear_pages_banked(2, 0x08);      // which will otherwise be visible in the bars.
  }

  // CONTINUE TO PREPARE GENERAL STUFF
  if (!in_graphics_mode || hires!=in_hires) V_Init(); // required buffer size has changed
  scroll_offset = 0;
  in_graphics_mode = 1;
  in_page_flip = page_flip;
  in_hires = hires;
  setsizeneeded = true;
  //if (!safeparm) I_InitDiskFlash(); // Initialize disk icon
  I_SetPalette(W_CacheLumpName("PLAYPAL",PU_CACHE));
  modeswitched=1; 
  if (current_mode==0x12) sprintf(mode_string,"%sMODE:X SIZE:%dx%d LFB:%s CPU:%d VBE:%d",  safestring,               screen_w, screen_h, linear ? "true" : "false", cpu_family, vesa_version);  
  else                    sprintf(mode_string,"%sMODE:%xh SIZE:%dx%d LFB:%s CPU:%d VBE:%d",safestring, current_mode, screen_w, screen_h, linear ? "true" : "false", cpu_family, vesa_version); 
}

//-----------------------------------------------------------------------------
void I_ResetScreen(void)
{
  if (!in_graphics_mode)
  {
     setsizeneeded = true;
     V_Init();
     return;
  }

  //I_ShutdownGraphics(); // Switch out of old graphics mode // GB 2014, not necessary

  I_InitGraphicsMode();     // Switch to new graphics mode

  if (automapactive) AM_Start();  // Reset automap dimensions

  ST_Start();               // Reset palette

  if (gamestate == GS_INTERMISSION)
    {
      WI_DrawBackground();
      V_CopyRect(0, 0, 1, SCREENWIDTH, SCREENHEIGHT, 0, 0, 0);
    }

  Z_CheckHeap();
}

//-----------------------------------------------------------------------------
void I_InitGraphics(void)
{
  static int firsttime=1;

  if (!firsttime) return;

  firsttime=0;

  check_cpu();    // 1/16/98 killough -- sets cpu_family based on CPU

#ifndef RANGECHECK
  asm("fninit");  // 1/16/98 killough -- prevents FPU exceptions
#endif

  timer_simulate_retrace(0);

  if (nodrawers)  return; // killough 3/2/98: possibly avoid gfx mode
  
  // enter graphics mode:

  atexit(I_ShutdownGraphics);

  signal(SIGINT, SIG_IGN);  // ignore CTRL-C in graphics mode

  in_page_flip = page_flip;

  I_InitGraphicsMode();    // killough 10/98

  Z_CheckHeap();
}

//----------------------------------------------------------------------------
//
// $Log: i_video.c,v $
// Revision 1.3  2000-08-12 21:29:28  fraggle
// change license header
//
// Revision 1.2  2000/07/29 22:48:23  fraggle
// fix for allegro v3.12
//
// Revision 1.1.1.1  2000/07/29 13:20:39  fraggle
// imported sources
//
// Revision 1.12  1998/05/03  22:40:35  killough
// beautification
//
// Revision 1.11  1998/04/05  00:50:53  phares
// Joystick support, Main Menu re-ordering
//
// Revision 1.10  1998/03/23  03:16:10  killough
// Change to use interrupt-driver keyboard IO
//
// Revision 1.9  1998/03/09  07:13:35  killough
// Allow CTRL-BRK during game init
//
// Revision 1.8  1998/03/02  11:32:22  killough
// Add pentium blit case, make -nodraw work totally
//
// Revision 1.7  1998/02/23  04:29:09  killough
// BLIT tuning
//
// Revision 1.6  1998/02/09  03:01:20  killough
// Add vsync for flicker-free blits
//
// Revision 1.5  1998/02/03  01:33:01  stan
// Moved __djgpp_nearptr_enable() call from I_video.c to i_main.c
//
// Revision 1.4  1998/02/02  13:33:30  killough
// Add support for -noblit
//
// Revision 1.3  1998/01/26  19:23:31  phares
// First rev with no ^Ms
//
// Revision 1.2  1998/01/26  05:59:14  killough
// New PPro blit routine
//
// Revision 1.1.1.1  1998/01/19  14:02:50  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
