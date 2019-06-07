################################################################
#
# $Id: makefile,v 1.1.1.1 2000-07-29 13:20:41 fraggle Exp $
#
################################################################

CC = gcc

# the command you use to delete files
RM = del

# the command you use to copy files
#CP = copy /y (/y does not work in XP)
CP = copy 

# options common to all builds
#CFLAGS_COMMON = -Wall
CFLAGS_COMMON = -Wall -Winline -Wno-parentheses

# new features; comment out what you don't want at the moment
# DDOGS - Friendly helper dogs,                   
# DBETA - Doom beta version emulation 
# DV12C Doom v1.2 Sight Routines (Demo compat)
# DCALT - Alternate GFX procedures in C, for noasm parameter.  
#CFLAGS_NEWFEATURES = -DDOGS -DBETA -DV12C -DCALT 
CFLAGS_NEWFEATURES = -DDOGS -DBETA -DV12C -DCALT 

# debug options
CFLAGS_DEBUG = -g -O2 -DRANGECHECK -DINSTRUMENTED
LDFLAGS_DEBUG =

# optimized (release) options
# CFLAGS_RELEASE =-O1 -ffast-math -fomit-frame-pointer -m486 -fexpensive-optimizations
# CFLAGS_RELEASE =-O1 -ffast-math -fomit-frame-pointer -mtune=i486 -Wno-pointer-sign

 CFLAGS_RELEASE =-O3 -ffast-math -m486 
# CFLAGS_RELEASE =-O1 -ffast-math -mtune=i486 -Wno-pointer-sign -fexpensive-optimizations
 

# GCC 2.7: -ffast-math makes no difference
# GCC 2.7: -fomit-frame-pointer gives a few FPS less 
# GCC 2.7: -mreg-alloc=adcbSDB gives 0,2 FPS less
# -fstrength-reduce = better FPS with GCC 4, less FPS with GCC 2
# GCC 2.7: can also mess up MBF in hires, but very rarely. Just "-O3 -m486" gives most FPS. 
# GCC 2.9: "-O1 -m486"  works in hires in MBF under PCem, sometimes...
# GCC 4.7: "-O0 -mtune=i486" already messes up hires mode in MBF under PCem. More options will also bug normal mode.
# GCC 4.7: max FPS: -O1 -ffast-math -fomit-frame-pointer -mtune=i486 -fstrength-reduce -fexpensive-optimizations
# In summary GCC 4 with optimal options can give 1,4 FPS more: like 36,4 vs 35,0. But is not reliable in PCem. 
# removing all NEWFEATURES actually decreases FSP from 35 to 31,8 !?

LDFLAGS_RELEASE = -s

# libraries to link in
LIBS = -lalleg -lm -lemu

# this selects flags based on debug and release tagets
MODE = RELEASE
CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_$(MODE)) $(CFLAGS_NEWFEATURES)
LDFLAGS = $(LDFLAGS_COMMON) $(LDFLAGS_$(MODE))

# subdirectory for objects (depends on target, to allow you
# to build debug and release versions simultaneously)

O = $(O_$(MODE))
O_RELEASE = obj
O_DEBUG = objdebug

