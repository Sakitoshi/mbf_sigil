//
//                        -= I_VGAVBE =-                    
//
// G. Broers 10-2014 
// http://members.quicknet.nl/lm.broers/        
//
// Based on examples found on the DJGPP site:  http://www.delorie.com/djgpp/
// 
// The video options now featured in MBF are:
//
// Lowres:
// Mode 13h, 320x200, no pageflipping, the most basic mode
// Mode X,   320x200, for pageflipping on older systems
// VESA 320x200, supports pageflipping only on VBE 2.0. 
// (This mode is only looked for when VBE 2.0 is detected, and replaces 13h and X)
//
// Hires:
// VESA 640x400, supports pageflipping only on VBE 2.0. 
// VESA 640x480, fallback mode when 640x400 is not available
//
// The VESA modes are accessed either in banked mode (=slower, most compatible) 
// or in Linear FrameBuffer mode (=fastest)
// The banked mode DPMI functions are not yet implemented, irq 10 is used. 
// Also the page flipping in LFB mode is done with irq 10, thus a switch to realmode.
//
// Compatibility was tested on 5 systems plus 2 emulators; no problems.
// The Allegro graphics routines are no longer necessary.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>      // for malloc+free
#include <stdio.h>
#include <go32.h>
#include <dpmi.h>
#include <dos.h>         // for outportb/w
#include <string.h>      // for memset
#include <sys/nearptr.h> // for __djgpp_conventional_base
#include <sys/farptr.h>  // for farpoke
#include "i_vgavbe.h"

//-----------------------------------------------------------------------------
typedef struct VESA_INFO
{
   unsigned char  VESASignature[4];   //__attribute__ ((packed));
   unsigned short VESAVersion           __attribute__ ((packed));
   unsigned long  OEMStringPtr          __attribute__ ((packed));
   unsigned char  Capabilities[4];    //__attribute__ ((packed));
   unsigned long  VideoModePtr          __attribute__ ((packed));
   unsigned short TotalMemory           __attribute__ ((packed));
   unsigned short OemSoftwareRev        __attribute__ ((packed));
   unsigned long  OemVendorNamePtr      __attribute__ ((packed));
   unsigned long  OemProductNamePtr     __attribute__ ((packed));
   unsigned long  OemProductRevPtr      __attribute__ ((packed));
   unsigned char  Reserved[222];      //__attribute__ ((packed));
   unsigned char  OemData[256];       //__attribute__ ((packed));
}VESA_INFO;

typedef struct MODE_INFO
{             // GB 2014, packed is redundant for unsigned char.
   unsigned short ModeAttributes        __attribute__ ((packed));
   unsigned char  WinAAttributes;     //__attribute__ ((packed)); 
   unsigned char  WinBAttributes;     //__attribute__ ((packed));
   unsigned short WinGranularity        __attribute__ ((packed));
   unsigned short WinSize               __attribute__ ((packed));
   unsigned short WinASegment           __attribute__ ((packed));
   unsigned short WinBSegment           __attribute__ ((packed));
   unsigned long  WinFuncPtr            __attribute__ ((packed));
   unsigned short BytesPerScanLine      __attribute__ ((packed));
   unsigned short XResolution           __attribute__ ((packed));
   unsigned short YResolution           __attribute__ ((packed));
   unsigned char  XCharSize;          //__attribute__ ((packed));
   unsigned char  YCharSize;          //__attribute__ ((packed));
   unsigned char  NumberOfPlanes;     //__attribute__ ((packed));
   unsigned char  BitsPerPixel;       //__attribute__ ((packed));
   unsigned char  NumberOfBanks;      //__attribute__ ((packed));
   unsigned char  MemoryModel;        //__attribute__ ((packed));
   unsigned char  BankSize;           //__attribute__ ((packed));
   unsigned char  NumberOfImagePages; //__attribute__ ((packed));
   unsigned char  Reserved_page;      //__attribute__ ((packed));
   unsigned char  RedMaskSize;        //__attribute__ ((packed));
   unsigned char  RedMaskPos;         //__attribute__ ((packed));
   unsigned char  GreenMaskSize;      //__attribute__ ((packed));
   unsigned char  GreenMaskPos;       //__attribute__ ((packed));
   unsigned char  BlueMaskSize;       //__attribute__ ((packed));
   unsigned char  BlueMaskPos;        //__attribute__ ((packed));
   unsigned char  ReservedMaskSize;   //__attribute__ ((packed));
   unsigned char  ReservedMaskPos;    //__attribute__ ((packed));
   unsigned char  DirectColorModeInfo;//__attribute__ ((packed));
   unsigned long  PhysBasePtr           __attribute__ ((packed));
   unsigned long  OffScreenMemOffset    __attribute__ ((packed));
   unsigned short OffScreenMemSize      __attribute__ ((packed));
   unsigned char  Reserved[206];      //__attribute__ ((packed));
}MODE_INFO;

