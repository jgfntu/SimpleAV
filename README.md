SimpleAV aims on simplifing your job on decoding videos.
It's released under WTFPL v2.

Current version is 0.1.0, tested under libav v0.7 and ffmpeg v0.6.90.

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

Currently SimpleAV will always be installed to /usr/local/ as shared libs, so you don't need to do anything special when calling gcc.

But as SimpleAV returns decoded frames as AVFrame-s, you would have to link your program to libav/ffmpeg.

SimpleAV is distributed with a small pkg-config script to help you with that.

Here's an example:

    $ gcc -o your_program your_code.c `pkg-config --cflags --libs SimpleAV`

Notice that the pkg-config command is infact equal to "pkg-config --cflags --libs libavcodec libavformat libavutil libswscale".

--------------------------------------------------------------------------------
### ABOUT SAPLAYER-OLD:

You would be able to use it after installing SimpleAV, or in this way:

    $ export LD_LIBRARY_PATH=.
    $ ./saplayer-old ...