# object files
OBJS=	\
	$(O)/doomdef.o      \
        $(O)/doomstat.o     \
        $(O)/dstrings.o     \
        $(O)/i_system.o     \
        $(O)/i_sound.o      \
        $(O)/i_video.o      \
        $(O)/i_net.o        \
        $(O)/tables.o       \
        $(O)/f_finale.o     \
        $(O)/f_wipe.o       \
        $(O)/d_main.o       \
        $(O)/d_net.o        \
        $(O)/d_items.o      \
        $(O)/g_game.o       \
        $(O)/m_menu.o       \
        $(O)/m_misc.o       \
        $(O)/m_argv.o       \
        $(O)/m_bbox.o       \
        $(O)/m_cheat.o      \
        $(O)/m_random.o     \
        $(O)/am_map.o       \
        $(O)/p_ceilng.o     \
        $(O)/p_doors.o      \
        $(O)/p_enemy.o      \
        $(O)/p_floor.o      \
        $(O)/p_inter.o      \
        $(O)/p_lights.o     \
        $(O)/p_map.o        \
        $(O)/p_maputl.o     \
        $(O)/p_plats.o      \
        $(O)/p_pspr.o       \
        $(O)/p_setup.o      \
        $(O)/p_sight.o      \
        $(O)/p_spec.o       \
        $(O)/p_switch.o     \
        $(O)/p_mobj.o       \
        $(O)/p_telept.o     \
        $(O)/p_tick.o       \
        $(O)/p_saveg.o      \
        $(O)/p_user.o       \
        $(O)/r_bsp.o        \
        $(O)/r_data.o       \
        $(O)/r_draw.o       \
        $(O)/r_main.o       \
        $(O)/r_plane.o      \
        $(O)/r_segs.o       \
        $(O)/r_sky.o        \
        $(O)/r_things.o     \
        $(O)/w_wad.o        \
        $(O)/wi_stuff.o     \
        $(O)/v_video.o      \
        $(O)/i_vgavbe.o     \
        $(O)/st_lib.o       \
        $(O)/st_stuff.o     \
        $(O)/hu_stuff.o     \
        $(O)/hu_lib.o       \
        $(O)/s_sound.o      \
        $(O)/z_zone.o       \
	$(O)/keyboard.o     \
        $(O)/info.o         \
        $(O)/sounds.o       \
        $(O)/mmus2mid.o     \
        $(O)/i_main.o       \
        $(O)/pproblit.o     \
        $(O)/drawspan.o     \
        $(O)/drawcol.o      \
        $(O)/p_genlin.o     \
        $(O)/d_deh.o	    \
 	$(O)/emu8kmid.o	    

doom all: $(O)/mbf.exe
	$(CP) $(O)\mbf.exe .

release: clean
	$(RM) tranmap.dat
	$(RM) mbf.cfg
	$(RM) mbfsrc.zip
	$(RM) examples.zip
	$(RM) mbf.zip
	pkzip -a -ex -rp mbfsrc
	$(MAKE) all
	pkzip -a -ex examples examples\*.*
	pkzip -a -ex mbf mbf.exe mbffaq.txt mbfedit.txt mbf.txt options.txt
	pkzip -a -Ex mbf doomlic.txt copying copying.dj snddrvr.txt doom17.dat
	pkzip -a -Ex mbf common.cfg examples.zip betalevl.wad betagrph.wad
	pkzip -a -Ex mbf asetup.exe cwsdpmi.exe copying copying.dj readme.1st
	$(RM) examples.zip

debug:
	$(MAKE) MODE=DEBUG

clean:
	$(RM) mbf.exe
	$(RM) $(O_RELEASE)\*.exe
	$(RM) $(O_DEBUG)\*.exe
	$(RM) $(O_RELEASE)\*.o
	$(RM) $(O_DEBUG)\*.o

$(O)/mbf.exe: $(OBJS) $(O)/version.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(O)/version.o -o $@ $(LIBS)
	$(RM) $(O)\version.o

$(O)/%.o:   %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o:   %.s
	$(CC) $(CFLAGS) -c $< -o $@

# Very important that all sources #include this one
$(OBJS): z_zone.h

# If you change the makefile, everything should rebuild
$(OBJS): Makefile

# individual file depedencies follow

$(O)/doomdef.o: doomdef.c doomdef.h z_zone.h m_swap.h version.h

$(O)/doomstat.o: doomstat.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h

$(O)/dstrings.o: dstrings.c dstrings.h d_englsh.h

$(O)/i_system.o: i_system.c i_system.h d_ticcmd.h doomtype.h i_sound.h \
 sounds.h doomstat.h doomdata.h d_net.h d_player.h d_items.h doomdef.h \
 z_zone.h m_swap.h version.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h m_misc.h g_game.h d_event.h w_wad.h v_video.h

$(O)/i_sound.o: i_sound.c z_zone.h doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h mmus2mid.h \
 i_sound.h sounds.h w_wad.h g_game.h d_event.h d_main.h s_sound.h

