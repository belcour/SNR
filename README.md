# SNR
Compute distances such as MSR, SNR and L2 on images

### Compilation

SNR uses CMake to generate Makefile or project (for Visual Studio or XCode). You can find CMake [here](http://cmake.org/)).

    cmake .
    make

### Usage

SNR requires two EXR files to be specified: one reference file and the query file for which the metrics will be computed.

    ./SNR file.exr reference.exr

where `file.exr` is the query file and `reference.exr` is the reference image.