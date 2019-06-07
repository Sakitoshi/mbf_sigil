//
//                        -= I_VGAVBE =-                    
//
// G. Broers 10-2014 
// http://members.quicknet.nl/lm.broers/        
//-----------------------------------------------------------------------------

#ifndef __IVGAVBE__
#define __IVGAVBE__

void blast_C(void *dest, unsigned char *src, int ymax);
int  vesa_get_mode_info(int mode);
int  vesa_set_mode(int mode);
void vesa_set_bank(int bank_number);
void vesa_blitscreen_banked(unsigned char *memory_buffer, int size, int scroll_offset);
int  vesa_set_displaystart(unsigned long pixel, unsigned long scanline, unsigned int waitVRT);
void set_mode_13h();
void set_mode_X();
void set_mode_text();
void wait_vsync();
int  vesa_find_modes(int lfb_disable);
int  vesa_get_info();
int  vesa_get_screen_base_addr(int close);
void vesa_clear_pages_banked(int pages, unsigned char color);
void vesa_clear_pages_LFB(int pages, unsigned char color);
int  get_vesa_pm_functions(int close);
/*
int  vesa_get_mode();
void vesa_clear_banks(int banks);
void vesa_putpixel(int x, int y, int color);
*/

extern int current_mode_info;
extern int current_mode;
extern int screen_w;
extern int screen_h;
extern int mode_BPS;
extern int mode_LFB;
extern int mode_banksize;
extern int vesa_version;
extern int vesa_mode_320x200;
extern int vesa_mode_640x400;
extern int vesa_mode_640x480;
extern unsigned long screen_base_addr;
extern unsigned long mode_LFB_PTR;

#endif