$(O)/i_video.o: i_video.c z_zone.h doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h v_video.h \
 r_data.h r_defs.h r_state.h d_main.h d_event.h st_stuff.h m_argv.h w_wad.h \
 sounds.h s_sound.h r_draw.h am_map.h m_menu.h wi_stuff.h 

$(O)/i_net.o: i_net.c z_zone.h doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h d_event.h \
 m_argv.h i_net.h

$(O)/tables.o: tables.c tables.h m_fixed.h i_system.h d_ticcmd.h doomtype.h

$(O)/f_finale.o: f_finale.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 d_event.h v_video.h r_data.h r_defs.h r_state.h w_wad.h s_sound.h \
 sounds.h dstrings.h d_englsh.h d_deh.h hu_stuff.h m_menu.h

$(O)/f_wipe.o: f_wipe.c doomdef.h z_zone.h m_swap.h version.h i_video.h \
 doomtype.h v_video.h r_data.h r_defs.h m_fixed.h i_system.h \
 d_ticcmd.h d_think.h p_mobj.h tables.h doomdata.h info.h r_state.h \
 d_player.h d_items.h p_pspr.h m_random.h f_wipe.h

$(O)/d_main.o: d_main.c doomdef.h z_zone.h m_swap.h version.h doomstat.h \
 doomdata.h doomtype.h d_net.h d_player.h d_items.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h dstrings.h \
 d_englsh.h sounds.h w_wad.h s_sound.h v_video.h r_data.h r_defs.h \
 r_state.h f_finale.h d_event.h f_wipe.h m_argv.h m_misc.h m_menu.h \
 i_sound.h i_video.h g_game.h hu_stuff.h wi_stuff.h st_stuff.h \
 am_map.h p_setup.h r_draw.h r_main.h d_main.h d_deh.h

$(O)/d_net.o: d_net.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h m_menu.h \
 d_event.h i_video.h i_net.h g_game.h

$(O)/d_items.o: d_items.c info.h d_think.h d_items.h doomdef.h z_zone.h \
 m_swap.h version.h

$(O)/g_game.o: g_game.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h f_finale.h \
 d_event.h m_argv.h m_misc.h m_menu.h m_random.h p_setup.h p_saveg.h \
 p_tick.h d_main.h wi_stuff.h hu_stuff.h st_stuff.h am_map.h w_wad.h \
 r_main.h r_data.h r_defs.h r_state.h r_draw.h p_map.h s_sound.h \
 dstrings.h d_englsh.h sounds.h r_sky.h d_deh.h p_inter.h g_game.h

$(O)/m_menu.o: m_menu.c doomdef.h z_zone.h m_swap.h version.h doomstat.h \
 doomdata.h doomtype.h d_net.h d_player.h d_items.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h dstrings.h \
 d_englsh.h d_main.h d_event.h i_video.h v_video.h r_data.h r_defs.h \
 r_state.h w_wad.h r_main.h hu_stuff.h g_game.h s_sound.h sounds.h \
 m_menu.h d_deh.h m_misc.h

$(O)/m_misc.o: m_misc.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h m_argv.h \
 g_game.h d_event.h m_menu.h am_map.h w_wad.h i_sound.h sounds.h \
 i_video.h v_video.h r_data.h r_defs.h r_state.h hu_stuff.h st_stuff.h \
 dstrings.h d_englsh.h m_misc.h s_sound.h d_main.h

$(O)/m_argv.o: m_argv.c

$(O)/m_vgavbe.o: m_vgavbe.c m_vgavbe.h

$(O)/m_bbox.o: m_bbox.c m_bbox.h z_zone.h m_fixed.h i_system.h d_ticcmd.h \
 doomtype.h

$(O)/m_cheat.o: m_cheat.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 g_game.h d_event.h r_data.h r_defs.h r_state.h p_inter.h m_cheat.h \
 m_argv.h s_sound.h sounds.h dstrings.h d_englsh.h d_deh.h

