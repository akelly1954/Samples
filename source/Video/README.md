# Video Capture 

Stream video frames from the source (a camera), through the linux driver, to (in this case) the process **main_video_capture**, which uses objects from this project (**Video**), as well as objects from the other projects in this repository.

## Some Notes

There are many many options and configurations that control how to get video from a camera to an end destination of a video pipeline.  As an example, run the help feature of the **ffmpeg** utility like this: **ffmpeg --help full**.  The scope is huge. In this project, the surface is barely skimmed, but it does a lot to demonstrate how to move data down the video pipline (currently only under linux), to be saved in a file, or piped to a cooperating application or utility (**ffmpeg** is used extensively), where it can be streamed to another process on the current system, or any other system on a connected network.  For exampele, in this project, the **main_video_player** uses **ffmpeg** internally to produce an mp4 file that any video player can pick up and display it on a screen.     
     
## Starting Slowly

To get started, let's take a look on how to run the utility with a bit of setup and a couple of exmples.  Let's assume we have a camera connected to a USB port on our system, and which we know to be working.  For example, run the **vlc** utility, and under the **Media --> Open Capture Device** menu, set the capture device to "/dev/video0" (on linux).  If the device drop-down menu does not show any entries, then the setup for the camera is not yet complete, or a few other possibilities that we're not going to cover here (disconnect/reconnect the usb line to the system, etc).  The actual device name can be something else, depending on the camera configuration.   
     
**So first**:     
     
The output shown below is from the utility run like this: **main_video_capture --help**.  Please take a look at the options to familiarize yourself with them, and we can then show a couple of examples.  Interspersed with the options are small sections of text that provide additional information about each:     
     
     $ main_video_capture --help   
            
     Usage:
     
       main_video_capture --help (or -h or help)
     
     Or:
     
       main_video_capture  [ options ]

**Usage**:  This is really an either/or situation. You either want "help" or you want to run the app with full functionality.
The following section lists all the options available from the command line - around nine or ten at this time. This list
tends to grow over time, and the README (this file) does not necessarily keep up in a timely manner. This also means that
the definitive description of the behavior of the various options lies in the source file (*video_capture_commandline.cpp*), 
and the JSON configuration file that is needed by the app in order to run (*video_capture.json*).    
     
**A note about video_capture_commandline.cpp** and the associated other source file that use it:  What goes on in there does seem a bit unwieldy. And it kind of is.  However... (and in my defense), I've been developing (improving) and moving the command line parsing and configuration from "inline" code towards usage of C++ templates and specialized template functions. You can see evidence of that in the objects that the parser uses to do its job. By the time this is done, most if not all the parsing and organization of the data at runtime, will mostly be done by a base class (see *commandline.cpp/.hpp* in the **Util** project) which will do all of the parsing work, and leave it to the caller to check values, legal limits, etc.
    
**So what is covered by the JSON file, and what is covered by command line options?**       
     
Glad you asked.  Most, if not all of the command line options are covered by the JSON configuration.  But not vice-versa. The JSON file has some complexity that is almost impossible to cover by the command line options in some sane fashion. Here's how this is managed:     
     
In the source files *video_capture_globals.cpp/.hpp* you will find the **struct vcGlobals** object in the **Video** namespace (aka *Video::vcGlobals* in the code).     
     
This *struct* encapsulates **ALL** the runtime options that the app uses. The order of precedence of how options take effect
is as follows: All options have their compiled initial values which are totally ignored and most of the time, over-written by the JSON file values at runtime.  The JSON syntax checking and parsing directly updates this **struct**.  Finally, run-time command line options overwrite the appropriate **struct** members' values when the command line parsing takes place.    
     
This means that the command line options are the most important - they have the final say in what *Video::vcGlobals* contains. The JSON values are secondary to that, followed by the compiled-in values that exist for static variables, etc, which matter the least.   
     
**Motivation**:  I really really don't like any of the command line parsing (and configuration) software that comes with *C++/linux*, or is available out there in the wild.  It's a personal choice, and as you can tell, my alternatives for these
projects are being developed as I go along.  My apologies if this offends the sensibilities of anyone who wishes to 
use this code for their own purpose.     
    
