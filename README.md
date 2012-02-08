SimpleAV aims on simplifing your job on decoding videos.
It's released under WTFPL v2.

Current version is 0.1.0, tested under libav v0.7 and ffmpeg v0.6.90.

The next version may features:

- precise seeking
- correct down-mixing

Start with "saplayer-old.c" if you are interested. :-P

--------------------------------------------------------------------------------
### COMPILATION & (UN)INSTALL:

    $ make
    $ sudo make install

    $ sudo make uninstall

--------------------------------------------------------------------------------
### USING SIMPLEAV:

SimpleAV is distributed with a small pkg-config script to help you with compiling.

Here's an example on using it:

    $ gcc -o your_program your_code.c `pkg-config --cflags --libs SimpleAV`

Notice that SimpleAV will always return decoded frames as AVFrame stuctures, so you would have to know how to convert a AVFrame to data format you need.

--------------------------------------------------------------------------------
### ABOUT SAPLAYER-OLD:

You would be able to use it after installing SimpleAV, or in this way:

    $ export LD_LIBRARY_PATH=.
    $ ./saplayer-old ...