$(O)/m_random.o: m_random.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 m_random.h

$(O)/am_map.o: am_map.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h st_stuff.h \
 d_event.h r_main.h r_data.h r_defs.h r_state.h p_setup.h p_maputl.h \
 w_wad.h v_video.h p_spec.h am_map.h dstrings.h d_englsh.h d_deh.h

$(O)/p_ceilng.o: p_ceilng.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 r_main.h r_data.h r_defs.h r_state.h p_spec.h p_tick.h s_sound.h \
 sounds.h

$(O)/p_doors.o: p_doors.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 r_main.h r_data.h r_defs.h r_state.h p_spec.h p_tick.h s_sound.h \
 sounds.h dstrings.h d_englsh.h d_deh.h

$(O)/p_enemy.o: p_enemy.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 m_random.h r_main.h r_data.h r_defs.h r_state.h p_maputl.h p_map.h \
 p_setup.h p_spec.h s_sound.h sounds.h p_inter.h g_game.h d_event.h \
 p_enemy.h p_tick.h m_bbox.h

$(O)/p_floor.o: p_floor.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 r_main.h r_data.h r_defs.h r_state.h p_map.h p_spec.h p_tick.h \
 s_sound.h sounds.h

$(O)/p_inter.o: p_inter.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 dstrings.h d_englsh.h m_random.h am_map.h d_event.h r_main.h r_data.h \
 r_defs.h r_state.h s_sound.h sounds.h d_deh.h p_inter.h p_tick.h

$(O)/p_lights.o: p_lights.c doomdef.h z_zone.h m_swap.h version.h \
 m_random.h doomtype.h r_main.h d_player.h d_items.h p_pspr.h d_net.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 doomdata.h r_data.h r_defs.h r_state.h p_spec.h p_tick.h doomstat.h

$(O)/p_map.o: p_map.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h r_main.h \
 r_data.h r_defs.h r_state.h p_maputl.h p_map.h p_setup.h p_spec.h \
 s_sound.h sounds.h p_inter.h m_random.h m_bbox.h

$(O)/p_maputl.o: p_maputl.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 m_bbox.h r_main.h r_data.h r_defs.h r_state.h p_maputl.h p_map.h \
 p_setup.h

$(O)/p_plats.o: p_plats.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 m_random.h r_main.h r_data.h r_defs.h r_state.h p_spec.h p_tick.h \
 s_sound.h sounds.h

$(O)/p_pspr.o: p_pspr.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h r_main.h \
 r_data.h r_defs.h r_state.h p_map.h p_inter.h p_enemy.h m_random.h \
 s_sound.h sounds.h d_event.h p_tick.h

$(O)/p_setup.o: p_setup.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 m_bbox.h m_argv.h g_game.h d_event.h w_wad.h r_main.h r_data.h \
 r_defs.h r_state.h r_things.h p_maputl.h p_map.h p_setup.h p_spec.h \
 p_tick.h p_enemy.h s_sound.h

$(O)/p_sight.o: p_sight.c r_main.h d_player.h d_items.h doomdef.h z_zone.h \
 m_swap.h version.h p_pspr.h m_fixed.h i_system.h d_ticcmd.h \
 doomtype.h tables.h info.h d_think.h p_mobj.h doomdata.h r_data.h \
 r_defs.h r_state.h p_maputl.h p_setup.h m_bbox.h

$(O)/p_spec.o: p_spec.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h p_spec.h \
 r_defs.h p_tick.h p_setup.h m_random.h d_englsh.h m_argv.h w_wad.h \
 r_main.h r_data.h r_state.h p_maputl.h p_map.h g_game.h d_event.h \
 p_inter.h s_sound.h sounds.h m_bbox.h d_deh.h r_plane.h

$(O)/p_switch.o: p_switch.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 w_wad.h r_main.h r_data.h r_defs.h r_state.h p_spec.h g_game.h \
 d_event.h s_sound.h sounds.h

