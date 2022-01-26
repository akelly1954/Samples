# Samples

## A collection of C++ objects in one or more libraries, and some main programs that utilize those objects.

The sources are organized in a source tree which is built using cmake (minimum version 3.10). 

New work is checked into the **dev** branch. The **master** branch is stable.

The **source** directory is where all the sources, build script(s) and CMake rlated files are.

After running the **linux-build.bash** script, the **build** directory is created (at the same level as **sources**).  It contains **ALL** the build artifacts.  

The **build** directory can always be removed without affecting any source files.  The **source** directory never has any build-related artifacts created under it.

These sources have been built and tested on Debian 9, with g++ (Debian 6.3.0-18+deb9u1), and cmake 3.15.3.

Currently WIN32 has not been built and tested yet. This will be added in the future.

## After Building: ##

**build/localrun** contains all the executables and libraries needed at runtim (set you LD_LIBRARY_PATH to at least ".:") 

**build/lib** contains all the libraries (shared and static) 

**build/include** contains all the .h\* files that are needed when building from an external build environment. 










