# Samples

## The Sources and How To Build Them

A collection of C++ objects in one or more libraries, and some main programs that utilize those objects.

The sources are organized in a source tree which is built using cmake (minimum version 3.10). There are additional README files in the subdirectories (of which this README is one) going into more detail of how to use the build environment.  

New work is checked into the **dev** branch. The **main** branch is stable. 

The **source** directory is where all the sources, build script(s) and CMake rlated source files are. (CMake runtime files are created mostly elsewhere - see the **build** directory mentioned below).

The script **base-linux-build.bash** in this (**source**) directory builds **everything linux**. 
After running it, the **../build** directory is created (if it does not exist) as a peer to this (**source**)
directory and it contains **ALL** the build artifacts.  Well, almost all.  Eclipse 
(if that's the IDE you use) maintains a few files that are created within this directory (under the **./build**)
directory.  I haven't found (nor looked for, actualy) a way to get it to put these files elsewhere, but 
they are handled by the **../.gitignore** configuration file, and are truly ignored by git.

Again, this is an "out of source" build environment.  The **../build** directory (not to be 
confused with various **build** directories found under this directory) can always be removed 
without affecting any source files.  

To clean everything, simply remove the **../build** directory (**./source/../build**) and rerun the **base-linux-build.bash**
script (which does exactly that before (re)building everything).

The various other directories that are base directories for the various projects included under **source**, each has its own **linux-build.bash** script which builds that particular project.  

These sources have been built and tested on Debian 11 (bullseye), with 
Eclipse version 4.21,
g++ (Debian 10.2.1-6) 10.2.1 20210110, 
and cmake 3.18.4.

This is what I'm starting with (Jan 2022).  I'm sure that as time goes by, the version numbers will
increase. 

The defaults for the various build scripts are set for the GNU tool chain running under linux, with
Eclipse.  However, the Eclipse IDE is not required. The build scripts can be run (with the 
right flags) just using g++, cmake, and (optionally) your favorite IDE if it's supported 
by the CMake version used for the build. 

If you use a different IDE, a different compiler (tool chain) - feel free change the build scripts 
and the various **cmake** source files.  Good luck...  I am available for some level of support (as
well as moral support) if you are brave enough to do that (**andrew@akelly.com**).

### Current deficiencies:

Currently the Windows' WIN32 configuration has not been built and tested yet. 
This may be added in the future.  My higher priority is to first get everything
"in" and get it running under linux.  I'm simply not there yet (not to speak of
not owning a Windows machine). 


## After Building: ##

**../build/localrun** contains all the executables and libraries needed at runtime (set your LD_LIBRARY_PATH to at least ".:") 

**../build/lib** contains all the libraries (shared and static), as well as executables (installed by Eclipse/cmake).

**../build/include** contains all the .h\* files that are needed when building from an external build environment. 










