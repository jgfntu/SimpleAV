SimpleAV aims on simplifing your job on decoding videos.
It's released under WTFPL v2.

Current version is 0.1.0, tested under libav v0.7.

The next version may features:

- precise seeking
- correct down-mixing

Start with "saplayer-old.c" if you are interested. :-P

--------------------------------------------------------------------------------
### COMPILATION:

    $ make
    $ sudo make install

--------------------------------------------------------------------------------
### USING SIMPLEAV:

To use SimpleAV in your own program, just add "-lSimpleAV" when linking.
But be cautious, you would have to tell gcc to link to libav.

Here's an example:

    $ gcc your_code.o -lSimpleAV `pkg-config --libs libavcodec libavformat libavutil libswscale sdl`

--------------------------------------------------------------------------------
### ABOUT SAPLAYER-OLD:

You would be able to use it after installing SimpleAV, or in this way:

    $ export LD_LIBRARY_PATH=.
    $ ./saplayer-old ...
