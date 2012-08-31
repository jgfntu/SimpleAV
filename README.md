SimpleAV aims on simplifing your job on decoding videos.
It's released under WTFPL v2.

Current version is 0.1.1, tested with libav v0.7.6 under Mac OS 10.7.
SimpleAV was created under Linux; so I will make sure it works under Linux later.

The next minor version may features:

- precise seeking
- correct down-mixing

Start with "saplayer-old.c" if you are interested. :-P

--------------------------------------------------------------------------------
### COMPILATION & (UN)INSTALL:

    $ cd build
    $ cmake ..
    $ make
    $ sudo make install

    $ cd build
    $ xargs rm < install_manifest.txt

Make sure you have zlib and libbzip2 installed.
SDL is also required for now, but you could crack CMakeList.txt if you dislike saplayer-old.

--------------------------------------------------------------------------------
### USING SIMPLEAV:

SimpleAV is distributed with a small pkg-config script to help you use it.

Here's an example on using it:

    $ gcc -o your_program your_code.c `pkg-config --cflags --libs SimpleAV`

SimpleAV will return decoded frames as AVFrames, so do some convertion yourself.

--------------------------------------------------------------------------------
### ABOUT SAPLAYER-OLD:

It's just a demo video player, so will not be installed with `make install`.
However, you could crack CMakeLists.txt if... (BOOM!)

