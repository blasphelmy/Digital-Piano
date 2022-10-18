# Digital-Piano

## Required
an sf2 sound file. Heres one that will work. 

	https://drive.google.com/u/0/uc?id=1p0jY3AgGyD9DJGWC25aEUEaydI_n1-3M&export=download

rename to soundfile_1.sf2 and place in project directory.

In main, adjust the gain through dcbGain.

### build for x64 release

In the command promt, enter in the name of one of the default midi files included in the project to play.

Up/down arrows to adjust playback speed

Left/Right arrows to skip forward and backwords.

You can also use a standard 88 key keyboard with this app.

### Notes

You may encunter bugs building and running the app in visual studio. Build first, then copy resource files to build directory (mid and sf2 files)