# STB TrueType Bee Movie

The is an example which uses [stb_truetype](https://github.com/nothings/stb) to render the entire Bee Movie script into a file. The Bee Movie script has been extracted from [here](http://www.script-o-rama.com/movie_scripts/a1/bee-movie-script-transcript-seinfeld.html).

It is based on an [example](https://github.com/nothings/stb/blob/c9064e317699d2e495f36ba4f9ac037e88ee371a/stb_truetype.h#L367) from **stb** itself, but converted to C++ and upgraded to output the Bee Movie script.

# Build

[xmake](https://xmake.io) is used to build the example, but since it is so small you can put it into whatever build system you want. Just make sure to **stb_truetype** to the include directories. Everything else is default C++11.