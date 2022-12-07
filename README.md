# Samples

## Going on now:   

**Done with the following projects:**     
     
 * The [README file in the Video project](source/Video/README.md) is complete in its current form.  This is a  multi page (multi-screenful) document full of useful information for those who are interested in using the Video capturing and processing of raw image buffers.  This is recommended reading if you want to wade into these waters. 

 * Done with the restructure of **main_video_capture** at the previous level of functionality that existed in *main_v4l2_raw_capture* (which is gone), and then some.    
     
* Done with the restructure of the Video project source files (as well as the associated *CMake* files.   
        
* There is no more **C** code in this project (previous interface to *V4L2*).  It's all **C++** now.     
     
* Fully integrated **JsonCpp** into the code.  Only used in some applications at this time.    
    
* Revamped the command line parsing utility (see CommandLine.cpp/.hpp), as well as the programs that use it.     
      
* Completed the capability of **main_video_capture** to stream out video frames to a process. 
This is controlled by the JSON config file (*video_capture.json*). Highly recommend taking a look at 
the [README](source/Video/README.md) in the **Video** project. 


 - one has to both enable the pipe to *popen()*, as well as specify the command line of the process to be piped to. Currently the command **"dd of=video_capture.dd.data 2> /dev/null"** is being used.  This allows testing if both the **write-to-file** as well as the **write-to-process** capabilities are enabled in the JSON file.  The *.data* file created by **write-to-file** has to be identical to the *.dd.data* file created by the **dd** utility (see **"output-process"** in *video_capture.json*). Which it is.     
     
**Next Steps**:    
    
1. Focus on the video frame grabber using opencv (currently planning on **opencv version 4.5.1**) in addition to **V4L2**. It was originally planned to use opencv version 4.6.0, however 4.5.1 is available through the **apt* utility from the Debian repositories with no relevant loss of functionality (currently using Debian 11 bullseye**).     
     
2. Add a Qt 5 program that will display grabbed frames from either video interface (v4l2 or opencv), change dynamic parameters in the **main_video_capture** interface and observe changes in performance. This will include writing out new JSON files to take snapshots of configuration.    
     
Both **main** and **dev** branches are now in sync. Always take **dev** before **main**.  However, **main** is more stable.    
     
The **README** files are still lagging behind reality by a bit.     
     
# Welcome.  
     
If you are here to look at how to get things done (one way, anyways) or you are here to evaluate my skills for whatever purpose,
you might want to start with either libraries of objects, or with some main programs that use them, and work your way out from there.
I'll be updating the README files as I go along, perhaps a step or two behind the software being developed.  

The underlying structure of the Samples project has to do with needing to serve as a cross-platform solution.  You will see references to both Windows (Visual Studio, C++, etc), as well as Linux with various build tool chains - C++, Make, CMake, Eclipse, and more - used for building user interface objects and tools.    
     
Those serve as the rationale for basing the whole Samples project on CMake. which seems to be the single most flexible tool which could help drive cross-platform as well as a cross-technology development infrastructure.  You will see references to all that, but in fact the real software and build environment that you see here, as well as the tools to "get it done" starts out being geared towards CMake, Eclipse, and the Gnu tool-chain under Linux.   
    
Not having a team of developers working on this at my disposal, I'm trying to get to creating the code first, then testing, with documention in the README's and comments in the code.     
     
I am not an expert with Java, Python, and cloud based infrastructures that help 
large development teams for large corporate technical needs
that one person (me) cannot possibly deal with.  
    
Yes, I am dating myself here - but please, do not waste your time if that is what you  need.     
     
## A collection of C++ objects (libraries), and some executable programs that utilize those objects.

The sources are organized in a source tree which is built using cmake (minimum version 3.10). There are (and will be) additional README files in the subdirectories going into more detail of how to use the build environment.  

New work is checked into the **dev** branch. The **main** branch is fairly stable. 

See the **source/README.md** file for detail about the file/directory structure, build tools, and how to build the sources. 

This is an "out of source" build environment. Mostly.  Check the **./.gitignore** file to see what's ignored. The **build** directory can always be removed without affecting any source files.  

Even though the IDE and build environment are set here as the defaults (eclipse, gnu toolchain), the Eclipse IDE is not required.     
     
      
## Intetesting main programs:

These programs are used to test objects, as well as to stress-test them.  They don't always have a useful purpose beyond that, except that 
if the reader is looking for a focused project that can help them solve real problems they face, this is not a bad way to start.    
     
**main_video_capture.cpp**    
found *...Samples/source/Video/src/main_programs/*    
(used to be *main_v4l2_raw_capture*)     
     
The sources for this project are being restructured at this time. But it already works (you can take a video on your camera, stream it to a file, convert it to an mp4 file (or any other format) and then play it on your favorite video viewer (vlc?).     
     
This is an infrastructure that pumps out video frames from the hardware (USB camera in my case) and then gets rid of these frames (by passing them on) to software that deals with each frame - either saving them in a file, analyzing/modifying each frame, and/or displaying them.    
      
Thanks to the online video community which speaks a different language and uses tools and designs that are a step or three beyond what I know, I'm using a C++ object at the heart of the video frame "pump" that originated from a C program which is published by kernel.org as part of the Linux kernel device driver documentation.  That code, now morphed into a C++ object interfaces betwen the V4L ("Video For Linux" - V4L2 in this case) interface to the hardware, and upper levels of the software (C++ objects, Qt, etc). This is part of software which is provided freely with the V4L2 API.      
     
     The actual C++ object used is based on the *v4l/capture.c* sample program copied 
     from *kernel.org*. See the directory ...Samples/References in this project for the 
     full detail of how to get to the file today, as well as a copy of the source in its 
     current raw (unmodified) form.      

The (now a) C++ class uses a set of configurable parameters (now read from the JSON config file, as well as adjusted by command line parameters) to fill up to 4 memory mapped buffers with video data in YUYV format (Packed YUV 4:2:2, YUY2) or H264 (H264 with start codes).  These buffers, one at a time as they are filled, are passed on to the next level up (written to a file, used to display in a viewer, analyzed, etc). Each buffer is copied only once, after which a shared pointer is what gets passed from one queue to another for further processing.  This frees up the memory mapped buffer (one of the 4) which allows the V4L driver in the kernel to fill it up with a new frame.
 
With h264 pixel format, I'm currently seeing frame sizes of up to 175Kb each using my webcam which 
provides 1920x1080 pixels per frame. Most frames, though, are less than 50Kb each). The numbers will be exponentialy higher for cameras that have a higher resolution.    
        
Currently all of this is implemented and working all the way up to and including writing out the video data to a file.  This file can be viewed by any of several viewers that handle YUYV or H264 formats. In order to see the results from the file in a viewer, you can do this:    
     
For h264 pixel format:     
     
    $ ffmpeg -f h264 -i video_capture.data -vcodec copy main_video_capture.mp4    
    
Or, for yuyv pixel format:     
     
    $ ffmpeg -f rawvideo -vcodec rawvideo -s 640x480 -r 25 -pix_fmt yuyv422  \
             -i video_capture.data -c:v libx264 -preset ultrafast -qp 0 main_video_capture.mp4
     
Where the *video_capture.data* is the name of the raw data file created by **main_video_capture**, and the .mp4 file is the output from *ffmpeg*. Having done this, you can view the video with any viewer you normally use, for example:   
    
    $ vlc ./main_video_capture.mp4        
    
I use this to test the sanity of the layers below the thread which is handling the frame buffers.       
   
Obviously this is a Linux-only solution.  The Windows-only solution for grabbing frames will have to be considerably different (and is not being addressed at the moment).   
    
Run time profiling implemented in this set of objects, gathers stats of various factors in this mechanism that control how well the whole mechanism works:  various configurable hardware parameters (the V4L2 interface), the number of frame buffer pointers in the ring buffer (the queue size), the current frame rate, etc.   

(Current results: the average frame rate is showing as more than 25 fps (with the driver set to deliver at 25fps).  On the average there are 0 shared_ptr's in the ring buffer:  When I inserted some sleep()'s here and there to simulate load, those number came up into the few dozens depending on which part of the system I slowed down.)
    
    
**main_ntwk_basic_sock_server.cpp** and **main_client_for_basic_server.cpp**   
found in *...Samples/source/EnetUtil/src/main_programs/*     
     
Multiple instances of the client can be started in parallel, each sending the contents of some (any) file to the 
server, which writes the data into its own copy of the file, and reports back to the client that it succeeded.  
Enough information is written to the server log file to enable the user to compare each of the resulting server files to its original client side copy, to ensure that no files were left out and that the content it identical (there are potentially hundreds of output files involved in any real live test).  The server is heavily multi-threaded. Each accepted connection starts a new thread in the server to handle the connection and to receive the contents of the one file that the connection handles reliably. 
The basis for handling the data is the **fixed_size_array** object (see *NtwkFixedArray.hpp* below) which encapsulates an *std::array<>* object (see *NtwkFixedArray.hpp* below). All communication of data is based on this fixed size container.    
    
In the same source directory the script **test_basic_server.bash** runs one test scenario which requires various data files (that are not checked in to this repository - some can be rather large). The script setup and use is documented at the beginning of the source file in a comment.    
    
**NtwkFixedArray.hpp**     
found in *...Samples/source/EnetUtil/include/*    
     
This is a template based object which encapsulates an std::array<T,N> which is where the data is kept for most of the network based
objects covered here. The current implementation uses an std::array<T,N> where **T** is **uint8_t** (for std::array<uint8_t,N>), and **N** is the size of the std::array<> (number of elements). In addition to other facets, it demonstrates a decent implementation of how to use the **shared_from_this()** mechanism properly.  By having **shared_ptr<>**'s holding fixed size buffers, a program (like **main_ntwk_basic_sock_server.cpp** in this case) can move the data around by moving the **shared_ptr<>**'s around instead of having to copy the data from one array buffer to the next.  Worth a look. (Who uses std::array<>'s anyways?)     
     
**main_condition_data.cpp**      
found in *...Samples/source/Util/src/main_programs*    
     
Exercises the condition_data object extensively for use from multiple threads. The program stretches the number of multipe threads to
a real large number (30,000 in one successful case on my system). This program was used while testing the **LoggerCpp** library at the time when it was eveluated for use in these sources.  This meant writing lots of log lines of output from 30,000 threads concurrently, into a single log file without any of the lines being broken, nor losing any data.  (I settled on **LoggerCpp** for use in these sources after that little experiment - see the *3rdparty* directory for more info on the package).

**main_circular_buffer.cpp**    
found in *...Samples/source/Util/src/main_programs*   
     
Exercises the ring buffer and shows functionality.  This is a great mechanism to use with **std::shared_ptr<>**'s.     
     
**main_enet_util.cpp**     
found in *...Samples/source/EnetUtil/src/main_programs* (see also *...Samples/source/EnetUtil/src/EnetUtil.cpp* and *...Samples/source/EnetUtil/include/EnetUtil.hpp*)     
      
Shows how to iterate through all of a system's network links and obtain useful information from them (ip address, family type, interface type, mac address, etc), without resorting to use of command line utilities embedded in the c++ code.  Also shows how to obtain a system's primary interface to the internet - including the system's self ip address and mac address.     
     
     
**Includes:**     
     
The following **include/** directories include the declaration of the objects used in the above programs:    
     
*...Samples/source/Util/include/*     
*...Samples/source/Video/include/*     
*...Samples/source/EnetUtil/src/main_programs/include/*     
*...Samples/source/EnetUtil/include/*     
     
See also the 3rd party packages' directories:     
*...Samples/source/3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8-modified/include/*     
*...Samples/source/3rdparty/JsonCpp/JsonCpp-8190e06-2022-07-15/jsoncpp/include/*     
     
     
**Shell scripts:**    
     
base-linux-build.bash (in *...Samples/source/*)   
linux-build.bash   (in *...Samples/source/EnetUtil/*)    
linux-build.bash   (in *...Samples/source/Util/*)     
linux-build.bash   (in *...Samples/source/Video/*)     
bash_env.sh (in *...Samples/source/shell_env/*)   
test_basic_server.bash (in *...Samples/source/EnetUtil/src/main_programs/*)     
localbuild.sh  (in *...Samples/source/3rdparty/LoggerCpp/* and in *...Samples/source/3rdparty/JsonCpp/*)     
     
                
**CMake Files:**      
     
*cmake/EnetUtil.cmake*   
*cmake/tools.cmake*   
*cmake/Util.cmake*        
*cmake/Video.cmake*    
*CMakeLists.txt*   
*EnetUtil/CMakeLists.txt*   
*Util/CMakeLists.txt*    
*Video/CMakeLists.txt*    
     
      
**The flow:**    
     
The shell scripts create the file structure and shell environment for CMake to run.   
The CMake files set up **cmake** to create an **Eclipse IDE** project using **make** for building the code.   
**Eclipse** does all the rest...   
   
   __________________   
   
    
    
# Please Note:
This is work in progress -- I'm writing code and uploading the sources to the repository while ensuring that everything is tested, building properly (at least on my system), and working. So for a period of time I've got restrictions on interactions with the repositories that allow one to view, clone and/or otherwise download the code (to which you are welcome as per the LICENSE) but I am not yet welcoming collaborators. Right now, even though I no longer have just some basic libraries and main programs that use/exercise them, there are still more objects coming.  As soon as I introduce enough code that is mostly stable, I'll remove the restrictions.  In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.     
     
Thank you.     
     
**SEE ALSO:**    

The [README](source/README.md) in the **source** folder.    
The [README](source/Video/README.md) in the **Video** project.     
The [LICENSE](./LICENSE) in the root **Samples** folder covering the entirety of the **Samples** project..    
The [LICENSE](source/3rdparty/JsonCpp/JsonCpp-8190e06-2022-07-15/jsoncpp/LICENSE) covering **JsonCpp**.    
The [LICENSE](source/3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8-modified/LICENSE.txt) covering **LoggerCpp**.     
    
### Current deficiencies:

Currently the Windows' WIN32 configuration has not been built and tested yet. 
This may be added in the future.