typedef struct VESA_PM_INFO
{
   unsigned short setWindow            __attribute__ ((packed)); // = set_bank
   unsigned short setDisplayStart      __attribute__ ((packed));
   unsigned short setPalette           __attribute__ ((packed));
   unsigned short IOPrivInfo           __attribute__ ((packed));
} VESA_PM_INFO;

VESA_INFO vesa_info;
MODE_INFO mode_info;
VESA_PM_INFO *vesa_pm_info;
void *pm_bank_switcher; 
void *pm_display_scroller; 
//int _mmio_segment = 0;
//static unsigned long mmio_linear = 0;     /* linear addr for mem mapped IO */

//int selector;
int current_bank=-1;
int current_mode=-1;
int current_mode_info=-1;
int screen_w=-1;
int screen_h=-1;
int mode_LFB=-1;
int mode_BPS=-1;
int mode_banksize=-1;
int vesa_version=-1; // initially -1, then 0 if fail, in case of success: 1 2 or 3
int vesa_mode_320x200=0;
int vesa_mode_640x400=0;
int vesa_mode_640x480=0;
int vesa_PM_enable=0;
unsigned long mode_LFB_PTR=0;
unsigned char backingbuf[16]; // for both blast_C and clear_banks
unsigned long screen_base_addr=0; // not set here..
int screen_base_addr_set=0;
__dpmi_meminfo mapping; 

//-----------------------------------------------------------------------------

// vga_vsync, from allegro 3.0
void wait_vsync()
{
   do {} while (inportb(0x3DA) & 8);
// if (_timer_use_retrace) 
// {
//      int t = retrace_count; 
//       do {} while (t == retrace_count);
//   }
// else 
   do { } while (!(inportb(0x3DA) & 8));  
}

//-----------------------------------------------------------------------------
void set_mode_13h()
{
   __dpmi_regs r;

   r.x.ax = 0x13;
   __dpmi_int(0x10, &r);

   screen_w=320;
   screen_h=200;
   current_mode=0x13;
}

//-----------------------------------------------------------------------------
void set_mode_text()
{
   __dpmi_regs r;

   r.x.ax = 3;
   __dpmi_int(0x10, &r);
  
   screen_w=-1;
   screen_h=-1;
   current_mode=-1;
}

//-----------------------------------------------------------------------------
// Blit a 320x200 buffer to a Mode X planar screen. This is no VESA thing.
// 16 in a row, in this setup it is just a tad faster then 8. 
#define SC_INDEX      0x03c4
#define SC_DATA       0x03c5
#define CRTC_INDEX    0x03d4
#define CRTC_DATA     0x03d5
#define MEMORY_MODE   0x04
#define UNDERLINE_LOC 0x14
#define MODE_CONTROL  0x17
#define GC_INDEX 	  0x03CE
#define GC_DATA       0x03CF

#ifdef CALT // GB 2014, C routine for blitting to 320x200 mode-X planar screen:
void blast_C(void *dest, unsigned char *src, int ymax)
{
// int pixel, count;
   unsigned char *buf=backingbuf;
   int c, plane;
   ymax=ymax*5;
   for (plane=0;plane<4;plane++) 
   {                                      // four planes in mode-X
      outportb(SC_INDEX, 0x02);           // write plane enable
      outportb(SC_DATA,  0x01 << plane);  // select plane 
      for (c=0;c<ymax;c++)                // go downwards
	  {                             
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4; buf++;
         *buf=*src; src+=4;
//       for(pixel=(c*64),count=0; pixel<(c*64+4*16); pixel+=4,count++) backingbuf[count]=src2[pixel]; //little slower, faster with 8 in a row
         memcpy(dest+c*16,buf=backingbuf,16); // transfer to video mem
      }
      src+=(-64*ymax+1); // reset back to start, with plane as offset (1, 2 or 3)
   }
}   
#endif // CALT

