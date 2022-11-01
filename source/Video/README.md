
# Video Capture 

Stream video frames from the source (a camera), through the linux driver, to the process **main_video_capture**, which uses objects from this project (**Video**), as well as objects from the other projects in this repository.

### Table of Contents

[INTRODUCTION](#introduction)  
  * [Starting Slowly](#starting-slowly)   
  * [Usage and the --help flag](#usage-and-the---help-flag)   
  * [Option Coverage: JSON file vs. command line](#option-coverage-json-file-vs-command-line)    

[Command Line Flags and Options](#command-line-flags-and-options)    
  * [The -fn flag:  write to file](#the--fn-flag)     
  * [The -pr flag: profiling](#the--pr-flag)     
  * [The -lg flag: log level](#the--lg-flag)    
  * [The -loginit flag: log initial verbose lines](#the--loginit-flag)     
  * [The -fg flag: select frame grabber](#the--fg-flag)     
  * [The -fc flag: frame count](#the--fc-flag)    
  * [The -dv flag: select device](#the--dv-flag)    
  * [The -proc-redir flag: redirect stderr](#the--proc-redir-flag)    
  * [The -use-other-proc flag: use different process for popen()](#the--use-other-proc-flag)     
  * [The -pf flag: pixel format](#the--pf-flag)     
  * [The -test-suspend-resume flag: suspend/resume a few times for less than a minute](#the--test-suspend-resume-flag) 
  
[EXAMPLES OF REAL USE](#examples-of-real-use) 
  * [Example 1: A simple run](#example-1-a-simple-run)


#### Introduction

There are many many options and configurations that control how to get video from a camera to an end destination of a video pipeline.  As an example, run the help feature of the **ffmpeg** utility like this: **ffmpeg --help full**.  The scope is huge. In this project, the surface is barely skimmed, but it does a lot to demonstrate how to move data down the video pipline (currently only under linux), to be saved in a file, or piped to a cooperating application or utility (**ffmpeg** is used extensively), where it can be streamed to another process on the current system, or any other system on a connected network.  For exampele, in this project, the **main_video_player** uses **ffmpeg** internally to produce an mp4 file that any video player can pick up and display it on a screen.   

[(Back to the top)](#video-capture)

#### Starting Slowly

To get started, let's take a look on how to run the utility with a bit of setup and a couple of exmples.  Let's assume we have a camera connected to a USB port on our system, and which we know to be working.  For example, run the **vlc** utility, and under the **Media --> Open Capture Device** menu, set the capture device to "/dev/video0" (on linux).  If the device drop-down menu does not show any entries, then the setup for the camera is not yet complete, or a few other possibilities that we're not going to cover here (disconnect/reconnect the usb line to the system, etc).  The actual device name can be something else, depending on the camera configuration.   

[(Back to the top)](#video-capture)

#### Usage and the "--help" flag 
     
The output shown below is from the utility run like this: **main_video_capture --help**.  Please take a look at the options to familiarize yourself with them, and we can then show a couple of examples.  Interspersed with the options are small sections of text that provide additional information about each:     
     
     $ main_video_capture --help   
            
     Usage:
     
       main_video_capture --help (or -h or help)
     
     Or:
     
       main_video_capture  [ options ]
      
This is really an either/or situation. You either want "help" or you want to run the app with full functionality.
The following section lists all the options available from the command line - around nine or ten at this time. This list
tends to grow over time, and the README (this file) does not necessarily keep up in a timely manner. This also means that
the definitive description of the behavior of the various options lies in the source file (*video_capture_commandline.cpp*), 
and the JSON configuration file that is needed by the app in order to run (*video_capture.json*).    
     
**A note about video_capture_commandline.cpp** and the associated other source file that use it:  What goes on in there does seem a bit unwieldy. And it kind of is.  However... (and in my defense), I've been developing (improving) and moving the command line parsing and configuration from "inline" code towards usage of C++ templates and specialized template functions. You can see evidence of that in the objects that the parser uses to do its job. By the time this is done, most if not all the parsing and organization of the data at runtime, will mostly be done by a base class (see *commandline.cpp/.hpp* in the **Util** project) which will do all of the parsing work, and leave it to the caller to check values, legal limits, etc.   

[(Back to the top)](#video-capture)

#### Option Coverage: JSON file vs. command line
  
Most, if not all of the command line options are covered by the JSON configuration.  But not vice-versa. The JSON file has some complexity that is almost impossible to cover by the command line options in some sane fashion. Here's how this is managed:     
     
In the source files *video_capture_globals.cpp/.hpp* you will find the **struct vcGlobals** object in the **Video** namespace (aka *Video::vcGlobals* in the code).     
     
This *struct* encapsulates **ALL** the runtime options that the app uses. The order of precedence of how options take effect
is as follows: All options have their compiled initial values which are totally ignored and most of the time, over-written by the JSON file values at runtime.  The JSON syntax checking and parsing directly updates this **struct**.  Finally, run-time command line options overwrite the appropriate **struct** members' values when the command line parsing takes place.    
     
This means that the command line options are the most important - they have the final say in what *Video::vcGlobals* contains. The JSON values are secondary to that, followed by the compiled-in values that exist for static variables, etc, which matter the least.   
     
**Motivation**:  I really really don't like any of the command line parsing (and configuration) software that comes with *C++/linux*, or is available out there in the wild.  It's a personal choice, and as you can tell, my alternatives for these
projects are being developed as I go along.  My apologies if this offends the sensibilities of anyone who wishes to 
use this code for their own purpose.     

[(Back to the top)](#video-capture)

### Command Line Flags and Options    
      
 In the descriptions listed below, I refer to the equivalent Json **Node** which contains the value used/modified 
 by the item in question like this:  **Root["Config"]["Logger"]["file-name"]** - which is
 *JasonCpp* syntax for C++ access to this node (after the JsonCpp object has been parsed).  It is the C++ *operator[]()* 
 method which uniquely defines the **Json Node** in the Json file. 
 (Please have the file *video_capture.json* open 
 for reference as we go through this. You will see plenty of this syntax in the code - see *video_capture_globals.cpp* or *video_capture_thread.cpp* for examples).     

[(Back to the top)](#video-capture)

#### The -fn flag    
       [ -fn [ file-name ] ]     Turns on the "write-to-file" functionality (see JSON file).  The file-name 
                                 parameter is the file which will be created to hold image frames. If it exists, 
                                 the file will be truncated. If the file name is omitted, the default name used 
                                 is the runtime value of "output-file" in the Json config file. (By default, the 
                                 "write-to-file" capability is turned off in favour of the "write-to-process" 
                                 member in the JSON config file).   
       
     Equivalent Json member(s):  Root["Config"]["App-options"]["write-to-file"] (bool, treated here like an int)     
                                 Root["Config"]["App-options"]["output-file"] (string)      
     
     Equivalent C++ Video::vcGlobals member(s): 
                                 static bool write_frames_to_file;
                                 static std::string output_file;

If the **-fn** flag is specified (on the command line), then the write-to-file capability is turned on (enabled). It is off by default.  If the **file-name** is not mentioned, the value (from the Json file) is used.  This capability writes out to
the file the actual raw frames that come from  the camera in the format specified by the json file and/or the command line options
(see more on that below).    
     
The write-to-file capability operates completely independently from the write-to-process capability outlined below.  They can both be enabled and produce an **h264** data file (or a **yuyv** data file), as well as an **mp4** file in the same run.  Usually one of the two suffices.      
     
(**A note about bool**:  Although
it is well defined enough in C++, values other than 0 or 1 can be used.  The way the JsonCpp *bool* is used in this project
is like an int (in the Json file), but when it is to be assigned to a *C++ bool type*, the univesally accepted conversion takes place:  If the value is 0, then the bool is set to *false*.  Anything else means *true*).

[(Back to the top)](#video-capture)

#### The -pr flag    
       [ -pr [ timeslice_ms ]]   Enable profiler stats. If specified, the optional parameter is the number of 
                                 milliseconds between profiler snapshots. (The default is the runtime value of 
                                 "profile-timeslice-ms" in the Json config file).    
     
     Equivalent Json member(s):  Root["Config"]["App-options"]["profiling"] (bool, treated here like an int)     
                                 Root["Config"]["App-options"]["profile-timeslice-ms"] (int)      
     
     Equivalent C++ Video::vcGlobals member(s): 
                                 static bool profiling_enabled;
                                 static int  profile_timeslice_ms;

Profiling numbers (stats) are run in their own thread every *timeslice_ms* milliseconds once the video frames are being 
captured by the video driver interface (which also runs in its own thread).  The counting of frames is why profiling intrudes
into the video interface to take a snapshot of some of the needed profiling data, because it's only there that it is easily known when one frame is finished, and the next is about to start.     
      
Although the linux driver dictates the fixed size of every memory mapped buffer used to hold raw video frames, the relationship is usually many-to-one: it takes more than one buffer to complete a single frame.  By the time it takes for a buffer to be copied and fed to the ring buffer of std::shared_ptr<>'s for further processing, it is no longer immediately apparent where one frame ends, and where the next one begins.  
     
Profiling is **disabled** by default.  If the **-pr** flag is used on the command line, profiling is **enabled**. How many milliseconds pass between profiling runs depends on the **timeslice_ms** command line parameter if it is specified, or the value specified in the Json file if it is not specified on the command line.    

[(Back to the top)](#video-capture)

#### The -lg flag    
       [ -lg log-level ]         Can be one of: {"DBUG", "INFO", "NOTE", "WARN", "EROR", "CRIT"}.
                                 (The default is the runtime value of "log-level" in the Json config file)    
     
     Equivalent Json member(s):  Root["Config"]["Logger"]["log-level"] (string) 
                                 Root["Config"]["Logger"]["file-name"]    
          
     Equivalent C++ Video::vcGlobals member(s): 
                                 static Log::Log::Level loglevel;
                                 static std::string log_level;
                                 static std::string logFilelName;

The log level determines the level of verbosity of information written to the log file (*Root["Config"]["Logger"]["file-name"]* in the Json config file).   Internally, the log level strings are translated to any of the enum values defined in the LoggerCpp source file *...Samples/source/3rdparty/include/LoggerCpp/Log.h* as *enum Level { eDebug = 0, eInfo, eNotice, eWarning, eError, eCritic }*.  Please note that the log file name itself does
not change after the program starts running. It is fixed as the content of the Json file member mentioned above.    

[(Back to the top)](#video-capture)

#### The -loginit flag    
       [ -loginit ]              (no parameters) This flag enables the logging of initialization info.   
        
     Equivalent Json member(s):  None
          
     Equivalent C++ Video::vcGlobals member(s): 
                                 static bool log_initialization_info;

The parsing of the Json config file, as well as the parsing of the command line produce a 
volumenous amount of information into the log file.  Typically, once command line issues and
json syntax issues are resolved, one does not need to see this information in the log file every time
one examines it.  This option causes all the log file information which is accumulated before
the logger is ready to write data into the log file, to be written into the log file.  Otherwise,
by default, this initial (and volumenous) output is supressed, and does not show up in the log file.           

[(Back to the top)](#video-capture)

#### The -fg flag    
       [ -fg [ video-grabber ]]  The video frame grabber to be used. Can be one of {"v4l2", "opencv"}. 
                                 (The default grabber is the runtime value of "preferred-interface" 
                                 in the Json config file).  
     
     Equivalent Json member(s):  Root["Config"]["Video"]["preferred-interface"] (string)     
          
     Equivalent C++ Video::vcGlobals member(s): 
                                 static std::string video_grabber_name;

The video frame grabber to be used in a run, depends on what is available both from the video source (camera), 
as well as in the operating system driver, as well as in the interface to the (OS) driver.  There has to be at least one 
configuration that matches all three, or the interface will not work.  In this case, since the software
is being developed for a USB webcam which provides an interface compatible with V4L (*Video for Linux* - V4L2 in this case), 
the interface in this program was originally written for V4L2 frame grabbing.  Another interface in the process
of being implemented, is **opencv**, which is not merely a frame grabber, but a complex and feature-heavy interface
to the camera source, which also provides raw video frames as an exposed interface which we will use. Although you might
see evidence of **opencv** in the code, it is not quite ready for use.  But the option exists in this interface to the operating system, even though it does not work yet, at the moment.    
     
The **-fg** flag currently has two acceptable values: "v4l2" or "opencv".  The default is the only one that works at the moment - "v4l2".  If you exmine the *video_capture.json* file, you will see some nested sections under *"Video"* for which the frame grabber name (as well as the *pixel-format* - see below) act as keys, so that specifying the correct frame grabber on the command line (or by default) also resolves additional grabber-dependent (as well as pixel format- dependent) capabilities that have reasonable values which will be automatically configured once the grabber and the pixel formats are chosen.     

[(Back to the top)](#video-capture)

#### The -fc flag    
       [ -fc frame-count ]       Number of frames to grab from the hardware. (The default is the runtime value of 
                                 "frame-count" in the Json config file).  
     
     Equivalent Json member(s):  Root["Config"]["App-options"]["frame-count"] (int)     
          
     Equivalent C++ Video::vcGlobals member(s): 
                                 static size_t framecount;
                                 static std::string str_frame_count;

This number (the frame count) will determine for how long the frame grabber will stream video from the source 
(camera).  The program will terminate normally after *frame-count* frames have been captured and processed. 
The default number (if *-fc* is not used) is set by the *"frame-count"* parameter specified in the Json configuration file.     
     
A special case of this option is when *"-fc 0"* is used on the command line (same as **frame-count** in the JSON file).  This tells the program to stream frames indefinitely.  It could be stopped by using the keyboard to interrupt the
program, or by disconnecting the camera from the USB port, which will cause an error.    
      
**CAUTION**:  Running the program with a large frame-count value or without limitation **WILL** take a huge amount of
space in the file system. A multitude of gigabytes can be consumed in a short time.    

[(Back to the top)](#video-capture)

#### The -dv flag    
       [ -dv video-device ]      The /dev entry for the video camera. (The default value is the runtime value of
                                 "device-name" in the Json config file in the section named for the video-grabber used.
     
     Equivalent Json member(s):  Root["Config"]["Video"]["frame-capture"]
                                                            [Video::vcGlobals::video_grabber_name]
                                                                            ["device-name"] (string)     
          
     Equivalent C++ Video::vcGlobals member(s): 
                                 static std::string str_dev_name;

The equivalent Json member refered to here uses the *Video::vcGlobals::video_grabber_name* static variable to define
the correct "device-name" Json member.  In actual fact, the static variable *Video::vcGlobals::str_dev_name* already has the correct string in it which is determined automatically during the Json parsing of the configuration
file).  This can be examined easily in the log file after the run.  

[(Back to the top)](#video-capture)

#### The -proc-redir flag    
       [ -proc-redir [ file ]]   If the "write-to-process" member of the JSON config file is set to 1 (enabled),
                                 the process which is started and streamed to (typically ffmpeg) has its "standard
                                 error" still open to the controlling display (terminal).  To get rid of the extra
                                 output on the screen, std::cerr, (stderr, fd 2, etc) can be redirected to a regular
                                 file or to "/dev/null" as needed by using this flag and a filename. If the "file"
                                 parameter is not specified, the standard error output will go to the screen.
                                 (By default, the flag is enabled, and the filename used is "/dev/null"). 
     
     Equivalent Json member(s):  None. But this capability affects what happens to:
      
                                 Root["Config"]["Video"]["frame-capture"]
                                       [Video::vcGlobals::video_grabber_name]
                                          ["pixel-format"]
                                            [string derived from enum pxl_formats Video::vcGlobals::pixel_format]
                                              ["output-process"] (string);
          
     Equivalent C++ Video::vcGlobals member(s): 
                                 static bool proc_redir;
                                 static std::string redir_filename;

The description for this flag almost says it all, but in a rather terse fashion.  The simplest way to explain this would be an example:    
                                         
If at runtime, the preferred interface is "v4l2", and the preferred pixel format for that interface is "yuyv" (see next flag below), the definitively precise string representing the process to be started with the popen() system call from within the **main_video_capture** program would be found at:     

        Root["Config"]["Video"]["frame-capture"]["v4l2"]["pixel-format"]["yuyv"]["output-process"]     

The linux command residing in that section currently is:     
     
        ffmpeg -nostdin -y -f rawvideo -vcodec rawvideo -s 640x480 -r 25 -pix_fmt yuyv422 \  
               -i  pipe:0 -c:v libx264 -preset ultrafast -qp 0 video_capture.mp4"
        
* If the *-proc-redir* flag is not used at all, the actual process started would be:    

        ffmpeg -nostdin -y -f rawvideo -vcodec rawvideo -s 640x480 -r 25 -pix_fmt yuyv422 \    
               -i  pipe:0 -c:v libx264 -preset ultrafast -qp 0 video_capture.mp4 2> /dev/null    

Which means that the ffmpeg stderr output would be redirected to the bit bucket (/dev/null). (This is the default).       
     
* If, however the flag is specified like this: "*-proc-dir ffmpeg_sderr.txt*", the actual process started would be: 

        ffmpeg -nostdin -y -f rawvideo -vcodec rawvideo -s 640x480 -r 25 -pix_fmt yuyv422 \     
               -i  pipe:0 -c:v libx264 -preset ultrafast -qp 0 video_capture.mp4 2> ffmpeg_sderr.txt      

Which means that the ffmpeg stderr output would be found in ffmpeg_sderr.txt after the run.   
     
* If, lastly, the flag is specified like this: "*-proc-dir*" with no parameter, the actual process started would be: 

        ffmpeg -nostdin -y -f rawvideo -vcodec rawvideo -s 640x480 -r 25 -pix_fmt yuyv422 \     
               -i  pipe:0 -c:v libx264 -preset ultrafast -qp 0 video_capture.mp4    

Which means that the ffmpeg stderr output would appear on the screen at runtime, obscuring the regular output 
to the screen of **main_video_capture**.   This can appear messy at the beginning, since you're not seeing the 
regular output you are used to seeing from the app, but instead you're seing **a lot** of detail of what's going
on with ffmpeg while it is converting the yuyv raw frames to an mp4 file.    
     
Please Note:  In this last case, you will not see the shell prompt showing that the app finished running, because ffmpeg most likely wrote over it.  You might think that the app got a "hang" - but it did not.  Just press the ENTER key and you will get the prompt.     

[(Back to the top)](#video-capture)

#### The -use-other-proc flag    
       [ -use-other-proc ]       (no parameters) This flag directs the program to use the "other" entry (within the 
                                 "pixel-format" section of "preferred-interface" of the "frame-capture" section of 
                                 the JSON configuration file) as the command string to use for popen() instead of the 
                                 default command string indicated by the name of the pixel format chosen.    
        
     Equivalent Json member(s):  None.     
          
     Equivalent C++ Video::vcGlobals member(s):  
                                 static bool use_other_proc;  

This flag directs **main_video_capture** to use the *"other"* entry specified in the *"pixel-format*" section instead of the 
default.  This is helpful for testing and/or debugging, particularly if the **-fn** flag is used as well. Assuming 
that the *"dd"* command is left in the *"other"* node, the output produced by it (in the file specified by the *"of="* 
parameter of *"dd"*, **should** always be identical to output produced in the file specified by the **-fn** flag.     
     
As mentioned before, both flags (-use-other-proc and -fn) can be used at the same time. Of course other things can be done by setting *"other"* to some other command line.  YMMV.    

[(Back to the top)](#video-capture)

#### The -pf flag    
       [ -pf pixel-format ]      The pixel format requested from the video driver, which can be "h264" or "yuyv".
                                 These are:
                                           V4L2_PIX_FMT_H264: H264 with start codes
                                           V4L2_PIX_FMT_YYUV: 16bit YUV 4:2:2
                                 Please see /usr/include/linux/videodev2.h for more information
                                 (The default pixel-format value is the runtime value of "preferred-pixel-format"
                                 in the Json config file in the section named for the video-grabber used).   
     
     Equivalent Json member(s):  Root["Config"]["Video"]["frame-capture"]
                                                            [Video::vcGlobals::video_grabber_name]
                                                                            ["preferred-pixel-format"] (string)     
     Equivalent C++ Video::vcGlobals member(s):  
                                 static enum pxl_formats pixel_format;  

For each section of "frame-capture" specified for a particular interface ("v4l2", "opencv"), a specific pixel format has
to exist in order to associate the frame grabbing with a specific device ("device-name"), a pixel format 
("preferred-pixel-format", and its associated "output-process").  As mentioned above, the depends on the OS driver used
as well as the source hardware (camera).  This option is set automatically based on the interface picked.  

[(Back to the top)](#video-capture)

#### The -test-suspend-resume flag    
       [ -test-suspend-resume ]  (no parameters) The program will run a special thread that first sets the frame-count
                                 to 0 (regardless of command-line or JSON settings), and then it allows main() to run. It 
                                 then interrupts the flow of video frames every few seconds with a "pause" request, waits
                                 a few seconds and then "resume"s. This goes on for 30 or 40 seconds, and then it terminates
                                 the program. The effects on the program and data flow can be seen in the log file.     
       
     Equivalent Json member(s):  None     
          
     Equivalent C++ Video::vcGlobals member(s):  
                                 static bool test_suspend_resume;    

[(Back to the top)](#video-capture)
   
### EXAMPLES OF REAL USE   
Don't be alarmed...   **THIS SECTION IS UNDER CONSTRUCTION....**     
  
This section will include examples run on my desktop, with real output and log file unchanged. 
There are a few explanations and pointers interjected along the way.  The most definitive way to
find out what exactly goes on during the run is to first, examine the output on the screen.  Then,
look at file sizes (log file, data file, mp4 file, etc).  Lastly, go into the log file and examine
the detail offered.  It's a lot of detail, but pretty much shows what happened.

#### Example 1: A simple run   
In this example, we run a capture of 50 frames, with profiling enabled. This is the output on the screen:       
     
     $ main_video_capture -fc 50 -pr
         Frame count is set to 50(int) = 50(string)
     Command line parsing:
         Logging of initialization lines is set to false.
         redirect stderr to file is set to true
         Stderr output from the process streamed to, will be redirected to /dev/null
         write-frames-to-file is set to false, file name is video_capture.data
         Profiling is set to enabled, timeslice = 300.
         Video frame grabber device name is set to /dev/video0
         Video pixel format is set to: V4L2_PIX_FMT_H264: H264 with start codes
         use_other_proc is set to false
         test_suspend_resume is set to false
         Video frame grabber name is set to v4l2
         Log level is set to 0 = DBUG
         Frame count is set to 50(int) = 50(string)
     
     Log file: video_capture_log.txt
     driver: frame: 1920 x 1080
     driver: pixel format set to 0x34363248 - H264: H264 with start codes
     NORMAL TERMINATION...
     $    

Next, files and their sizes:    

     $ ls -ltrh 
         .  .  .  .  . . .
     -rwxr-xr-x 1 andrew andrew 630K Nov  1 01:53 main_video_capture
     -rw-r--r-- 1 andrew andrew 2.1M Nov  1 01:57 video_capture.mp4
     -rw-r--r-- 1 andrew andrew 8.0K Nov  1 01:57 video_capture_log.txt
     $   

[(Back to the top)](#video-capture)

Now for the log file.  There are a few sections of text explaining what's going on below. 
In particular, look for the profiling entries.  Something to say about that below.        
   
     $ cat video_capture_log.txt
     
     2022-11-01 01:57:03.295  video_capture INFO START OF NEW VIDEO CAPTURE RUN
     2022-11-01 01:57:03.295  video_capture DBUG Current option values after getting shared_ptr<> to Log::Logger:
          Log Level 0
          Log level string DBUG
          Log channel name video_capture
          Log file name video_capture_log.txt
          Output to console disabled
          Output to log file enabled     

The above section is all about setting up the logger.  This has to be done sooner rather than later at runtime.     
     
     2022-11-01 01:57:03.295  video_capture INFO 
     
     Logger setup is complete.
     
     2022-11-01 01:57:03.295  video_capture INFO 
     2022-11-01 01:57:03.295  video_capture INFO The last few lines of deferred output from app initialization are shown here.   ******
     2022-11-01 01:57:03.295  video_capture INFO For the full set of deferred lines, use the -loginit flag on the command line.  ******
     2022-11-01 01:57:03.295  video_capture INFO DELAYED: .  .  .  . . . .
     
     From JSON:  Getting available pixel formats for interface "v4l2":
             {  h264  other  yuyv    }
     From JSON:  Set logger channel-name to: video_capture
     From JSON:  Set logger file-name to: video_capture_log.txt
     From JSON:  Set default logger log level to: DBUG
     From JSON:  Enable writing raw video frames to output file: false
     From JSON:  Set raw video output file name to: video_capture.data
     From JSON:  Enable writing raw video frames to process: true
     From JSON:  Enable profiling: false
     From JSON:  Set milliseconds between profile snapshots to: 300
     From JSON:  Set default video-frame-grabber to: v4l2
     From JSON:  Set number of frames to grab (framecount) to: 20
     From JSON:  v4l2 is labeled as: V4L2
     From JSON:  Set v4l2 device name to /dev/video0
     From JSON:  Set v4l2 pixel format to V4L2_PIX_FMT_H264: H264 with start codes
     From JSON:  Set raw video output process command to: ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec cop video_capture.mp4 
     
The above section is all parsing the JSON config file, and assigning values to the global C++ struct (vcGlobals). 
This gives you an idea of the default values included in the .json file this was run against.  But this was 
assembled before command line parsing was applied, so you can see, for example, that in the JSON file, profiling is
"false".  But that changes to "true" below, because of the "-pr" flag used on the command line.     

[(Back to the top)](#video-capture)

     2022-11-01 01:57:03.295  video_capture INFO DELAYED: .  .  .  . . . .
     
     Command line parsing:
         Logging of initialization lines is set to false.
         redirect stderr to file is set to true
         Stderr output from the process streamed to, will be redirected to /dev/null
         write-frames-to-file is set to false, file name is video_capture.data
         Profiling is set to enabled, timeslice = 300.
         Video frame grabber device name is set to /dev/video0
         Video pixel format is set to: V4L2_PIX_FMT_H264: H264 with start codes
         use_other_proc is set to false
         test_suspend_resume is set to false
         Video frame grabber name is set to v4l2
         Log level is set to 0 = DBUG
         Frame count is set to 50(int) = 50(string)
     
     2022-11-01 01:57:03.296  video_capture DBUG main_video_capture:  started video profiler thread
     2022-11-01 01:57:03.296  video_capture DBUG main_video_capture:  the video capture thread will kick-start the video_profiler operations.
     2022-11-01 01:57:03.296  video_capture DBUG Profiler thread started...
     2022-11-01 01:57:03.296  video_capture DBUG main_video_capture: kick-starting the queue operations.
     2022-11-01 01:57:03.296  video_capture NOTE Profiler thread: skipping first frame to establish a duration baseline.
     2022-11-01 01:57:03.296  video_capture DBUG main_video_capture:  starting the video capture thread.
     2022-11-01 01:57:03.296  video_capture DBUG 
     raw_buffer_queue_handler: Updated output process to:  ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4 2> /dev/null
     2022-11-01 01:57:03.296  video_capture DBUG main_video_capture:  kick-starting the video capture operations.
     2022-11-01 01:57:03.296  video_capture INFO Video Capture thread: Requesting the v4l2 frame-grabber.
     2022-11-01 01:57:03.297  video_capture INFO Video Capture thread: The list of available frame grabbers in the json config file is: opencv v4l2 
     2022-11-01 01:57:03.297  video_capture INFO Video Capture thread: Picking the v4l2 frame-grabber.
     2022-11-01 01:57:03.297  video_capture INFO Video Capture thread: Interface used is v4l2
     2022-11-01 01:57:03.297  video_capture DBUG Started the process "ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4 2> /dev/null".
     2022-11-01 01:57:03.297  video_capture DBUG In VideoCapture::raw_buffer_queue_handler(): Successfully started "ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4".
     2022-11-01 01:57:03.368  video_capture INFO Device /dev/video0
     2022-11-01 01:57:03.368  video_capture DBUG Set video format to (1920 x 1080), pixel format is V4L2_PIX_FMT_H264: H264 with start codes
     2022-11-01 01:57:03.369  video_capture DBUG driver: frame: 1920 x 1080
     2022-11-01 01:57:03.369  video_capture DBUG driver: pixel format set to 0x34363248 - H264: H264 with start codes
     2022-11-01 01:57:03.369  video_capture DBUG driver: bytes required: 4147200
     2022-11-01 01:57:03.369  video_capture DBUG driver: I/O METHOD: IO_METHOD_MMAP
     2022-11-01 01:57:03.544  video_capture DBUG vidcap_v4l2_driver_interface::run() - kick-starting the video_profiler operations.
     2022-11-01 01:57:03.544  video_capture NOTE Profiler info...
     2022-11-01 01:57:03.544  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 01:57:03.544  video_capture NOTE Number of frames received: 0
     2022-11-01 01:57:03.544  video_capture NOTE Current avg frame rate (per second): 0
     2022-11-01 01:57:03.844  video_capture NOTE Profiler info...
     2022-11-01 01:57:03.844  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 01:57:03.844  video_capture NOTE Number of frames received: 11
     2022-11-01 01:57:03.845  video_capture NOTE Current avg frame rate (per second): 38.5965
     2022-11-01 01:57:04.145  video_capture NOTE Profiler info...
     2022-11-01 01:57:04.145  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 01:57:04.145  video_capture NOTE Number of frames received: 18
     2022-11-01 01:57:04.145  video_capture NOTE Current avg frame rate (per second): 31.8584
     2022-11-01 01:57:04.445  video_capture NOTE Profiler info...
     2022-11-01 01:57:04.445  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 01:57:04.445  video_capture NOTE Number of frames received: 26
     2022-11-01 01:57:04.445  video_capture NOTE Current avg frame rate (per second): 29.3785
     2022-11-01 01:57:04.746  video_capture NOTE Profiler info...
     2022-11-01 01:57:04.746  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 01:57:04.746  video_capture NOTE Number of frames received: 33
     2022-11-01 01:57:04.746  video_capture NOTE Current avg frame rate (per second): 28.3262
     2022-11-01 01:57:05.046  video_capture NOTE Profiler info...
     2022-11-01 01:57:05.046  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 01:57:05.046  video_capture NOTE Number of frames received: 41
     2022-11-01 01:57:05.046  video_capture NOTE Current avg frame rate (per second): 27.6094
     2022-11-01 01:57:05.347  video_capture NOTE Profiler info...
     2022-11-01 01:57:05.347  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 01:57:05.347  video_capture NOTE Number of frames received: 48
     2022-11-01 01:57:05.347  video_capture NOTE Current avg frame rate (per second): 27.1955
     2022-11-01 01:57:05.390  video_capture INFO v4l2if_mainloop: CAPTURE TERMINATION REQUESTED.
     2022-11-01 01:57:05.398  video_capture DBUG MAIN: Video Capture thread is done. Cleanup and terminate.
     2022-11-01 01:57:05.398  video_capture DBUG main_video_capture:  terminating queue thread.
     2022-11-01 01:57:05.398  video_capture DBUG Queue thread terminating ...
     2022-11-01 01:57:05.398  video_capture DBUG Shutting down the process "ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4" (fflush, pclose(): 
     2022-11-01 01:57:05.399  video_capture DBUG vidcap_v4l2_driver_interface::run() - terminating the video_profiler thread.
     2022-11-01 01:57:05.399  video_capture INFO vidcap_v4l2_driver_interface: NORMAL TERMINATION REQUESTED
     2022-11-01 01:57:05.647  video_capture DBUG Profiler thread terminating ...
     2022-11-01 01:57:06.297  video_capture INFO Terminating the logger.
     
     























  
   ________   
    
    
# Please Note:
This is work in progress -- I'm writing code and uploading the sources to the repository while ensuring that everything is tested, building properly (at least on my system), and working. So for a period of time I've got restrictions on interactions with the repositories that allow one to view, clone and/or otherwise download the code (to which you are welcome as per the LICENSE) but I am not yet welcoming collaborators.     
     
Right now, even though I no longer have just some basic libraries and main programs that use/exercise them, there are still more objects coming.  As soon as I introduce enough code that is mostly stable, I'll remove the restrictions.  In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.     
     
Thank you.     
     
**SEE ALSO:**    

The [README.md file](../../README.md) in the root **Samples** folder.    
The [README.md file](../README.md) in the **source** folder.    
The file [LICENSE](../../LICENSE) in the root **Samples** folder, contains the legal language covering distribution and use of the sources in this project (that belong to me).    
The software that does not belong to me, is covered by its own license which is clearly marked in the code and in the README files.     
     
     
### Current deficiencies:

Currently the Windows' WIN32 configuration has not been built and tested yet. 
This may be added in the future.









