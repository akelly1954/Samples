
# Welcome.

This project includes software I've developed in the past year or so (since Jan 2022), as well as some rewritten older code 
from prehistoric times.     
      
Whereas my original intent was to release the project into the Github mainstream, I now realize, a year++ later,
that I have no intention of releasing any of it.  I mostly use Github to keep track of development, and as 
backup for those times when I mess up and need to push "reset".  You are welcome to view, copy, or clone the
software as you need to.  Feedback/questions are welcome - email **andrew@akelly.com**.  I do not accept
collaborators for the repository, therefore your interactions with it would be limited ("look, copy, use, but no touching...").     
     
I do hope that people who view this can use some of the ideas and code for their benefit.  The code I write is
all licensed under the **MIT** (very very permissive) license, and the only thing to check is for 3rd party software
used (logger, json parsing, etc) which might have a different license.  Now that some **Qt** (version 6) is being 
used, the appropriate Qt Copyright license should be understood and respected as well.  It's all very clearly documented in the 
source tree as well as embedded in various files in the code.     
     
Full disclosure for those who are looking for how to do things in a specific language and/or operating system: 
I am not an expert with Java, Python, Perl, .NET, current embedded toolsets, nor cloud based infrastructures that 
help development teams for large-scale corporate technical needs that one person (me) cannot possibly deal with. 
And although I'm great with C++ and Linux/Unix technologies  in general, 
not so much with Windows infrastructure and tools (although I do like working with Visual C++).   
    
Most of what you find here is Linux/Unix based, using Eclipse, CMake, and the **gnu** toolchain (g++ etc) for development. 
Having boasted more than once that I could bake a cake using Eclipse if I had to, it is not actually necessary for all developers. 
The key with this code is **CMake**, where using the "-G" flag you can specify which compiler/toolset to use.   If you do that, 
there are some shell scripts that would have to be modified as well (look for scripts whose name ends with ...linux-build.bash, 
as well as source/Samples/shell_env/).
    
*Apologia*...  Finally, it would be untruthful of me to claim that the main goal of this project is to benefit others. 
I started this project after my (two-year) sabbatical where I did not touch technology at all.  I then needed to see 
how I felt about doing the work I performed for decades in the software industry.  I needed to come up to speed on 
current technologies, remind myself how to spell C++, and see how I felt about it all.  Conclusion:  I loved it, and still
do. A lot. And taking a break from corporate culture didn't hurt either.    
    
My plans for this project change almost daily - I'm governed by my own sense of creativity, and not by what 
others might consider to be "logical".  I like to be creative, and that inspiration comes with its own logic.    

There's a section below which outlines what's going on right now, and what has been completed recently. 
These are speculations about the future which may not turn out to be true to reality.  These are definitely
not promises nor commitments.   
    
There are other README files listed below as well. Please have fun with it all.     
    
## Going on now:   

**Ongoing task:** Updating README files and doing some cleanup. It will be done when it's done.   

**And...**   