$(O)/p_mobj.o: p_mobj.c doomdef.h z_zone.h m_swap.h version.h doomstat.h \
 doomdata.h doomtype.h d_net.h d_player.h d_items.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h m_random.h \
 r_main.h r_data.h r_defs.h r_state.h p_maputl.h p_map.h p_tick.h \
 sounds.h st_stuff.h d_event.h hu_stuff.h s_sound.h g_game.h p_inter.h

$(O)/p_telept.o: p_telept.c doomdef.h z_zone.h m_swap.h version.h p_spec.h \
 r_defs.h m_fixed.h i_system.h d_ticcmd.h doomtype.h d_think.h p_user.h \
 p_mobj.h tables.h doomdata.h info.h d_player.h d_items.h p_pspr.h \
 p_maputl.h p_map.h r_main.h r_data.h r_state.h p_tick.h s_sound.h \
 sounds.h doomstat.h d_net.h

$(O)/p_tick.o: p_tick.c z_zone.h doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h p_user.h \
 p_spec.h r_defs.h p_tick.h

$(O)/p_saveg.o: p_saveg.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 r_main.h r_data.h r_defs.h r_state.h p_maputl.h p_spec.h p_tick.h \
 p_saveg.h m_random.h am_map.h d_event.h p_enemy.h

$(O)/p_user.o: p_user.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h d_event.h \
 r_main.h r_data.h r_defs.h r_state.h p_map.h p_spec.h p_user.h

$(O)/r_bsp.o: r_bsp.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h m_bbox.h \
 r_main.h r_data.h r_defs.h r_state.h r_segs.h r_plane.h r_things.h

$(O)/r_data.o: r_data.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h w_wad.h \
 r_main.h r_data.h r_defs.h r_state.h r_sky.h

$(O)/r_draw.o: r_draw.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h w_wad.h \
 r_main.h r_data.h r_defs.h r_state.h v_video.h m_menu.h

$(O)/r_main.o: r_main.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h r_data.h \
 r_defs.h r_state.h r_main.h r_bsp.h r_segs.h r_plane.h r_things.h \
 r_draw.h m_bbox.h r_sky.h v_video.h

$(O)/r_plane.o: r_plane.c z_zone.h i_system.h d_ticcmd.h doomtype.h w_wad.h \
 doomdef.h m_swap.h version.h doomstat.h doomdata.h d_net.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 r_plane.h r_data.h r_defs.h r_state.h r_main.h r_bsp.h r_segs.h \
 r_things.h r_draw.h r_sky.h

$(O)/r_segs.o: r_segs.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h r_main.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_plane.h r_things.h r_draw.h \
 w_wad.h

$(O)/r_sky.o: r_sky.c r_sky.h m_fixed.h i_system.h d_ticcmd.h doomtype.h

$(O)/r_things.o: r_things.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 w_wad.h r_main.h r_data.h r_defs.h r_state.h r_bsp.h r_segs.h \
 r_draw.h r_things.h

$(O)/w_wad.o: w_wad.c doomstat.h doomdata.h doomtype.h d_net.h d_player.h \
 d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h w_wad.h

$(O)/wi_stuff.o: wi_stuff.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 m_random.h w_wad.h g_game.h d_event.h r_main.h r_data.h r_defs.h \
 r_state.h v_video.h wi_stuff.h s_sound.h sounds.h 

$(O)/v_video.o: v_video.c doomdef.h z_zone.h m_swap.h version.h r_main.h \
 d_player.h d_items.h p_pspr.h m_fixed.h i_system.h d_ticcmd.h \
 doomtype.h tables.h info.h d_think.h p_mobj.h doomdata.h r_data.h \
 r_defs.h r_state.h m_bbox.h w_wad.h v_video.h i_video.h i_vgavbe.h 

$(O)/st_lib.o: st_lib.c doomdef.h z_zone.h m_swap.h version.h v_video.h \
 doomtype.h r_data.h r_defs.h m_fixed.h i_system.h d_ticcmd.h \
 d_think.h p_mobj.h tables.h doomdata.h info.h r_state.h d_player.h \
 d_items.h p_pspr.h w_wad.h st_stuff.h d_event.h st_lib.h r_main.h \
 r_bsp.h r_segs.h r_plane.h r_things.h r_draw.h

