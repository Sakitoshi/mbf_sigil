// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: m_fixed.h,v 1.5 1998/05/10 23:42:22 killough Exp $
//
//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//      Fixed point arithemtics, implementation.
//
//-----------------------------------------------------------------------------


// GB 2014: This file was trouble: Floating point exceptions when compiler set to optimize!
// replaced enterily with the contents from SMMU, by Fraggle. 

#ifndef __M_FIXED__
#define __M_FIXED__

#ifndef __GNUC__
#define __inline__
#define __attribute__(x)
#endif

#include <stdlib.h> // for abs
#include "i_system.h"

//
// Fixed point, 32bit as 16.16.
//

#define FRACBITS 16
#define FRACUNIT (1<<FRACBITS)

typedef int fixed_t;

//
// Absolute Value
//

// killough 5/10/98: In djgpp, use inlined assembly for performance
// killough 9/05/98: better code seems to be gotten from using inlined C

#ifdef DJGPP
#define abs(x) ({fixed_t _t = (x), _s = _t >> (8*sizeof _t-1); (_t^_s)-_s;})
#endif 

//
// Fixed Point Multiplication
//

#ifdef DJGPP

// killough 5/10/98: In djgpp, use inlined assembly for performance
// sf: code imported from lxdoom

inline static const fixed_t FixedMul(fixed_t a, fixed_t b)
{
  fixed_t result;
  int dummy;

  asm("  imull %3 ;"
      "  shrdl $16,%1,%0 ;"
      : "=a" (result),          /* eax is always the result */
        "=d" (dummy)		/* cphipps - fix compile problem with gcc-2.95.1
				   edx is clobbered, but it might be an input */
      : "0" (a),                /* eax is also first operand */
        "r" (b)                 /* second operand could be mem or reg before,
				   but gcc compile problems mean i can only us reg */
      : "%cc"                   /* edx and condition codes clobbered */
      );

  return result;
}

#else

__inline__ static fixed_t FixedMul(fixed_t a, fixed_t b)
{
  return (fixed_t)((long long) a*b >> FRACBITS);
}

#endif 

//
// Fixed Point Division
//

#ifdef DJGPP

// killough 5/10/98: In djgpp, use inlined assembly for performance
// killough 9/5/98: optimized to reduce the number of branches
// sf: code imported from lxdoom

inline static const fixed_t FixedDiv(fixed_t a, fixed_t b)
{
  if (abs(a) >> 14 < abs(b))
    {
      fixed_t result;
      int dummy;
      asm(" idivl %4 ;"
	  : "=a" (result),
	    "=d" (dummy)  /* cphipps - fix compile problems with gcc 2.95.1
			     edx is clobbered, but also an input */
	  : "0" (a<<16),
	    "1" (a>>16),
	    "r" (b)
	  : "%cc"
	  );
      return result;
    }
  return ((a^b)>>31) ^ MAXINT;
}

#else 

__inline__ static fixed_t FixedDiv(fixed_t a, fixed_t b)
{
  return (abs(a)>>14) >= abs(b) ? ((a^b)>>31) ^ MAXINT :
    (fixed_t)(((long long) a << FRACBITS) / b);
}

#endif 

#endif

//----------------------------------------------------------------------------
//
// $Log: m_fixed.h,v $
//
// GB 2014: This file was trouble: Floating point exceptions when compiler set to optimize!
// replaced enterily with the contents from SMMU, by Fraggle. 
//
// Revision 1.5  1998/05/10  23:42:22  killough
// Add inline assembly for djgpp (x86) target
//
// Revision 1.4  1998/04/27  01:53:37  killough
// Make gcc extensions #ifdef'ed
//
// Revision 1.3  1998/02/02  13:30:35  killough
// move fixed point arith funcs to m_fixed.h
//
// Revision 1.2  1998/01/26  19:27:09  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:53  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------

