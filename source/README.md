
# Samples
    
**For what's going on right now, please see the [root Samples README.md file](../README.md).**     
     
## The Sources and How To Build Them

This directory (**source/**) is where all the sources, build script(s) and CMake rlated source files are. (CMake runtime files are created mostly elsewhere - see the **../build/** directory mentioned below).

**SEE ALSO:** The **README.md** file in the parent folder (**../README.md**).  At the very end of this document there
is a section about how (and some of why) I use **Eclipse** the way I do.  It's not everyone's cup of tea.  

The script **base-linux-build.bash** in this (**source/**) directory builds **everything linux**. 
After running it, the **../build/** directory is created (if it does not exist) as a peer to this (**source**)
directory and it contains all the build artifacts.  Well, almost all.  Eclipse (if that's the IDE you use), 
maintains a few files that are created under this directory (in various **build/**)
directories.  I haven't found (nor looked for, actualy) a way to get it to put these files elsewhere, but 
they are handled by the **../.gitignore** configuration file, and are safely ignored by git.  I believe that if
any of these **build/** directories get destroyed by mistake, Eclipse will remake them without complaining.

Again, this is an "out of source" build environment.  The **../build/** directory (as well as any of the 
various **build/** directories found under this directory) can always be removed 
without affecting any source files.  

To clean everything, simply rerun the **base-linux-build.bash** script, which first removes the **../build/** 
directory and its contents before (re)building everything.  

**CAVEAT:**  You can almost completely ignore all I have to say about **eclipse** in the following couple of paragraphs, 
since there have been **lots** of improvements in how **eclipse** handles changes to its configuration.  It's still quirky (read buggy) 
at times, but it's much much better now.      
     
A (now tentative) word about **eclipse**... it does not handle changes to any of its configuration files (CMakeLists.txt, 
\*.cmake, etc) while it is running very well.  If you have to modify these type of files 
(as you do when running any of the ...build.bash scripts, you might want to do it from the command line without Eclipse running,
as it may get confused and ask you quite a few questions (about refreshing its files), and then try to recreate
its environment (which at least at this time it does not do very well). If this happens, simply close all editor windows
within eclipse, and shut it down.  Then rerun the **base-linux-build.bash** script again.    
     
The other thing about **eclipse**, is that when it comes up, it takes a while for the indexing to do its work.  This means, 
for example, that when you hover with the mouse over an object name in the source, it may tell you that it does not recognize it
even though 5 minutes later, it will.  It's just the way it is (for now).  (**Note:** this has also improved, but it still remains a 
mystery to me why it indexes some files, and not others, as well the timing of such index updates.  This is still an annoyance (to me) but refreshing the files, and then rebuilding the index usually does the trick).     
      
The various other directories that are base directories for the various projects included under **source**, each has its own **linux-build.bash** script which builds that particular project (with exception of the **QtApps** directory).  

These sources have been built and tested on Debian 11 (bullseye), with 
Eclipse version 4.21 (and later with 4.22),
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


## After Building: ##

**../build/localrun** contains all the executables and libraries needed at runtime (set your LD_LIBRARY_PATH to at least ".:") 

**../build/lib** contains all the libraries (shared and static), as well as executables (installed by Eclipse/cmake).

**../build/include** contains all the .h\* files that are needed when building from an external build environment. 

Make sure that in linux, you have the LD_LIBRARY_PATH variable include either **".:"** or **"../lib:"** in order for the runtime environment to find shared libraries that have to be loaded to run any of the executables.  


## So The Thing With Eclipse ##

Earlier in my career I spent a few choice months trying to get CMake, Qt, Visual C++, and Eclipse (using either/or Windows or Linux) to play together in some sane fashion.  As those months progressed, I slowly came to the realization that the technology was simply not there yet. Admittedly this took place almost twenty years ago, and I'm well aware of how much further along all of these technologies have progressed.  But it's still not all quite there.  This is not a rant, so I'm not getting into details.

I love working with Visual Studio, I love working with Qt designer, qtcreator, and the weird and wonderful build environment they have. I love working with Eclipse (my favorite IDE).  Just not all together.      
      
**Later update:** as of Qt 6.2 (the version I'm currently using on Linux) I've noticed vast improvements in how **qtcreator** handles the CMake build environment -- which is the main reason why I'm trying out using **qtcreator** instead of Eclipse for the Qt apps.  So far, at the level of projects I"m dealing with, it does a superb job with CMake (currently seems to me to be "better" than Eclipse). However, none of the above is objective.  I fall in and out of love with technology on a dime, and fairly frequently at that.      

Focusing on Eclipse running on Linux here, my solution to the challenge has been to use CMake as the primary "build tool", with Unix Makefiles on the Linux side, and at this time, it is the "glue" I use for setting up the **qtcreator** projects I'm creating.  Stay tuned...  

My approach is to drive the "heavy" build tasks from shell scripts (bash), and get CMake to create both the Linux and Visual Studio solutions.  Hence the **...linux-build.sh** scripts you will find here - that's where I typically solve problems (under Linux). 
Again - currently, the **QtApps** projects are the exception, but we shall see.     


### Consequences for the workflow: ###

With these projects, don't use Eclipse for modifying the CMake files in these projects, including adding new directories/projects.   Exit eclipse, and when you're ready, run the **base-linux-build.bash** script from the directory where it resides (**...Samples/source**).

That's about the only limitation I introduced on using Eclipse in this workflow.

Editing files, adding .cpp and/or .h\* files to existing source directories, rebuilding, installing, running the debugger - all work as they should.  Until you have to modify a CMakeLists.txt file or change some configuration aspect in a .cmake file (in the **cmake/** directory). With Eclipse, you have to add the source files directly into the sources in the Ecplise Explorer tab, by right-clicking on the directory name you want the file to reside in, and then choosing **"New"** on the drop-down menu that shows up.    
     
But if you don't need to do that, you can save all your files, and, do the following:

In the Eclipse Project Explorer pane, under the Samples project, expand the **sample-Debug@build** item, and then expand the **Build Targets** item. You can right-click on any of - "all", "clean", "install", "install/local", or "install/strip" and choose **Build Target** option in the drop-down menu that appears (when you right-click).  I haven't used any of the other options that are shown, so I don't have much insight as to what would happen if you picked them.

This works reliably and can be trusted. 

**To view/edit your source files in Eclipse**, Expand the **Source** section in the project explorer to get to all the source directories included in your projects. 

The build itself is set up (by CMake) to handle build files outside fo the **Samples/source** directory.  If that were not done, Eclipse, as of some version past, complains about the build directory being under the source directory.  This causes it problems as it runs.

File indexing is done in the background as soon as Eclipse starts running.  It takes a while (a few minutes) to complete, and has some visual side-effects during your editing session (auto-complete, etc).  It is what it is.  

I tend not to use source code control (git) from within Eclipse, since it tends to slow things down considerably, especially if you have a slow network connection.     

**SEE ALSO:**    

The [README.md file in the root Samples folder](../README.md).    
The [README file in the Video project](Video/README.md).     
The [README file in the Video/src/plugins project](Video/src/plugins/README.md).        
    
The file [LICENSE](../LICENSE) in the root **Samples** folder, contains the legal language covering distribution and use of the sources in this project (that belong to me).    
The file [LICENSE](3rdparty/JsonCpp/JsonCpp-8190e06-2022-07-15/jsoncpp/LICENSE) covering **JsonCpp**.    
The file [LICENSE.txt](3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8-modified/LICENSE.txt) covering **LoggerCpp**.     
     


