# Samples

# Welcome.  Please Note:

This is work in progress -- I'm uploading my own sources to the repository while ensuring that the code is tested and working. So for a period of time I've got restrictions on interactions with the repositories that allow one to clone and download code (to which you are welcome as per the LICENSE) but am not yet welcoming collaborators. Right now I only have some base libraries that don't really do much.  As soon as I introduce code that does something useful, I'll remove the restrictions.  In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.

Thank you.

## A collection of C++ objects in one or more libraries, and some main programs that utilize those objects.

The sources are organized in a source tree which is built using cmake (minimum version 3.10). There are (will be) additional README files in the subdirectories going into more detail of how to use the build environment.  

New work is checked into the **dev** branch. The **main** branch is stable. 

The **source** directory is where all the sources, build script(s) and CMake rlated source files are. (CMake runtime files are created elsewhere - see the **build** directory mentioned below).

The script **base-linux-build.bash** in the **source** directory builds **everything linux**. After running it, the **build** directory is created (if it does not exist) at the same level as the **source** directory (as in **./source/../build/**) and it contains **ALL** the build artifacts.  

Again, this is an "out of source" build environment.  The **build** directory can always be removed without affecting any source files.  The **source** directory never has any build-related artifacts created within it.  To clean everything, simply remove the **build** directory (**./source/../build**). 

The various other directories that are base directories for the various projects included under **source**, each has its own **linux-build.bash** script which builds that particular project.  

These sources have been built and tested on Debian 11 (bullseye), with 
Eclipse version 4.21,
g++ (Debian 10.2.1-6) 10.2.1 20210110, 
and cmake 3.18.4.

The Eclipse IDE is not required. The build scripts can be run (with the 
right flags) just using g++, cmake, and (optionally) your favorite IDE if it's supported 
by the CMake version used for the build. 

### Current deficiencies:

Currently the Windows' WIN32 configuration has not been built and tested yet. 
This may be added in the future.

The **base-linux-build.bash** script is currently hardcoded to produce **eclipse/make** projects.  Many other options are available courtesy of **cmake**, and this will be accommodated in the future (complete with a --help flag which does not exist right now).


## After Building: ##

**build/localrun** contains all the executables and libraries needed at runtime (set your LD_LIBRARY_PATH to at least ".:") 

**build/lib** contains all the libraries (shared and static) 

**build/include** contains all the .h\* files that are needed when building from an external build environment. 