//-----------------------------------------------------------------------------
// Mode X - Credits: brackeen.com / killough / GB 2014
// Original Doom used Mode X, but drew directly to video memory.
void set_mode_X()
{
   int c;
   void *dest= 0xa0000 + (unsigned char *) __djgpp_conventional_base;
   if (current_mode!=0x13) set_mode_13h();

   outportb(SC_INDEX, 4);                // turn off chain 4 and odd/even  
   outportb(SC_DATA, (inportb(SC_DATA) & ~8) | 4); 

   outportb(GC_INDEX, 5);                // turn off odd/even and set write mode 0
   outportb(GC_DATA, inportb(GC_DATA) & ~0x13);

   outportb(GC_INDEX, 6);                // turn off chain 4
   outportb(GC_DATA, inportb(GC_DATA) & ~2); 

   outportb(SC_INDEX, 0x02);             // write plane enable
   outportb(SC_DATA,  0xFF);             // select all planes, must be FF! (F0 is just half)

   memset(backingbuf,0,16);              // color zero
   for (c=0;c<4000;c++) memcpy(dest+c*16,backingbuf,16); // the 4 pages of a 1000x16 bytes

   outportb(CRTC_INDEX, UNDERLINE_LOC);  // turn off long mode
   outportb(CRTC_DATA, 0x00);

   outportb(CRTC_INDEX, MODE_CONTROL);   // turn on byte mode
   outportb(CRTC_DATA, 0xe3);

   current_mode=0x12;                    // lets label it like the 16 color mode
   //rest(1000);                         // debug to check if screen gets cleared
}

//-----------------------------------------------------------------------------
/*
int vesa_get_mode() 
{
   __dpmi_regs r;  
   
   r.x.ax = 0x4F03;
   __dpmi_int(0x10, &r);
   if (r.h.ah) return -1;
   current_mode=r.x.bx & 0x1FF; // GB 2014: remove attributes
   return current_mode;
}
*/

//-----------------------------------------------------------------------------
int get_vesa_pm_functions(int close)
{
   __dpmi_regs r;
   if (vesa_version < 2) return -1;    
   if (close) 
   {
      free(vesa_pm_info);            // may be NULL, ignored by free()
	  return 0;
   }
   r.x.ax = 0x4F0A;
   r.x.bx = 0;
   __dpmi_int(0x10, &r);             // call the VESA function 
   if (r.h.ah) return -1;
   vesa_pm_info = malloc(r.x.cx);    // allocate space for the code stubs 
   dosmemget(r.x.es*16+r.x.di, r.x.cx, vesa_pm_info); // copy the code into our address space 

   pm_bank_switcher    = (void *)((char *)vesa_pm_info + vesa_pm_info->setWindow);       // store a pointer to the bank switch routine 
   pm_display_scroller = (void *)((char *)vesa_pm_info + vesa_pm_info->setDisplayStart); // store a pointer to the bank switch routine 
   // optionally add pallette routine too... but currently it is VGA outport based, which is acceptable in speed.
   vesa_PM_enable=1;
   return 0;
}

//-----------------------------------------------------------------------------
// GB 2014: rough setup, works but need to review this:
int vesa_set_displaystart(unsigned long pixel, unsigned long scanline, unsigned int waitVRT)
{ 
   waitVRT = waitVRT ? 0x0080 : 0x0000; // Wait verticle retrace, yes or no.
   if (vesa_PM_enable)
   {
     long offset = (pixel + scanline * mode_BPS) / 4; 
//	 int seg = selector ? selector : _my_ds(); // GB 2014, seems to work with just _my_ds? and even zero? need to clear this up
	 int seg = _my_ds(); 
	 asm (                            // from allegro 3.0
	    "  pushl %%ebp ; "
	    "  pushw %%es ; "
	    "  movw %w1, %%es ; "         // set the IO segment 
	    "  call *%0 ; "               // call the VESA function 
	    "  popw %%es ; "
	    "  popl %%ebp ; "
	 :                                // no outputs 
	 : "S" (pm_display_scroller),     // ESI function pointer in 
	   "a" (seg),                     // EAX, IO segment  
	   "b" (waitVRT),                 // EBX, mode 
	   "c" (offset&0xFFFF),           // ECX, low word of address 
	   "d" (offset>>16)               // EDX, high word of address 
	 : "memory", "%edi", "%cc"        // clobbers edi and flags 
	 );
   }
   else
   {
      __dpmi_regs r;
      if (current_mode<255) return 1; // not in a VESA mode: fail
      r.x.ax = 0x4F07;
      r.x.bx = waitVRT; 
      r.x.cx = pixel;      
      r.x.dx = scanline;   
      __dpmi_int(0x10, &r);
      if (r.h.ah==0x01 || r.h.al!=0x4F) return 1; // fail   // Status, goes for all VESA functions:  
   }			  									    	// AL == 4Fh: Function is supported      
   return 0;											    // AL != 4Fh: Function is not supported  
}     												    	// AH == 00h: Function call successful   
   													    	// AH == 01h: Function call failed       

