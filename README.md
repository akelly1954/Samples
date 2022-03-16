# Samples

# Welcome.  

If you are here to look at how to get things done (one way, anyways) or you are here to evaluate my skills for whatever purpose,
you might want to start with either libraries of objects, or with some main programs that use them, and work your way out from there.
I'll be updating the README files as I go along, perhaps a step or two behind the software being developed.  

## Intetesting main programs: 

These programs are used to test objects, as well as to stress-test them.  They don't always have a useful purpose beyond that, except that 
if the reader is looking for a focused project that can help them solve real problems they face, this is not a bad way to start.  The underlying
structure of the Samples project has to do with needing to serve as a cross-platform solution.  You will see references to both Windows (Visual Studio, C++, etc), linux for various build tool chains - C++, Make, CMake, Eclipse, and Qt - for building user interface objects and tools. Those 
serve as the rationale for basing the whole Samples project on CMake. which seems to be the single most flexible tool which could help drive 
cross-platform as well as a cross-technology development infrastructure.  You will see references to all that, but in fact the real software and
build environment that you see here, as well as the tools to "get it done" starts out being geared towards CMake, Eclipse, and the Gnu tool-chain under Linux. 
Not having a team of developers working on this at my disposal, I'm trying to get to creating the code first, then testing, then documenting, and then one day 
perhaps get to the point where I actually own a Windows system (which I currently don't), let alone a Windows development system. 

What you will not see here, is software based on Java, Python, and cloud based infrastructure that helps large development teams for large corporate technical needs
that one person (me) cannot possibly deal with.  Yes, I am dating myself here - but please, do not waste your time if that is what you need.

**main_v4l2_raw_capture.cpp**    
found *...Samples/source/Video/src/main_programs/*    
     
This is under development even as we speak (so to speak), and will result with an infrastructure that pumps out video frames from the hardware (USB camera in my case) and then gets rid of these frames (i.e. passing them on) to software that deals with each frame - either saving them in a file, analyzing/modifying each frame, and/or displaying them (Qt).    
      
Thanks to the online video community which speaks a different language and uses tools and designs that are a step or three beyond what I know, I'm using a C program at the heart of the video frame "pump" that interfaces betwen the V4L ("Video For Linux" - V4L2 in this case) interface to the hardware, and upper levels of the software (C++, Qt, etc).  This is part of software which is provided freely with the V4L2 API (the actual C function is based on *kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c*.  It uses a configurable number of memory mapped buffers (4, in this case) that get filled up, one at a time, by the API.  This layer is implemented as a C function which uses callback functions to deliver each frame in YUYV format - Packed YUV 4:2:2, YUY2 - to the next level up, C++ objects that copy each memory mapped buffer (currently I'm seeing frame sizes of up to 175Kb each using my webcam - 1920x1080 pixels). The numbers will be exponentialy higher for cameras that have a higher resolution.    
        
Each frame buffer is copied into a C++ std::array<> object (each element represented as a uint8_t character), a std::shared_ptr<> set up for it, and from that point, the data does not have to be copied again unless and until the frame has to be converted into a different video format (by Qt, one hopes...).   Each std::shared_ptr<> object is added to a ring buffer (*Util/include/circular_buffer.hpp*) and handled, one by one, in a different std::thread (not the data itself, but the pointer object to it) - using an object which has an embedded condition variable, to avoid having to poll the ring buffer for newly available frame buffers (*Util/include/condition_data.hpp*).     
    
Currently all of this is implemented and working all the way up to and including the separate thread which peels off one frame at a time from the ring buffer, and writes all the frames that are ready, to be processed to a file, which can be viewed by any of several viewers that can handle YUYV format, albeit with some abnormalities (there's no metadata included with the frames).  This is simply a test mechanism for the sanity of the layers below the thread which is handling the frame buffers.       
   
Obviously this is a Linux-only position.  The Windows-only solution for grabbing frames will be considerably different (and is not being addressed at the moment).   
    
Currenly, I'm implementing and paramaterizing profiling done at run time of various factors in this mechanism that will control how well the whole mechanism works:  various configurable hardware parameters (the V4L2 interface), the number of frame buffer pointers in the ring buffer (the queue size), etc.  
     
As this is being built up from hardware, I pause at each layer to document and test (and fix bugs) before moving up to the next layer.  After this, it's Qt time.  That should be fun.    
    
    
**main_ntwk_basic_sock_server.cpp** and **main_client_for_basic_server.cpp**   
found in *...Samples/source/EnetUtil/src/main_programs/*     
     
Multiple instances of the client can be started in parallel, each sending the contents of some (any) file to the 
server, which writes the data into its own copy of the file, and reports back to the client that it succeeded.  
Enough information is written to the server log file to enable the user to compare each of the resulting server files to its original client side copy, to ensure that no files were left out and that the content it identical (there are potentially hundreds of output files involved in any real live test).  The server is heavily multi-threaded. Each accepted connection starts two threads in the server to handle the connection and to receive the contents of the one file that the connection handles reliably. The basis for handling the data is the **fixed_size_array** object (see *NtwkFixedArray.hpp* below) which encapsulates an *std::vector<>* object. All communication of data is based on this fixed size container.    
    
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

**Includes:** *...Samples/source/EnetUtil/include/* and *...Samples/source/Util/include/* show the definition of objects used in the above programs.  See also *...Samples/source/EnetUtil/src/main_programs/include*.

**Shell scripts:**   
base-linux-build.bash (in *...Samples/source/*)   
linux-build.bash   (in *...Samples/source/EnetUtil/*)    
linux-build.bash   (in *...Samples/source/Util/*)     
bash_env.sh (in *...Samples/source/shell_env/*)   
test_basic_server.bash (in *...Samples/source/EnetUtil/src/main_programs/*)

**CMake Files:**      
cmake/EnetUtil.cmake   
cmake/tools.cmake   
cmake/Util.cmake   
CMakeLists.txt   
EnetUtil/CMakeLists.txt   
Util/CMakeLists.txt   

**The flow:**   
The shell scripts create the file structure and shell environment for CMake to run.   
The CMake files set up **cmake** to create an **Eclipse IDE** progject using **makefile** for building the code.   
**Eclipse** does all the rest...   
   
   __________________   
   
    
    
# Please Note:
This is work in progress -- I'm uploading my sources to the repository while ensuring that the code is tested, building properly (at least on my system), and working. So for a period of time I've got restrictions on interactions with the repositories that allow one to view, clone and/or otherwise download the code (to which you are welcome as per the LICENSE) but I am not yet welcoming collaborators. Right now I only have some basic libraries and main programs that use/exercise them and there are more objects coming.  As soon as I introduce enough code that is stable, I'll remove the restrictions.  In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.

Thank you.

**SEE ALSO:** The README.md file in the **source** folder (./source/README.md).

## A collection of C++ objects (libraries), and some executable programs that utilize those objects.

The sources are organized in a source tree which is built using cmake (minimum version 3.10). There are (will be) additional README files in the subdirectories going into more detail of how to use the build environment.  

New work is checked into the **dev** branch. The **main** branch is fairly stable. 

See the **source/README.md** file for detail about the file/directory structure, build tools, and how to build the sources. 

This is an "out of source" build environment. Mostly.  Check the **./.gitignore** file to see what's ignored. The **build** directory can always be removed without affecting any source files.  

Even though the IDE and build environment are set here as the defaults (eclipse, gnu toolchain), the Eclipse IDE is not required. 

### Current deficiencies:

Currently the Windows' WIN32 configuration has not been built and tested yet. 
This may be added in the future.

**NEXT**: Read the **source/README.md** information for more info. 









