# Samples

## A collection of C++ objects in one or more libraries, and some main programs that utilize those objects.

The sources are organized in a source tree which is built using cmake (minimum version 3.10). 

New work is checked into the **dev** branch. The **main** branch is stable. The **master** branch is only accessible to the owner of the repository.  

The **source** directory is where all the sources, build script(s) and CMake rlated source files are. (CMake runtime files are created elsewhere - see the **build** directory below).

After running the **linux-build.bash** script, the **build** directory is created (at the same level as the **source** directory).  It contains **ALL** the build artifacts.  

The **build** directory can always be removed without affecting any source files.  The **source** directory never has any build-related artifacts created within it.

These sources have been built and tested on Debian 11 (bullseye), with 
Eclipse version 4.21,
g++ (Debian 10.2.1-6) 10.2.1 20210110, 
and cmake 3.18.4.

The Eclipse IDE is not required. The build scripts can be run (with the 
right flags) just using g++, cmake, and (optionally) your favorite IDE if it's supported 
by the CMake version used for the build.

Currently the Windows' WIN32 configuration has not been built and tested yet. 
This may be added in the future.

## After Building: ##

**build/localrun** contains all the executables and libraries needed at runtime (set your LD_LIBRARY_PATH to at least ".:") 

**build/lib** contains all the libraries (shared and static) 

**build/include** contains all the .h\* files that are needed when building from an external build environment. 