//-----------------------------------------------------------------------------
void vesa_set_bank(int bank_number) 
{
   if (current_bank==bank_number) return; // already there
   current_bank=bank_number;
   if (vesa_PM_enable)
   {
      asm (
      " call *%0 "
      :                               // no outputs 
      : "r" (pm_bank_switcher),       // function pointer in any register 
        "b" (0),                      // set %ebx to zero 
        "d" (bank_number)             // bank number in %edx 
      :   "%eax",                     // clobber list 
          "%ecx",                   
          "%esi",                   
          "%edi"  // GB 2014, removal of registers from the clobber list that appear in the input list too.
      );
   }
   else
   {
      __dpmi_regs r;
      r.x.ax = 0x4F05;
      r.x.bx = 0;
      r.x.dx = bank_number;
      __dpmi_int(0x10, &r);
   }
}

//-----------------------------------------------------------------------------
int vesa_set_mode(int mode)
{
   __dpmi_regs r;
   if (mode<255) return 1;

   r.x.ax = 0x4F02;
   r.x.bx = mode;
   __dpmi_int(0x10, &r);
   if (r.h.ah) return -1;     // failure!
   current_mode=mode & 0x1FF; // GB 2014: remove attributes
   current_bank=-1;           // reset;
   return 0;
}

//-----------------------------------------------------------------------------
int vesa_get_info()
{
   __dpmi_regs r;
   long dosbuf;
   int c;
   
   dosbuf = __tb & 0xFFFFF;
   for (c=0; c<sizeof(VESA_INFO); c++)
     _farpokeb(_dos_ds, dosbuf+c, 0);
   dosmemput("VBE2", 4, dosbuf);
   r.x.ax = 0x4F00;
   r.x.di = dosbuf & 0xF;
   r.x.es = (dosbuf>>4) & 0xFFFF;
   __dpmi_int(0x10, &r);
   if (r.h.ah) return -1; //failure!
   dosmemget(dosbuf, sizeof(VESA_INFO), &vesa_info);
   if (strncmp(vesa_info.VESASignature, "VESA", 4) != 0) 
   {
     vesa_version=0; // fail, no vesa
     return -1;
   }
   vesa_version=vesa_info.VESAVersion>>8; 
   return 0;
}

//-----------------------------------------------------------------------------
int vesa_get_mode_info(int mode)
{
   __dpmi_regs r;
   long dosbuf;
   int c;

   dosbuf = __tb & 0xFFFFF;
   for (c=0; c<sizeof(MODE_INFO); c++)
     _farpokeb(_dos_ds, dosbuf+c, 0);
   r.x.ax = 0x4F01;
   r.x.di = dosbuf & 0xF;
   r.x.es = (dosbuf>>4) & 0xFFFF;
   r.x.cx = mode;
   __dpmi_int(0x10, &r);
   if (r.h.ah) return 0; //failure!
   dosmemget(dosbuf, sizeof(MODE_INFO), &mode_info);

   // GB 2014, share some important info:
   current_bank=-1;
   current_mode_info=mode;
   screen_w=mode_info.XResolution;
   screen_h=mode_info.YResolution;
   mode_BPS=mode_info.BytesPerScanLine;
   mode_banksize=mode_info.WinGranularity*1024;
   if (mode_info.ModeAttributes & 128) mode_LFB=1; else mode_LFB=0;
   mode_LFB_PTR=mode_info.PhysBasePtr;

   return 1;
}