* Enhancing the raw queue (ring buffer of shared_ptr<>'s) thread to split out each each major functionality 
required of the raw queue thread (writing to file, writing to an external process like ffmpeg, or sending the video 
frame's shared_ptr<> to a consumer app like a GUI based video player) into their own threads.   

* Developing a video streaming app in Qt6 (**VideoCapturePlayer**) relying on the video capture project. 
For now this is merely a sandbox for ideas and trying things out in the **dev** branch.  But it's beginning
to look like a real app.   

As a sidenote, to see how that project (**VideoCapturePlayer**) is progressing, see *.../source/QtApps/src/VideoCapturePlayer*. 
The project is built by **QtCreator** with **cmake** (using **ninja**, not **make**) and does not participate in the 
complete build of the **Samples** project.  To build it, you have to go to the above directory, and run **qtcreator** 
(after Qt6 and all other requirements have been installed).  Currently the app looks great, and is fully functional except for... ~~actually showing the streamed video~~ -- never mind that - it's showing video finally - from a file, not video capture.  That's the part I'm working on right now (actual streaming of captured video) (3/9/2023). 
     
      
**Done with the following projects:**     
     
* Finished migrating the video capture objects (all the way to the Qt apps) to use the new data containers described 
below instead of the std::array<> based objects the video capture objects were previously using.   

* Completed the objects declared/defined in **data_item.hpp** and **shared_data_items.hpp** (path: *source/Util/include/*). 
these are described in detail below - a way manage large amounts of data (say, video frames) and move the data around the system 
without having to copy (much).   

* Modified the Video project to use **plugins** for dealing with the different streaming interfaces (currently **v4l2** - next, **opencv**).  This involved refactoring some of the design, and much of the code.  First and formoset, the **v4l2** interface was implemented as a plugin, and next comes OpenCV - it is constantly getting pushed down in the priority list - my apologies. At startup, the main program (main_video_capture) loads the interface plugin as per the JSON config file, and it will error-exit if anything goes wrong. The interface for plugins is defined by [README.md](source/Video/src/plugins/README.md) under source/Video/plugins/.      

* The [README file in the Video project](source/Video/README.md) is complete in its current form.  This is a  multi page (multi-screenful) document full of useful information for those who are interested in using the Video capturing and processing of raw image buffers.  This is recommended reading if you want to wade into these waters. (Some of the sections are slightly out of date - but do not impede forward movement.  I will be updating this doc gradually over time).     
         
**Next Steps**:    
    
1. Further development of the video capture plugin interface documentation - [source/Video/src/plugins/README.md](source/Video/src/plugins/README.md).  The document covers just about everything (Jan 2023), but will be fleshed out with more details in the future.     
     
2. Add a Qt5 program (ongoing - now Qt6) that will display grabbed frames from either video interface (v4l2 or opencv), change dynamic parameters in the **main_video_capture** interface and observe changes in performance. This will include writing out new JSON files to take snapshots of configuration.    
     
The **README** files are still lagging behind reality by a bit.     
     
     
# The Samples Project:  
     
## A collection of C++ objects (libraries), and some executable programs that utilize those objects.

The sources are organized in a source tree which is built using cmake (minimum version 3.10). There are (and will be) additional README files in the subdirectories going into more detail of how to use the build environment.  

New work is checked into the **dev** branch. The **main** branch is fairly stable. 

See [source/README](source/README.md) for detail about the file/directory structure, build tools, and how to build the sources. 

This is an "out of source" build environment. Mostly.  Check the **./.gitignore** file to see what's ignored. The main **build/** directory  as well as any **build/** directory found within the source tree can always be removed without affecting any source files.  

Even though the IDE and build environment are set here as the defaults (eclipse, cmake, gnu toolchain), the Eclipse IDE is not required (see the **-G** flag for cmake).     
     
      
## Intetesting programs and objects:

These programs are used to test objects, as well as to stress-test them.  They don't always have a useful purpose beyond that, except that if the reader is looking for a focused project that can help them solve real problems they face, this is not a bad way to start.    

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**data_item.hpp** - contains the *data_item_container<T>* object.   
**shared_data_items.hpp** - contains the *shared_data_items<T>* object.      
Path: *source/Util/include/*   
   
These objects have been created to overcome difficulties encountered with the practical use of 
**std::array<>**-based objects. To avoid expressing vague but polite explanations, I'd prefer to be 
blunt: Having to define an object with a fixed size, particularly when the object is passed 
around various 
parts of a program (and even between programs) practically forces the consumer of the data object 
into having unreasonable data size requirements.  The basic requirement of always declaring an 
array object with a predetermined size which is well known at compile-time, dictates requirements on consumer objects which are, 
say, attempting to use the array based object found in a 3rd party library - even years after 
it was written. At compile time.   

I'm well aware of efforts to circumvent the C++ compiler's enforcement of its tight and strict policy regarding the declaration of **std::array<>**, but have found little merit in designing a complete set of tools whose goal is to circumvent a somewhat major c++ language requirement.   

So the idea is that the *data_item_container<T>* manages and handles a sequence of typname T objects in a container, and the number of items in the container is determined at runtime separately from any requirement akin to those enforced with C++ std::array<>'s.    

This (the dynamic number of items in a collection) could well be solved by using say, **std::vector<T>** objects, but those objects are geared for frequent editing of the data - adding, appending, inserting and resizing its internal data objects.  The need satisfied by the two objects defined here is how to simply use a container with large amounts of data, without having to do too much manipulation of the data itself, but simply to move it around the system using shared_ptr<T>'s.   

By encapsulating the *data_item_container<T>* object in a shared_ptr<T> based *shared_data_items<T>* object, convenient management of the data within the data_item_container is made easy. The shared_ptr<> is assignable, copyable, delete'able, and most of all will prevent unneccessary copying and recopying of the data. Runtime typing is enforced, with no memory leaks, while being able to use the shared_ptr<> within STL objects (std::vector<>, std::map<>, etc) as well as std:: objects in general (std::copy(...), std::sort(...), etc).   

Prologue over...    

The two objects introduced here are designed to be "moved around" the running program using std::shared_ptr<T>'s. The *data_item_container<T>* is not meant to be used standalone (without using the shared_ptr<> pointing to it), although it can be: There are no restrictions enforcing the use of 
a *data_item_container<T>* solely within an accompanying *shared_data_items<T>* object.  It's just not useful IMO.   

Both objects are template based, where typename T is the underlying type of the data.  Although currently both objects are used with T = uint8_t (unsigned char) from within programs that use them, 
the T type can be anything that complies with common std:: and STL requirements (int32_t, double, std::vector<T>, class whatever, etc).   

**Note**: Currently these objects are not yet tested with *std::string*. This is a **TODO** item. 
(3-17-2023)   

The *data_item_container* object holds a sequence of "typename T" items.  The object handles creation, assignment, copy and move semantics, deletion, and access (to each element).

The *shared_data_items* object encapsulates (as opposed to "derives from") a *data_item_container* object, and exposes most of *data_item_container*'s capablities and data to the outside world.  In addition, it derives from **std::enable_shared_from_this<>**, and so it manages the creation of **std::shared_ptr<>**'s to the object as well as all other reasonable capabilities (see the source).   

**Note:** the *shared_data_items* object currently disallows **move semantics** altogether (even though the *data_item_container* does not).  These will be implemented later if appropriate). 

**Note:** the *shared_data_items* object is a good example of how to use the std:: **shared_from_this** construct properly (i.e. avoid having a dangling copy of the shared_ptr<> which is not included in the reference count in the shared_ptr<> upon deletion).    

The basic test for these objects is in **source/Util/src/main/programs/main_util_combo_objects.cpp**.  It tests all the basic objects' creation and capabilities, but still **TODO:** additional tests, including test of multi-threaded operations). 
      
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**main_video_capture.cpp**    
found in *...Samples/source/Video/src/main_programs/*    
     
This is an infrastructure that pumps out video frames from the hardware (USB camera in my case) and then "gets rid" of these frames (by passing them on) to software that deals with each frame - either piping it to a linux process (ffmpeg, vlc, etc), and/or saving each frame in a file, and/or analyzing/modifying each frame, and/or displaying them.    
      
In other words, you can take a video on your camera, stream it to a file or a linux process, convert it to an mp4 file (or any other format) and then play it on your favorite video viewer (vlc?).     
     
Thanks to the online video community which speaks a different language and uses tools and designs that are a step or three beyond what I know, I'm using a C++ object at the heart of the video frame "pump" that originated from a C program which is published by kernel.org as part of the Linux kernel device driver documentation.  That code, now morphed into a C++ object interfaces betwen the V4L ("Video For Linux" - V4L2 in this case) interface to the hardware, and upper levels of the software (C++ objects, Qt, etc). This is part of software which is provided freely with the V4L2 API.      
     
     The actual C++ object used is based on the *v4l/capture.c* sample program copied 
     from *kernel.org*. See the directory ...Samples/References in this project for the 
     full detail of how to get to the file today, as well as a copy of the source in its 
     current raw (unmodified) form.      

The (now a) C++ class uses a set of configurable parameters (now read from the JSON config file, as well as adjusted by command line parameters) to fill up to 4 memory mapped buffers with video data in YUYV format (Packed YUV 4:2:2, YUY2) or H264 (H264 with start codes).  These buffers, one at a time as they are filled, are passed on to the next level up (written to a file, used to display in a viewer, analyzed, etc). Each buffer is copied only once, after which a shared pointer is what gets passed from one queue to another for further processing.  This frees up the memory mapped buffer (one of the 4) which allows the V4L driver in the kernel to fill it up with a new frame.
     
Obviously this is a Linux-only solution.  The Windows-only solution for grabbing frames will have to be considerably different (and is not being planned at the moment).   
    
Run time profiling implemented in this set of objects, gathers stats of various factors in this mechanism that control how well the whole mechanism works:  various configurable hardware parameters (the V4L2 interface), the number of frame buffer pointers in the ring buffer (the queue size), the current frame rate, etc.   

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**Class ConfigSingleton**     
found in *...Samples/source/Util/include/ConfigSingleton.hpp*    
found in *...Samples/source/Util/src/ConfigSingleton.cpp*    
Used from *...Samples/source/Util/src/main_programs/main_jsoncpp_samplecfg.cpp*     
Used from *...Samples/source/Video/src/main_programs/main_video_capture.cpp*    
    
This object reads, parses, validates, and updates **JSON** files that deal with configuration (and could be used for other purposes).  It is implemented as a **Singleton**, and demonstrates one of the better ways to use the **std::shared_from_this<>** base class (there are multiple ways to do this).   
    
(**Deficiencies** - at this time (Jan 2023) this object has not been put to strenuous use yet in this project.  I will continue to 
develop it in the future as needed by new applications).     

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**main_ntwk_basic_sock_server.cpp** and **main_client_for_basic_server.cpp**   
found in *...Samples/source/EnetUtil/src/main_programs/*     
     
Multiple instances of the client can be started in parallel, each sending the contents of some (any) file to the 
server, which writes the data into its own copy of the file, and reports back to the client that it succeeded.  
Enough information is written to the server log file to enable the user to compare each of the resulting server files to its original client side copy, to ensure that no files were left out and that the content is identical (there are potentially hundreds of output files involved in any real live test).  The server is heavily multi-threaded. Each accepted connection starts a new thread in the server to handle the connection and to receive the contents of the one file that the connection handles reliably. 
The basis for handling the data is the **fixed_size_array** object (see *NtwkFixedArray.hpp* below) which encapsulates an *std::array<>* object (see *NtwkFixedArray.hpp* below). All communication of data is based on this fixed size container.    
    
In the same source directory the script **test_basic_server.bash** runs one test scenario which requires various data files (that are not checked in to this repository - some can be rather large). The script setup and use is documented at the beginning of the source file in a comment.    

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**NtwkFixedArray.hpp**     
found in *...Samples/source/EnetUtil/include/*    
     
This is a template based object which encapsulates an std::array<T,N> which is where the data is kept for most of the network based
objects covered here. The current implementation uses an std::array<T,N> where **T** is **uint8_t** (for std::array<uint8_t,N>), and **N** is the size of the std::array<> (number of elements). In addition to other facets, it demonstrates a decent implementation of how to use the **shared_from_this()** mechanism properly.  By having **shared_ptr<>**'s holding fixed size buffers, a program (like **main_ntwk_basic_sock_server.cpp** in this case) can move the data around by moving the **shared_ptr<>**'s around instead of having to copy the data from one array buffer to the next.  Worth a look. (Who uses std::array<>'s anyways?)     

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**main_condition_data.cpp**      
found in *...Samples/source/Util/src/main_programs*    
     
Exercises the condition_data object extensively for use from multiple threads. The program stretches the number of multipe threads to
a really large number (30,000 in one successful case on my system). This program was used while testing the **LoggerCpp** library at the time when it was eveluated for use in these sources.  This meant writing lots of log lines of output from 30,000 threads concurrently, into a single log file without any of the lines being broken, nor losing any data.  (I settled on **LoggerCpp** for use in these sources after that little experiment - see the *3rdparty* directory for more info on the package).

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**main_circular_buffer.cpp**    
found in *...Samples/source/Util/src/main_programs*   
     
Exercises the ring buffer and shows functionality.  This is a great mechanism to use with **std::shared_ptr<>**'s.   

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**main_enet_util.cpp**     
found in *...Samples/source/EnetUtil/src/main_programs* (see also *...Samples/source/EnetUtil/src/EnetUtil.cpp* and *...Samples/source/EnetUtil/include/EnetUtil.hpp*)     
      
Shows how to iterate through all of a system's network links and obtain useful information from them (ip address, family type, interface type, mac address, etc), without resorting to use of command line utilities embedded in the c++ code.  Also shows how to obtain a system's primary interface to the internet - including the system's self ip address and mac address.     

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**Includes:**     
     
The following **include/** directories include the declaration of the objects used in the above programs:    
     
*...Samples/source/Util/include/*     
*...Samples/source/Video/include/*     
*...Samples/source/EnetUtil/src/main_programs/include/*     
*...Samples/source/EnetUtil/include/*     
     
See also the 3rd party packages' directories:     
*...Samples/source/3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8-modified/include/*     
*...Samples/source/3rdparty/JsonCpp/JsonCpp-8190e06-2022-07-15/jsoncpp/include/*     

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**Shell scripts:**    
     
base-linux-build.bash (in *...Samples/source/*)   
linux-build.bash   (in *...Samples/source/EnetUtil/*)    
linux-build.bash   (in *...Samples/source/Util/*)     
linux-build.bash   (in *...Samples/source/Video/*)     
bash_env.sh (in *...Samples/source/shell_env/*)   
test_basic_server.bash (in *...Samples/source/EnetUtil/src/main_programs/*)     
localbuild.sh  (in *...Samples/source/3rdparty/LoggerCpp/* and in *...Samples/source/3rdparty/JsonCpp/*)     

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**CMake Files:**      
     
*cmake/EnetUtil.cmake*   
*cmake/tools.cmake*   
*cmake/Util.cmake*        
*cmake/Video.cmake*    
*CMakeLists.txt*   
*EnetUtil/CMakeLists.txt*   
*Util/CMakeLists.txt*    
*Video/CMakeLists.txt*    

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**The flow:**    
     
The shell scripts create the file structure and shell environment for CMake to run.   
The CMake files set up **cmake** to create an **Eclipse IDE** project using **make** for building the code.   
**Eclipse** does all the rest...   

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

# Please Note:
This is work in progress -- I'm writing code and uploading the sources to the repository while ensuring that everything is tested, building properly (at least on my system), and working. There are no Github "collaborators" established for the project (that would happen only in rare cases). In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.     
     
Thank you.     

<hr style="border: 0.3px solid lightgray; width:40%;"></hr>   

**SEE ALSO:**    

The [README](source/README.md) in the **source** folder.    
The [README](source/Video/README.md) in the **Video** project.     
The [README](source/Video/src/plugins/README.md) in the **Video/src/plugins** project.        
    
The [LICENSE](./LICENSE) in the root **Samples** folder covering the entirety of the **Samples** project..    
The [LICENSE](source/3rdparty/JsonCpp/JsonCpp-8190e06-2022-07-15/jsoncpp/LICENSE) covering **JsonCpp**.    
The [LICENSE](source/3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8-modified/LICENSE.txt) covering **LoggerCpp**.     
    

