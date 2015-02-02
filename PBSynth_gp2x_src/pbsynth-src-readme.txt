PBSynth v0.1a Sources Copyright (c)2004, 2008, 2009 by Christian Nowak
======================================================================

========
= GP2X =
========

The file "sdl.cpp" contain almost all principals modifications done for the GP2X port.

Sourcecode was compiled on GP2X using the CodeBlocks project (PBSynth_GP2X.cbp),
but i guess it could compile easily with the original makefile.

===================
= Original readme =
===================


As promised, here is another piece of source code from my active GP32 days - this time PBSynth, the Pocket synthesizer. Read PBSynth/readme.txt for further details about the program itself.


a) You can compile a GP32 application, cd to the PBSynth directory and type

	make -f Makefile.gp32
	
This assumes you have devkitadv and the GP32 patch installed.

b) To compile the synth as a native Win32 application, cd to PBSynth and type

	make

For this you need to have MinGW and the SDL library installed.

c) To compile a Steinberg VSTi of PBSynth, cd to PBSynth_VSTi and type

	make

You also need to have MinGW installed for this to work.


If you have any issues using, compiling or porting the sources, feel free to contact me, even though it may take some time for me to reply as I might be busy.
There will be more source codes coming from me in the near future, also previously unreleased source codes.. Just let me see what treasures I will dig out from my archives.

Have fun and good luck with your projects!

    Christian Nowak a.k.a. chn, Velbert/Germany, 12th January, 2008
    chnowak@web.de, linzizheng@gmail.com