$(O)/st_stuff.o: st_stuff.c doomdef.h z_zone.h m_swap.h version.h \
 doomstat.h doomdata.h doomtype.h d_net.h d_player.h d_items.h \
 p_pspr.h m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h \
 p_mobj.h m_random.h i_video.h w_wad.h st_stuff.h d_event.h st_lib.h \
 r_defs.h v_video.h r_data.h r_state.h r_main.h am_map.h m_cheat.h \
 s_sound.h sounds.h dstrings.h d_englsh.h

$(O)/hu_stuff.o: hu_stuff.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 hu_stuff.h d_event.h hu_lib.h r_defs.h v_video.h r_data.h r_state.h \
 st_stuff.h w_wad.h s_sound.h dstrings.h d_englsh.h sounds.h d_deh.h \
 r_draw.h

$(O)/hu_lib.o: hu_lib.c doomdef.h z_zone.h m_swap.h version.h v_video.h \
 doomtype.h r_data.h r_defs.h m_fixed.h i_system.h d_ticcmd.h \
 d_think.h p_mobj.h tables.h doomdata.h info.h r_state.h d_player.h \
 d_items.h p_pspr.h hu_lib.h r_main.h r_bsp.h r_segs.h r_plane.h \
 r_things.h r_draw.h

$(O)/s_sound.o: s_sound.c doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h z_zone.h m_swap.h version.h p_pspr.h \
 m_fixed.h i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h \
 s_sound.h i_sound.h sounds.h r_main.h r_data.h r_defs.h r_state.h \
 m_random.h w_wad.h

$(O)/z_zone.o: z_zone.c z_zone.h doomstat.h doomdata.h doomtype.h d_net.h \
 d_player.h d_items.h doomdef.h m_swap.h version.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h

$(O)/info.o: info.c doomdef.h z_zone.h m_swap.h version.h sounds.h \
 m_fixed.h i_system.h d_ticcmd.h doomtype.h p_mobj.h tables.h \
 d_think.h doomdata.h info.h w_wad.h

$(O)/sounds.o: sounds.c doomtype.h sounds.h

$(O)/mmus2mid.o: mmus2mid.c mmus2mid.h z_zone.h

$(O)/i_main.o: i_main.c doomdef.h z_zone.h m_swap.h version.h m_argv.h \
 d_main.h d_event.h doomtype.h i_system.h d_ticcmd.h

$(O)/p_genlin.o: p_genlin.c r_main.h d_player.h d_items.h doomdef.h \
 z_zone.h m_swap.h version.h p_pspr.h m_fixed.h i_system.h d_ticcmd.h \
 doomtype.h tables.h info.h d_think.h p_mobj.h doomdata.h r_data.h \
 r_defs.h r_state.h p_spec.h p_tick.h m_random.h s_sound.h sounds.h \
 doomstat.h

$(O)/d_deh.o: d_deh.c doomdef.h z_zone.h m_swap.h version.h doomstat.h \
 doomdata.h doomtype.h d_net.h d_player.h d_items.h p_pspr.h m_fixed.h \
 i_system.h d_ticcmd.h tables.h info.h d_think.h p_mobj.h sounds.h \
 m_cheat.h p_inter.h g_game.h d_event.h dstrings.h d_englsh.h w_wad.h

$(O)/version.o: version.c version.h z_zone.h

# Allegro patches required to function satisfactorily

$(O)/emu8kmid.o: emu8kmid.c emu8k.h internal.h interndj.h allegro.h

$(O)/keyboard.o: keyboard.c internal.h interndj.h allegro.h
	$(CC) $(CFLAGS_COMMON) -O $(CFLAGS_NEWFEATURES) -c $< -o $@

# bin2c utility

bin2c: $(O)/bin2c.exe
	$(CP) $(O)\bin2c.exe .

$(O)/bin2c.exe: $(O)/bin2c.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(O)/bin2c.o -o $@ $(LIBS)

