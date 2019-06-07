// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: keyboard.h,v 1.1 2000-08-11 23:11:50 fraggle Exp $
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
//--------------------------------------------------------------------------
//
// Include file for modified allegro keyboard.c
//
//--------------------------------------------------------------------------

#ifndef DOOMKEY_H
#define DOOMKEY_H

extern void (*doom_keyboard_lowlevel_callback)(int);
extern int doom_install_keyboard();
extern void doom_clear_keybuf();
extern int doom_keypressed();
extern int doom_readkey();
extern void doom_simulate_keypress(int key);
extern int doom_install_keyboard();
extern void doom_set_leds(int leds);

extern unsigned char doom_key_ascii_table[128];
extern unsigned char doom_key_capslock_table[128];
extern unsigned char doom_key_shift_table[128];

#endif /* DOOMKEY_H */
