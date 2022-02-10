# Samples

## The Sources and How To Build Them

A collection of C++ objects in one or more libraries, and some main programs that utilize those objects.

The sources are organized in a source tree which is built using cmake (minimum version 3.10). There are additional README files in the subdirectories (of which this README is one) going into more detail of how to use the build environment.  

New work is checked into the **dev** branch. The **main** branch is stable. 

This directory (**source/**) is where all the sources, build script(s) and CMake rlated source files are. (CMake runtime files are created mostly elsewhere - see the **../build/** directory mentioned below).

The script **base-linux-build.bash** in this (**source/**) directory builds **everything linux**. 
After running it, the **../build/** directory is created (if it does not exist) as a peer to this (**source**)
directory and it contains all the build artifacts.  Well, almost all.  Eclipse (if that's the IDE you use), 
maintains a few files that are created under this directory (in various **build/**)
directories.  I haven't found (nor looked for, actualy) a way to get it to put these files elsewhere, but 
they are handled by the **../.gitignore** configuration file, and are safely ignored by git.  I believe that if
any of these **build/** directories get destroyed by mistake, Eclipse will remake them without complaining.

Again, this is an "out of source" build environment.  The **../build** directory (as well as any of the 
various **build/** directories found under this directory) can always be removed 
without affecting any source files.  

To clean everything, simply rerun the **base-linux-build.bash** script, which first removes the **../build/** 
directory and its contents before (re)building everything.  

A word about **eclipse**... it does not handle changes to any of its configuration files (CMakeLists.txt, 
\*.cmake**, etc) very well.  If you have to modify them (as you do when running any of the ...build.bash files),
you might want to do it from the command line without Eclipse running,
as it will get confused and ask you quite a few questions (about refreshing its files), and then try to recreate
its environment (which at least at this time it does not do very well). If this happens, simply close all editor windows
within eclipse, and shut it down.  Then rerun the **base-linux-build.bash** script again (and don't do it again...)

The various other directories that are base directories for the various projects included under **source**, each has its own **linux-build.bash** script which builds that particular project.  

These sources have been built and tested on Debian 11 (bullseye), with 
Eclipse version 4.21,
g++ (Debian 10.2.1-6) 10.2.1 20210110, 
and cmake 3.18.4.

This is what I'm starting with (Jan 2022).  I'm sure that as time goes by, the version numbers will
increase. 

The defaults for the various build scripts are set for the GNU tool chain running under linux, with
eclipse.  However, the Eclipse IDE is not required, nor is g++. The build scripts can be run (with the 
right flags) using any compiler, cmake, and (optionally) your favorite IDE if it's supported 
by the CMake version used for the build. 

If you use a different IDE or a different compiler (tool chain) - feel free change the build scripts 
and the various **cmake** source files.  As this is not for the faint of heart, I am available for some level of support 
(including moral support) if you are brave enough to do that (**andrew@akelly.com**).

### Current deficiencies:

Currently the Windows' WIN32 configuration has not been built and tested yet. 
This may be added in the future.  My higher priority is to first get everything
"in" and get it running under linux.  I'm simply not there yet (not to speak of
not having a Windows machine). 


## After Building: ##

**../build/localrun** contains all the executables and libraries needed at runtime (set your LD_LIBRARY_PATH to at least ".:") 

**../build/lib** contains all the libraries (shared and static), as well as executables (installed by Eclipse/cmake).

**../build/include** contains all the .h\* files that are needed when building from an external build environment. 










