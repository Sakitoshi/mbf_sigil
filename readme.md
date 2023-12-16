### DOOM 'Marine's Best Friend' with Sigil support

![alt_text](https://raw.githubusercontent.com/Sakitoshi/mbf_sigil/master/docs/mbf_sigil.png)

![alt_text](docs/mbf_sigil2.png)

This is MBF modified to accomodate Sigil & Sigil II as proper 5th and 6th episodes.
I got the source code from [here](https://www.vogons.org/viewtopic.php?f=24&t=40857) and ported the changes from [libretro-prboom](https://github.com/libretro/libretro-prboom) (pull requests 88 to 90 and 97)

#### Compiling

##### Pre-requisites

- [allegro 3.0](https://liballeg.org/old.html)
- an old djgpp version from MBF's era (confirmed working: bnu27b.zip, djdev201.zip, GCC2721B.ZIP, mak3791b.zip, txi390b.zip)
- a pc running dos (dosbox is fine too)

##### Steps (on Linux with dosbox installed)

- Clone this repo
- Unzip djgpp into a new folder in your checkout called `djgpp` such that gcc is at `djgpp/djgpp/bin/gcc.exe`
- Unzip the allegro 3.0 sources to a new folder in your checkout called `allegro`
- Run `./build.sh allegro` which will run dosbox for you with the right settings (current folder mounted as `c:\`) and build allegro.
- Then you can `./build.sh mbf` to build MBF.

#### Changes
- Added full support for episode 6: Sigil II
- Added full support for episode 5: Sigil.
- Sound card settings are read from allegro.cfg again.

#### Important notes
- You need to use The Ultimate Doom wad (named DOOM.WAD or DOOMU.WAD).
- SIGIL_SHREDS.WAD is not supported for obvious reasons (no mp3 playback).
- To play Sigil II, you need to also load Sigil (`-iwad doom.wad -file sigil.wad -file sigil2.wad`).