//-----------------------------------------------------------------------------
int vesa_get_screen_base_addr(int close)
{
   if (screen_base_addr_set > 0)
      __dpmi_free_physical_address_mapping(&mapping); 
   if (close) return 0; // just cleanup

   if (mode_LFB_PTR==0) return 1;  // fail
   mapping.address = mode_LFB_PTR;
   mapping.size = mode_BPS*screen_h*2; // Normally set to: vesa_info.TotalMemory << 16;
   if (__dpmi_physical_address_mapping(&mapping) != 0) return 1; // fail, back to banked mode
     else screen_base_addr=mapping.address + __djgpp_conventional_base;
   screen_base_addr_set=1;

   // GB 2014: for PM displaystart, need to clear this up?:
   //__dpmi_set_segment_base_address(selector, mapping.address);
   //__dpmi_set_segment_limit(selector, mapping.size-1);

   return 0;
}

//-----------------------------------------------------------------------------
// GB 2014: DOOM MBF specific, for 320x200 or 640x400, or else 640x480 with black bars
void vesa_blitscreen_banked(unsigned char *memory_buffer, int size, int scroll_offset)
{
   int offset, bank_number=0, copy_size=0;

   // Faster shortcut for 320x200 page no.1
   if (screen_h==200 && mode_banksize>=size && scroll_offset==0) 
   {
	  vesa_set_bank(0);
      dosmemput(memory_buffer, size, 0xA0000); // transfer the data
	  return;
   } 

   // Preparation part 1, calculate offset:
   offset=mode_BPS*scroll_offset; 
   // Some cards have no 640x400 (Intel, Cirrus), use 640x480 with black bars on top and bottom:
   if (screen_h==480) offset+=mode_BPS*40;  

   if (offset>0)
   {
      // Preparation part 2, calculate starting bank:
      while (offset>mode_banksize)
      {
         bank_number++; 
         offset-=mode_banksize;
      }
      // Preparation part 3, calculate amount to copy in the first pass:
      copy_size=mode_banksize-offset; 
      if (copy_size>size) copy_size=size; 
   }  
	
   while (size>0)
   {
      vesa_set_bank(bank_number);
      // calculate amount to copy in this pass (not the first pass):
      if (offset==0)
	  {
        // copy a whole bank or just the leftovers:
        if (size>mode_banksize) copy_size=mode_banksize; else copy_size=size;
	  }
	  dosmemput(memory_buffer, copy_size, 0xA0000+offset); // transfer the data
      size-=copy_size;
      memory_buffer+=copy_size;
      offset=0;
      bank_number++;
   }
}

//-----------------------------------------------------------------------------
// GB 2014: Done once for for 640x480 mode init
void vesa_clear_pages_banked(int pages, unsigned char color)
{
   int bank_number=0, copy_size, size, off;
   size=screen_h*mode_BPS*pages;
   memset(backingbuf,color,16); // prepare a buffer with the color to transfer
   while (size>0)
   {
      vesa_set_bank(bank_number);
      // copy a whole bank or just the leftovers:
      if (size>mode_banksize) copy_size=mode_banksize; else copy_size=size;
      // transfer the color, 16 bytes at  a time.
	  for (off=0 ; off<(copy_size/16); off++) dosmemput(backingbuf,16, 0xA0000+(off*16)); 
      size-=copy_size;
      bank_number++;
   }
}

//-----------------------------------------------------------------------------
// GB 2014: Freakin intel graphics won't do the above banked clear when intialized in LFB mode (flag 0x4000)
void vesa_clear_pages_LFB(int pages, unsigned char color)
{
   int size, off;
   unsigned char *dascreen;
   if (!screen_base_addr_set) return;
   size=screen_h*mode_BPS*pages;
   dascreen = (unsigned char *) screen_base_addr;
   memset(backingbuf,color,16); // prepare a buffer with the color to transfer
   // transfer the color, 16 bytes at  a time.
   for (off=0 ; off<(size/16); off++) memcpy(dascreen+(off*16),backingbuf,16);
}