**Command Line Options**:    
      
 In the descriptions listed below, I refer to the equivalen Json **Node** which contains the value used/modified 
 by the item in question like this, for example:  **Root["Config"]["Logger"]["file-name"].asString()** - which is
 *JasonCpp* syntaxt for C++ access to this node (after the JsonCpp object has been parsed).  It is the C++ *operator[]()* 
 method which uniquely defines the string (in this case) in the Json file.  (Please have the file *video_capture.json* open 
 for reference as we go through this. You will see plenty of this syntax in the code - see *video_capture_globals.cpp* or *video_capture_thread.cpp* for examples).     

    
       [ -fn [ file-name ] ]     Turns on the "write-to-file" functionality (see JSON file).  The file-name
                                 parameter is the file which will be created to hold image frames. If it exists,
                                 the file will be truncated. If the file name is omitted, the default name
                                 "video_capture.data" will be used. (By default, the "write-to-file" capability
                                 is turned off in favor of the "write-to-process" member in the JSON config file).   
     
     Equivalent Json member(s):  Root["Config"]["App-options"]["write-to-file"] (treated here like an int)     
                                 Root["Config"]["App-options"]["output-file"] (string)      
      
If the -fn flag is specified (on the command line), then the write-to-file capability is turned on (enabled). It is off by default.  If the file-name is not mentioned, the default value (from the Json file) is used.     
     
(A note about *bool*:  Although
it is well defined enough in C++, values other than 0 or 1 can be used.  The way the JsonCpp *bool* is used in this project
is like an int (in the Json file), but when it is to be assigned to a *C++ bool type*, the univesally accepted conversion takes place:  If the value is 0, then the bool is set to *false*.  Anything else means *true*).   
      
       [ -pr [ timeslice_ms ]]   Enable profiler stats. If specified, the optional parameter is the number
                                 of milliseconds between profiler snapshots. The default is 800 milliseconds.  
                                   
       [ -lg log-level ]         Can be one of: {"DBUG", "INFO", "NOTE", "WARN", "EROR", "CRIT"}.
                                 (The default is NOTE)  
                                 
       [ -loginit ]              (no parameters) This flag enables the logging of initialization info.   
          
       [ -fg [ video-grabber ]]  The video frame grabber to be used. Can be one of {"v4l2", "opencv"}.
                                 (The default grabber is "v4l2").  
                                   
       [ -fc frame-count ]       Number of frames to grab from the hardware (default is 200)   
         
       [ -dv video-device ]      The /dev entry for the video camera. (The default value is /dev/video0)   
       
       [ -proc-redir [ file ]]   If the "write-to-process" member of the JSON config file is set to 1 (enabled),
                                 the process which is started and streamed to (typically ffmpeg) has its "standard
                                 error" still open to the controlling display (terminal).  To get rid of the extra
                                 output on the screen, std::cerr, (stderr, fd 2, etc) can be redirected to a regular
                                 file or to "/dev/null" as needed by using this flag and a filename. If the "file"
                                 parameter is not specified, the standard error output will go to the screen.
                                 (By default, the flag is enabled, and the filename used is "/dev/null").  
                                   
       [ -pf pixel-format ]      The pixel format requested from the video driver, which can be "h264" or "yuyv".
                                 These are:
                                           V4L2_PIX_FMT_H264: H264 with start codes
                                           V4L2_PIX_FMT_YYUV: 16bit YUV 4:2:2
                                 Please see /usr/include/linux/videodev2.h for more information
                                 (The default pixel-format value is "h264").




   __________________   
   
    
    
# Please Note:
This is work in progress -- I'm writing code and uploading the sources to the repository while ensuring that everything is tested, building properly (at least on my system), and working. So for a period of time I've got restrictions on interactions with the repositories that allow one to view, clone and/or otherwise download the code (to which you are welcome as per the LICENSE) but I am not yet welcoming collaborators. Right now, even though I no longer have just some basic libraries and main programs that use/exercise them, there are still more objects coming.  As soon as I introduce enough code that is mostly stable, I'll remove the restrictions.  In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.     
     
Thank you.     
     
**SEE ALSO:**    

The README.md file in the root **Samples** folder (./README.md).    
The README.md file in the **source** folder (./source/README.md).    
The file **./LICENSE** contains the legal language covering distribution and use of the sources in this project (that belong to me). 
The software that does not belong to me, is covered by its own license which is clearly marked in the code and in the README files.     
     
     
### Current deficiencies:

Currently the Windows' WIN32 configuration has not been built and tested yet. 
This may be added in the future.