$(O)/bin2c.o: bin2c.c

###############################################################################
# $Log: makefile,v $
# Revision 1.1.1.1  2000-07-29 13:20:41  fraggle
# imported sources
#
# Revision 1.38  1998/05/18  22:59:22  killough
# Update p_lights.o depedencies
#
# Revision 1.37  1998/05/16  09:41:24  jim
# formatted net files, installed temp switch for testing Stan/Lee's version
#
# Revision 1.36  1998/05/15  00:35:40  killough
# Remove p_tick.h dependence from i_main.o
#
# Revision 1.35  1998/05/13  22:58:58  killough
# Restore Doom bug compatibility for demos
#
# Revision 1.34  1998/05/12  12:47:35  phares
# Removed OVER_UNDER code
#
# Revision 1.33  1998/05/10  23:43:03  killough
# Add p_user.h to p_user.o, p_telept.o dependencies
#
# Revision 1.32  1998/05/05  16:29:20  phares
# Removed RECOIL and OPT_BOBBING defines
#
# Revision 1.31  1998/05/03  23:20:15  killough
# Reflect current dependencies, beautify
#
# Revision 1.30  1998/04/27  02:25:42  killough
# Fix missing tabs after fixing cvsto script
#
# Revision 1.26  1998/04/14  08:14:50  killough
# remove obsolete ADAPTIVE_GAMETICS
#
# Revision 1.25  1998/04/13  13:03:22  killough
# Add -DADAPTIVE_GAMETIC
#
# Revision 1.24  1998/04/13  09:43:20  killough
# Add watermap.wad to dckboom.zip
#
# Revision 1.23  1998/04/12  02:07:37  killough
# Add r_segs.c dependency for translucency
#
# Revision 1.22  1998/04/09  09:18:52  thldrmn
# Added dependency of d_main.c on d_deh.h
#
# Revision 1.21  1998/03/29  21:40:17  jim
# Fix to DEH text problem
#
# Revision 1.20  1998/03/23  15:24:21  phares
# Changed pushers to linedef control
#
# Revision 1.19  1998/03/23  03:23:47  killough
# Add dckboom, bin2c targets, add new dependencies
#
# Revision 1.18  1998/03/20  00:29:51  phares
# Changed friction to linedef control
#
# Revision 1.17  1998/03/16  12:32:18  killough
# Add -m486 flag for better 486 codegen
#
# Revision 1.16  1998/03/11  17:48:20  phares
# New cheats, clean help code, friction fix
#
# Revision 1.15  1998/03/10  07:15:05  jim
# Initial DEH support added, minus text
#
# Revision 1.14  1998/03/09  07:17:18  killough
# Turn on instrumenting in debug builds
#
# Revision 1.13  1998/03/04  21:02:24  phares
# Dynamic HELP screen
#
# Revision 1.12  1998/03/02  11:38:42  killough
# Add dependencies, clarify CFLAGS_DEBUG
#
# Revision 1.11  1998/02/27  08:10:05  phares
# Added optional player bobbing
#
# Revision 1.10  1998/02/24  08:46:08  phares
# Pushers, recoil, new friction, and over/under work
#
# Revision 1.9  1998/02/23  04:42:54  killough
# Correct depedencies
#
# Revision 1.8  1998/02/18  01:00:11  jim
# Addition of HUD
#
# Revision 1.7  1998/02/17  05:41:34  killough
# Add drawspan.s, drawcol.s, fix some dependencies
#
# Revision 1.6  1998/02/08  05:35:13  jim
# Added generalized linedef types
#
# Revision 1.5  1998/02/02  13:26:04  killough
# Add version information files
#
# Revision 1.4  1998/01/30  14:45:16  jim
# Makefile changed to build BOOM.EXE
#
# Revision 1.3  1998/01/26  19:31:15  phares
# First rev w/o ^Ms
#
# Revision 1.2  1998/01/26  05:56:52  killough
# Add PPro blit routine
#
# Revision 1.1.1.1  1998/01/19  14:02:52  rand
# Lee's Jan 19 sources
#
##################################################