//-----------------------------------------------------------------------------
/*
void vesa_putpixel(int x, int y, int color)
{
// int address = y*640+x;
   int address = y*mode_info.BytesPerScanLine+x; // GB 2014
   int bank_size = mode_info.WinGranularity*1024;
   int bank_number = address/bank_size;
   int bank_offset = address%bank_size;
   //if (pm_bankswitch) vesa_set_bank_pm(bank_number);
   //else               set_vesa_bank   (bank_number);
   vesa_set_bank (bank_number);
   _farpokeb(_dos_ds, 0xA0000+bank_offset, color);
}
*/
//-----------------------------------------------------------------------------
// DJGPP example:
/*
int vesa_find_mode(int w, int h)
{
    int mode_list[256], c;
    long mode_ptr;
    // check that the VESA driver exists, and get information about it:
 
    if (vesa_get_info() != 0) return 0;
    // convert the mode list pointer from seg:offset to a linear address:
  
	mode_ptr = ((vesa_info.VideoModePtr & 0xFFFF0000) >> 12) + (vesa_info.VideoModePtr & 0xFFFF);
  
    number_of_modes = 0;
    // read the list of available modes :
    while (_farpeekw(_dos_ds, mode_ptr) != 0xFFFF) {
      mode_list[number_of_modes] = _farpeekw(_dos_ds, mode_ptr);
      number_of_modes++;
      mode_ptr += 2;
    }
    // scan through the list of modes looking for the one that we want:
    for (c=0; c<number_of_modes; c++) {
       // get information about this mode:
       if (vesa_get_mode_info(mode_list[c]) != 1) continue;
       // check the flags field to make sure this is a color graphics mode,
       // and that it is supported by the current hardware 
       if ((mode_info.ModeAttributes & 0x19) != 0x19) continue;
       // check that this mode is the right size 
       if ((mode_info.XResolution != w) || (mode_info.YResolution != h)) continue;
       // check that there is only one color plane 
       if (mode_info.NumberOfPlanes != 1) continue;
       // check that it is a packed-pixel mode (other values are used for
       // different memory layouts, eg. 6 for a truecolor resolution):
       if (mode_info.MemoryModel != 4) continue;
       // check that this is an 8-bit (256 color) mode:
       if (mode_info.BitsPerPixel != 8) continue;
       // if it passed all those checks, this must be the mode we want!:
	   vesa_foundmode=current_mode_info=mode_list[c];
	   return current_mode_info;
    }
    // there was no mode matching the one we wanted! 
    current_mode_info=-1;
    return 0; 
}
*/

//-----------------------------------------------------------------------------
// Made specific for MBF, so we don't have to do this three times
int vesa_find_modes(int lfb_disable) 
{

   int mode_list[256], c, found=0;
   int number_of_modes; 
   long mode_ptr;
   // check that the VESA driver exists, and get information about it:
 
   if (vesa_get_info() != 0) return 0;
   // convert the mode list pointer from seg:offset to a linear address:
  
   mode_ptr = ((vesa_info.VideoModePtr & 0xFFFF0000) >> 12) + (vesa_info.VideoModePtr & 0xFFFF);
     
   // read the list of available modes :
   number_of_modes = 0;
   while (_farpeekw(_dos_ds, mode_ptr) != 0xFFFF) 
   {	
      mode_list[number_of_modes] = _farpeekw(_dos_ds, mode_ptr);
      number_of_modes++;
      mode_ptr += 2;
   }

   // scan through the list of modes looking for the one that we want:
   for (c=0; c<number_of_modes; c++) 
   {
      // get information about this mode:
      if (vesa_get_mode_info(mode_list[c]) != 1) continue;
      // check the flags field to make sure this is a color graphics mode,
      // and that it is supported by the current hardware 
      if ((mode_info.ModeAttributes & 0x19) != 0x19) continue;
      // check that there is only one color plane 
      if (mode_info.NumberOfPlanes != 1) continue;
      // check that it is a packed-pixel mode (other values are used for
      // different memory layouts, eg. 6 for a truecolor resolution):
      if (mode_info.MemoryModel != 4) continue;
      // check that this is an 8-bit (256 color) mode:
      if (mode_info.BitsPerPixel != 8) continue;
      // toggle the LFB attribute, as set by find_mode_info, for later use.
	  if (lfb_disable) mode_LFB=0;
      // check that this mode is the right size 
           if ((mode_info.XResolution == 320) && (mode_info.YResolution == 200)) {vesa_mode_320x200=mode_list[c]+mode_LFB*0x4000; found++;}
      else if ((mode_info.XResolution == 640) && (mode_info.YResolution == 400)) {vesa_mode_640x400=mode_list[c]+mode_LFB*0x4000; found++;}
      else if ((mode_info.XResolution == 640) && (mode_info.YResolution == 480)) {vesa_mode_640x480=mode_list[c]+mode_LFB*0x4000; found++;}
	  if (found>=3) return found;
   }
   return found; 
}

