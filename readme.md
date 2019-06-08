### DOOM 'Marine's Best Friend' with Sigil support

![alt_text](https://raw.githubusercontent.com/Sakitoshi/mbf_sigil/master/docs/mbf_sigil.png)

This is MBF modified to accomodate Sigil as a proper 5th episode.
I got the source code from [here](https://www.vogons.org/viewtopic.php?f=24&t=40857) and ported the changes from [libretro-prboom](https://github.com/libretro/libretro-prboom) (pull requests 88 to 90)

#### Compiling
To compile it you'll need allegro 3.0, djgpp and a pc running dos (dosbox is fine too).

#### Changes
- Added full support for episode 5: Sigil.
- Sound card settings are read from allegro.cfg again.

#### Important notes
- You need to use The Ultimate Doom wad (named DOOM.WAD or DOOMU.WAD).
- SIGIL_SHREDS.WAD is not supported for obvious reasons (no mp3 playback).
