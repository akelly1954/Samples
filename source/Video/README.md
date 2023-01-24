
# Video Capture 

Stream video frames from the source (a camera), through the linux driver, to the process **main_video_capture**, which uses objects from this project (**Video**), as well as objects from the other projects in this repository.

### Table of Contents

[INTRODUCTION](#introduction)  
  * [Starting Slowly](#starting-slowly)   
  * [Usage and the --help flag](#usage-and-the---help-flag)   
  * [Option Coverage: JSON file vs. command line](#option-coverage-json-file-vs-command-line)    

[Command Line Flags and Options](#command-line-flags-and-options)    
  * [The -dr flag:  write runtime configuration to a file](#the--dr-flag)     
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
  * [Example 2: A longer run With YUYV Pixel Format](#example-2-a-longer-run-with-yuyv-pixel-format)
  * [Example 3: When Things Go Terribly Wrong](#example-3-when-things-go-terribly-wrong)
  * [Example 4: Is This Working? - Write frames to file](#example-4-is-this-working)    
   
[SEE ALSO](#see-also)   
[Attached files](#attached-files)

#### Introduction

There are many many options and configurations that control how to get video from a camera to an end destination of a video pipeline.  As an example, run the help feature of the **ffmpeg** utility like this: **ffmpeg --help full**.  The scope is huge. In this project, the surface is barely skimmed, but it does a lot to demonstrate how to move data down the video pipline (currently only under linux), to be saved in a file, or piped to a cooperating application or utility (**ffmpeg** is used extensively), where it can be streamed to another process on the current system, or any other system on a connected network.  For exampele, in this project, the **main_video_player** uses **ffmpeg** internally to produce an mp4 file that any video player can pick up and display  on a screen.   

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


#### The -dr flag
       [ -dr [ file-name ] ]     Write out detailed runtime configuration to the parameter "file-name" instead of 
                                 the log file. The default file-name is "video_capture_runtime_config.txt". 
                                 The information provided is a snapshot of the runtime configuration after all json options 
                                 as well as command-line options have been set, and the plugin has been loaded.     
    
The **-dr** flag is one of the more relevant command line flags for debugging of loading the video capture plugin, 
as well as for how video frames wind up being collected in a file, or piped to a process (and which process/flags) once 
video capture has begun. 
It causes the main() program to collect the **current** runtime configuration of the running program after all initialization 
items have been completed without errors.  It shows the current (runtime) value of every relevant item, where to find it in the 
code - which **Video::vcGlobals** member(s) affect it, which section in the JSON config file is being used, as well as which 
JSON file fields are relevant, and lastly, which command line flags can affect it. (BTW, the --loginit command line flag is 
not needed for producing this file and its contents).     
    
Please see the attached file [video_capture_runtime_config.txt](https://github.com/akelly1954/Samples/files/10429232/video_capture_runtime_config.txt) showing a sample output file produced by using the **-dr** flag.    

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
      
The above section shows the final value of all the configuration parameters that could be modified
by command line flags.   The next section actually starts the main threads of the app and gets the 
real work going.   

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
     
[(Back to the top)](#video-capture)

The above section shows all of the participating threads logging their status and providing
information as they start running.  The output below, shows mostly profiler output, as it takes
a snapshot of some stats every few frames.      
     
**Please Note:**  the first few frame-rate reports
show an unduly fast frame-rate.  The true number should be close to 25 frames per second (fps). 
In reality, the driver does provide video buffers at a decaying rate, however in this case, the
baseline for snapshot timing is not well established. So it it more informative (and accurate) to
ignore the first couple of hundred frames before taking this number seriously.     
     
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
     
Also, please note that there are constantly 0 buffers waiting in the ring-buffer.  This is because the video
driver interface adds the video buffers' shared_ptr<>'s to the ring-buffer, and the queueing thread plucks them 
out as soon as they come in, and before the profiler has had a chance to see them in the queue.   This is because 
the driver takes much more time to provide video buffers than it takes the processing of buffers (piped to ffmpeg and/or written out to file).  That is 
the way all this is meant to work (it's a good thing).  There are places in the code that interject sleeps (look 
for commented out thread::sleep_for()'s which slow things down to the point where you could find dozens of members 
in the queue ready to be processed (those sleeps are used for testing when needed).    
     
[(Back to the top)](#video-capture)

#### Example 2: A longer run With YUYV Pixel Format 

In this example, we switch from default H264 format to YUYV, with profiling enabled. Also, the frame count will be high. See the text explaining things down below.  This is the output on the screen:    

     $ main_video_capture -fc 3000 -pr 1000 -pf yuyv
     Frame count is set to 3000(int) = 3000(string)
     Command line parsing:
         Logging of initialization lines is set to false.
         redirect stderr to file is set to true
         Stderr output from the process streamed to, will be redirected to /dev/null
         write-frames-to-file is set to false, file name is video_capture.data
         Profiling is set to enabled, timeslice = 1000.
         Video frame grabber device name is set to /dev/video0
         Video pixel format is set to: V4L2_PIX_FMT_YYUV: 16bit YUV 4:2:2
         use_other_proc is set to false
         test_suspend_resume is set to false
         Video frame grabber name is set to v4l2
         Log level is set to 0 = DBUG
         Frame count is set to 3000(int) = 3000(string)
     
     Log file: video_capture_log.txt
     driver: frame: 640 x 480
     driver: pixel format set to 0x56595559 - YUYV: aka "YUV 4:2:2": Packed format with ½ horizontal chroma resolution
     NORMAL TERMINATION...
     
Notice the increased number of frames (from 50 in the previous run, to 3000 in this run), and the profiling
slice (milliseconds between snapshots) has increased from 300ms to 1000ms (= 1 second).  And here are the 
file sizes:    

     -rwxr-xr-x 1 andrew andrew 631K Nov  1 08:54 main_video_capture
     -rw-r--r-- 1 andrew andrew  42K Nov  1 12:51 video_capture_log.txt
     -rw-r--r-- 1 andrew andrew 401M Nov  1 12:51 video_capture.mp4

Notice the mp4 file is now a bit bigger.  At 401Mb, the file contains 3000 frames, and will run for 
approximately 2 minutes.  All things being equal, if the video were to run for an hour, the file size then 
would be approximately 120Gb.  Just sayin'.    
     
[(Back to the top)](#video-capture)

The next section shows an abbreviated log file - just trying to save some virtual trees. The section showing the 
profiling snapshot numbers is from the very end of the run:
     
     $ 
     $ cat video_capture_log.txt  
     2022-11-01 12:49:13.593  video_capture INFO START OF NEW VIDEO CAPTURE RUN
     2022-11-01 12:49:13.594  video_capture DBUG Current option values after getting shared_ptr<> to Log::Logger:
          Log Level 0
          Log level string DBUG
          Log channel name video_capture
          Log file name video_capture_log.txt
          Output to console disabled
          Output to log file enabled
     
     2022-11-01 12:49:13.594  video_capture INFO 
     
     Logger setup is complete.
     
     2022-11-01 12:49:13.594  video_capture INFO 
     2022-11-01 12:49:13.594  video_capture INFO The last few lines of deferred output from app initialization are shown here.   ******
     2022-11-01 12:49:13.594  video_capture INFO For the full set of deferred lines, use the -loginit flag on the command line.  ******
     2022-11-01 12:49:13.594  video_capture INFO DELAYED: .  .  .  . . . .
     
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
     From JSON:  Set raw video output process command to: ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4
     
[(Back to the top)](#video-capture)

     2022-11-01 12:49:13.594  video_capture INFO DELAYED: .  .  .  . . . .
     
     Command line parsing:
         Logging of initialization lines is set to false.
         redirect stderr to file is set to true
         Stderr output from the process streamed to, will be redirected to /dev/null
         write-frames-to-file is set to false, file name is video_capture.data
         Profiling is set to enabled, timeslice = 1000.
         Video frame grabber device name is set to /dev/video0
         Video pixel format is set to: V4L2_PIX_FMT_YYUV: 16bit YUV 4:2:2
         use_other_proc is set to false
         test_suspend_resume is set to false
         Video frame grabber name is set to v4l2
         Log level is set to 0 = DBUG
         Frame count is set to 3000(int) = 3000(string)
     
     2022-11-01 12:49:13.594  video_capture DBUG main_video_capture:  started video profiler thread
     2022-11-01 12:49:13.594  video_capture DBUG main_video_capture:  the video capture thread will kick-start the video_profiler operations.
     2022-11-01 12:49:13.594  video_capture DBUG Profiler thread started...
     2022-11-01 12:49:13.594  video_capture DBUG main_video_capture: kick-starting the queue operations.
     2022-11-01 12:49:13.594  video_capture NOTE Profiler thread: skipping first frame to establish a duration baseline.
     2022-11-01 12:49:13.594  video_capture DBUG 
     raw_buffer_queue_handler: Updated output process to:  ffmpeg -nostdin -y -f rawvideo -vcodec rawvideo \
                               -s 640x480 -r 25 -pix_fmt yuyv422 -i  pipe:0 -c:v libx264 -preset ultrafast \
                               -qp 0 video_capture.mp4 2> /dev/null
     2022-11-01 12:49:13.594  video_capture DBUG main_video_capture:  starting the video capture thread.
     2022-11-01 12:49:13.594  video_capture DBUG main_video_capture:  kick-starting the video capture operations.
     2022-11-01 12:49:13.594  video_capture INFO Video Capture thread: Requesting the v4l2 frame-grabber.
     2022-11-01 12:49:13.594  video_capture INFO Video Capture thread: The list of available frame grabbers in the json config file is: opencv v4l2 
     2022-11-01 12:49:13.594  video_capture INFO Video Capture thread: Picking the v4l2 frame-grabber.
     2022-11-01 12:49:13.594  video_capture INFO Video Capture thread: Interface used is v4l2
     2022-11-01 12:49:13.595  video_capture DBUG Started the process "ffmpeg -nostdin -y -f rawvideo \
                              -vcodec rawvideo -s 640x480 -r 25 -pix_fmt yuyv422 -i  pipe:0 -c:v libx264 \
                              -preset ultrafast -qp 0 video_capture.mp4 2> /dev/null".
     2022-11-01 12:49:13.595  video_capture DBUG In VideoCapture::raw_buffer_queue_handler(): Successfully \
                              started "ffmpeg -nostdin -y -f rawvideo -vcodec rawvideo -s 640x480 -r 25 \
                              -pix_fmt yuyv422 -i  pipe:0 -c:v libx264 -preset ultrafast -qp 0 video_capture.mp4".
     
[(Back to the top)](#video-capture)

Notice that with this being the V4L2 interface, but with YUYV pixel format, the appropriate **ffmpeg** command 
was picked from the JSON config file.
     
     2022-11-01 12:49:13.664  video_capture INFO Device /dev/video0
     2022-11-01 12:49:13.664  video_capture DBUG Set video format to (640 x 480), pixel format is V4L2_PIX_FMT_YYUV: 16bit YUV 4:2:2
     2022-11-01 12:49:13.664  video_capture DBUG driver: frame: 640 x 480
     2022-11-01 12:49:13.664  video_capture DBUG driver: pixel format set to 0x56595559 - YUYV: aka "YUV 4:2:2": Packed format with ½ horizontal chroma resolution
     2022-11-01 12:49:13.664  video_capture DBUG driver: bytes required: 614400
     2022-11-01 12:49:13.664  video_capture DBUG driver: I/O METHOD: IO_METHOD_MMAP
     2022-11-01 12:49:14.140  video_capture DBUG vidcap_v4l2_driver_interface::run() - kick-starting the video_profiler operations.
     2022-11-01 12:49:14.140  video_capture NOTE Profiler info...
     2022-11-01 12:49:14.140  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:49:14.140  video_capture NOTE Number of frames received: 0
     2022-11-01 12:49:14.140  video_capture NOTE Current avg frame rate (per second): 0
     2022-11-01 12:49:15.141  video_capture NOTE Profiler info...
     2022-11-01 12:49:15.141  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:49:15.141  video_capture NOTE Number of frames received: 28
     2022-11-01 12:49:15.141  video_capture NOTE Current avg frame rate (per second): 28.2543
     2022-11-01 12:49:16.141  video_capture NOTE Profiler info...
     2022-11-01 12:49:16.141  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:49:16.141  video_capture NOTE Number of frames received: 53
     2022-11-01 12:49:16.141  video_capture NOTE Current avg frame rate (per second): 26.5797
     2022-11-01 12:49:17.141  video_capture NOTE Profiler info...
     2022-11-01 12:49:17.141  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:49:17.141  video_capture NOTE Number of frames received: 77
     2022-11-01 12:49:17.141  video_capture NOTE Current avg frame rate (per second): 25.9872
     2022-11-01 12:49:18.142  video_capture NOTE Profiler info...
     2022-11-01 12:49:18.142  video_capture NOTE Shared pointers in the ring buffer: 0
     
Notes for the section of the logfile shown above:  The frame rate, again, starts out being higher 
than it should be (28+ fps).   
     
[(Back to the top)](#video-capture)

            . . . . . SNNIPPED OUT A BUNCH OF SIMILAR LOG LINES . . . . . 
                                 (Praise the trees) 
      
     2022-11-01 12:51:07.176  video_capture NOTE Profiler info...
     2022-11-01 12:51:07.176  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:51:07.176  video_capture NOTE Number of frames received: 2821
     2022-11-01 12:51:07.176  video_capture NOTE Current avg frame rate (per second): 24.9648
     2022-11-01 12:51:08.176  video_capture NOTE Profiler info...
     2022-11-01 12:51:08.176  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:51:08.176  video_capture NOTE Number of frames received: 2846
     2022-11-01 12:51:08.176  video_capture NOTE Current avg frame rate (per second): 24.9634
     2022-11-01 12:51:09.177  video_capture NOTE Profiler info...
     2022-11-01 12:51:09.177  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:51:09.177  video_capture NOTE Number of frames received: 2871
     2022-11-01 12:51:09.177  video_capture NOTE Current avg frame rate (per second): 24.9648
     2022-11-01 12:51:10.177  video_capture NOTE Profiler info...
     2022-11-01 12:51:10.177  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:51:10.177  video_capture NOTE Number of frames received: 2897
     2022-11-01 12:51:10.177  video_capture NOTE Current avg frame rate (per second): 24.9666
     2022-11-01 12:51:11.177  video_capture NOTE Profiler info...
     2022-11-01 12:51:11.177  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:51:11.177  video_capture NOTE Number of frames received: 2921
     2022-11-01 12:51:11.177  video_capture NOTE Current avg frame rate (per second): 24.9656
     2022-11-01 12:51:12.177  video_capture NOTE Profiler info...
     2022-11-01 12:51:12.177  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:51:12.178  video_capture NOTE Number of frames received: 2946
     2022-11-01 12:51:12.178  video_capture NOTE Current avg frame rate (per second): 24.965
     2022-11-01 12:51:13.178  video_capture NOTE Profiler info...
     2022-11-01 12:51:13.178  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:51:13.178  video_capture NOTE Number of frames received: 2970
     2022-11-01 12:51:13.178  video_capture NOTE Current avg frame rate (per second): 24.9576
     2022-11-01 12:51:14.178  video_capture NOTE Profiler info...
     2022-11-01 12:51:14.178  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-01 12:51:14.178  video_capture NOTE Number of frames received: 2996
     2022-11-01 12:51:14.178  video_capture NOTE Current avg frame rate (per second): 24.9592
     2022-11-01 12:51:14.344  video_capture INFO v4l2if_mainloop: CAPTURE TERMINATION REQUESTED.
     2022-11-01 12:51:14.350  video_capture DBUG vidcap_v4l2_driver_interface::run() - terminating the video_profiler thread.
     2022-11-01 12:51:14.350  video_capture INFO vidcap_v4l2_driver_interface: NORMAL TERMINATION REQUESTED
     2022-11-01 12:51:14.547  video_capture DBUG MAIN: Video Capture thread is done. Cleanup and terminate.
     2022-11-01 12:51:14.547  video_capture DBUG main_video_capture:  terminating queue thread.
     2022-11-01 12:51:14.547  video_capture INFO Terminating the logger.

[(Back to the top)](#video-capture)

#### Example 3: When Things Go Terribly Wrong  

This example focuses on one way to get to the cause of something that went wrong.  This is just one
example, but it does point to a decent way to get some answers.   Dealing with an anomaly, I try to 
find out what actually caused a problem, regardless of who's at fault.  Plenty of time to blame someone 
else later.  This particular example happens occasionally, and has to do with the USB connection to the 
camera, the V4L2 driver, and the software layers "above" that, which include a lot of software I've 
written.  To reproduce the problem, I configure the system to grab frames in YUYV pixel format (much like 
I do in the previous example), and then switch back to H264 (much like I do in this example below). Here's what happens:       
     
First, run the command:   

     $ main_video_capture -fc 300 -pr 100
          
This is meant to go to the default H264 pixel format. The Output on the screen:     
     
     Frame count is set to 300(int) = 300(string)
     Command line parsing:
         Logging of initialization lines is set to false.
         redirect stderr to file is set to true
         Stderr output from the process streamed to, will be redirected to /dev/null
         write-frames-to-file is set to false, file name is video_capture.data
         Profiling is set to enabled, timeslice = 100.
         Video frame grabber device name is set to /dev/video0
         Video pixel format is set to: V4L2_PIX_FMT_H264: H264 with start codes
         use_other_proc is set to false
         test_suspend_resume is set to false
         Video frame grabber name is set to v4l2
         Log level is set to 0 = DBUG
         Frame count is set to 300(int) = 300(string)
     
     Log file: video_capture_log.txt
     driver: frame: 1920 x 1080
     driver: pixel format set to 0x34363248 - H264: H264 with start codes
     
     Exception thrown: v4l2if_start_capturing (MMAP): ioctl VIDIOC_STREAMON/V4L2_BUF_TYPE_VIDEO_CAPTURE \
                       error, errno=110: Connection timed out ...aborting.
     ERROR:  Program terminated.

So there's one problem. The V4L2 linux driver is being asked to connect to the USB camera, and fails on one of 
the **ioctl()** system calls that is needed. I know exactly where this error came from, since I wrote the 
**throw** code to throw this 
exception, as well as the **try/catch** statements that produce this output.  Yet there are no conclusions to be drawn 
from this at this time.  We have to see the files created, as well as the detail in the log file to give us more 
information.  

[(Back to the top)](#video-capture)

The list of files created during the run: 

     # ls -ltrh 
         . . . . 
     -rwxr-xr-x 1 andrew andrew 631K Nov  1 08:54 main_video_capture
     -rw-r--r-- 1 andrew andrew 401M Nov  1 12:51 video_capture.mp4
     -rw-r--r-- 1 andrew andrew 6.0K Nov  1 14:26 video_capture_log.txt
     
Notice the timestamp on the mp4 file.  It is more than an hour before the timestamp on the log file.  It must be 
the mp4 file created in the previous YUYV run, and is irrelevant to this run.  The conclusion is that the driver interface 
code never even came to the point of writing any data to the **ffmpeg** utility.  Viewing the file with **vlc** 
indeed shows the 640x480 video produced for YUYV, not the 1920x1080 format we're expecting.    

The log file exists, though, so let's take a look at it:  

     $ cat video_capture_log.txt
     2022-11-01 14:26:40.872  video_capture INFO START OF NEW VIDEO CAPTURE RUN
     2022-11-01 14:26:40.872  video_capture DBUG Current option values after getting shared_ptr<> to Log::Logger:
          Log Level 0
          Log level string DBUG
          Log channel name video_capture
          Log file name video_capture_log.txt
          Output to console disabled
          Output to log file enabled
     
     2022-11-01 14:26:40.873  video_capture INFO 
     
     Logger setup is complete.
     
     2022-11-01 14:26:40.873  video_capture INFO 
     2022-11-01 14:26:40.873  video_capture INFO The last few lines of deferred output from app initialization are shown here.   ******
     2022-11-01 14:26:40.873  video_capture INFO For the full set of deferred lines, use the -loginit flag on the command line.  ******
     2022-11-01 14:26:40.873  video_capture INFO DELAYED: .  .  .  . . . .
     
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
     From JSON:  Set raw video output process command to: ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4
     
     2022-11-01 14:26:40.873  video_capture INFO DELAYED: .  .  .  . . . .
     
     Command line parsing:
         Logging of initialization lines is set to false.
         redirect stderr to file is set to true
         Stderr output from the process streamed to, will be redirected to /dev/null
         write-frames-to-file is set to false, file name is video_capture.data
         Profiling is set to enabled, timeslice = 100.
         Video frame grabber device name is set to /dev/video0
         Video pixel format is set to: V4L2_PIX_FMT_H264: H264 with start codes
         use_other_proc is set to false
         test_suspend_resume is set to false
         Video frame grabber name is set to v4l2
         Log level is set to 0 = DBUG
         Frame count is set to 300(int) = 300(string)

So far so good.  All the options and settings are configured correctly for this run.

[(Back to the top)](#video-capture)

     2022-11-01 14:26:40.873  video_capture DBUG main_video_capture:  started video profiler thread
     2022-11-01 14:26:40.873  video_capture DBUG main_video_capture:  the video capture thread will kick-start the video_profiler operations.
     2022-11-01 14:26:40.873  video_capture DBUG Profiler thread started...
     2022-11-01 14:26:40.873  video_capture NOTE Profiler thread: skipping first frame to establish a duration baseline.
     2022-11-01 14:26:40.873  video_capture DBUG main_video_capture: kick-starting the queue operations.
     2022-11-01 14:26:40.873  video_capture DBUG main_video_capture:  starting the video capture thread.
     2022-11-01 14:26:40.873  video_capture DBUG raw_buffer_queue_handler: Updated output process to: \
                              ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4 2> /dev/null 
     2022-11-01 14:26:40.876  video_capture DBUG main_video_capture:  kick-starting the video capture operations.
     2022-11-01 14:26:40.876  video_capture INFO Video Capture thread: Requesting the v4l2 frame-grabber.
     2022-11-01 14:26:40.876  video_capture INFO Video Capture thread: The list of available frame grabbers in the json \
                                                                       config file is: opencv v4l2 
     2022-11-01 14:26:40.876  video_capture INFO Video Capture thread: Picking the v4l2 frame-grabber.
     2022-11-01 14:26:40.876  video_capture INFO Video Capture thread: Interface used is v4l2
     2022-11-01 14:26:40.876  video_capture DBUG Started the process "ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec \
                                                                             copy video_capture.mp4 2> /dev/null".
     2022-11-01 14:26:40.876  video_capture DBUG In VideoCapture::raw_buffer_queue_handler(): Successfully started \
                                                "ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4".
     2022-11-01 14:26:40.948  video_capture INFO Device /dev/video0
     2022-11-01 14:26:40.949  video_capture DBUG Set video format to (1920 x 1080), pixel format is V4L2_PIX_FMT_H264: H264 with start codes
     2022-11-01 14:26:40.949  video_capture DBUG driver: frame: 1920 x 1080
     2022-11-01 14:26:40.949  video_capture DBUG driver: pixel format set to 0x34363248 - H264: H264 with start codes
     2022-11-01 14:26:40.949  video_capture DBUG driver: bytes required: 4147200
     2022-11-01 14:26:40.949  video_capture DBUG driver: I/O METHOD: IO_METHOD_MMAP
     2022-11-01 14:26:51.116  video_capture EROR v4l2if_start_capturing (MMAP): \
                   ioctl VIDIOC_STREAMON/V4L2_BUF_TYPE_VIDEO_CAPTURE error, errno=110: Connection timed out ...aborting.
     2022-11-01 14:26:51.116  video_capture EROR vidcap_v4l2_driver_interface:   

And there's the error.  (**Hint:** to find the error in a long log file, search for the string **EROR** - this is the token printed 
out by *LoggerCpp* for log line that the programmer classifies as an error).  So the first error that shows up in the log originates 
with the V4L2 linux driver, and it shows the same error we saw on the display after starting this run.     

             ****************************************
             ***** ERROR TERMINATION REQUESTED. *****
             ****************************************
     
     2022-11-01 14:26:51.116  video_capture DBUG vidcap_v4l2_driver_interface::run() - terminating the video_profiler thread.
     2022-11-01 14:26:51.117  video_capture EROR vidcap_v4l2_driver_interface::run(): Got exception running the video capture: \
                       v4l2if_start_capturing (MMAP): ioctl VIDIOC_STREAMON/V4L2_BUF_TYPE_VIDEO_CAPTURE error, errno=110: \
                       Connection timed out ...aborting.. Aborting...
     2022-11-01 14:26:51.381  video_capture DBUG MAIN: ERROR: Video Capture thread terminating. Cleanup and terminate.
     2022-11-01 14:26:51.381  video_capture DBUG main_video_capture:  terminating queue thread.
     2022-11-01 14:26:51.381  video_capture EROR ERROR:  Program terminated.
     2022-11-01 14:26:51.381  video_capture DBUG Queue thread terminating ...
     2022-11-01 14:26:51.381  video_capture DBUG Shutting down the process "ffmpeg -nostdin -y -f h264 \
                                                   -i  pipe:0 -vcodec copy video_capture.mp4" (fflush, pclose(): 
     2022-11-01 14:26:51.381  video_capture INFO Terminating the logger.   

So it seems like the main thread acted appropriately, with the only difference is that it knows that an error occured.   
    
It would be easy to blame either the V4L2 driver at this point, or the USB driver/subsystem, but there is a lot more work 
to be done here to narrow down the possibilities:  Connect a different USB camera to the system and try to reproduce.  Or, 
move the USB connection from the USB hub it was connected to, to one of the computer's USB connections (which is also a hub,
but internal to the system). Another possibility is to run a test scenario that I use regularly, where the **-fn** flag is 
used on the command line, as well as the **-use-other-proc** flag which, if configured properly, would create **two** .data 
files which should be identical.  See the next example (but don't expect answers for the issue just shown above, since 
that is not the focus of this document.  More work to be done, and not too satisfying. Almost like 
one of those detective movies on TV, but you haven't figured out by the end of the movie "so who dunnit?".  To be 
continued.    

[(Back to the top)](#video-capture)

#### Example 4: Is This Working?  

This example shows a quick "sanity test" on the system. The question "is this thing working at all?" does come up 
when doing battle with a nasty bug, or json syntax issues, or whatever, and this example puts that question to rest very quickly 
(one way or another).

The **main_video_capture** app has two different and unrelated ways to write the raw video stream that comes from 
the driver, to a file.  By convention (here), the data file suffix is ".data".     

One way, is to write the stream out to a .data file using the **-fn** flag.

The other is to write the stream out to a (different) .data file, using the **-proc-redir** and **-use-other-proc** 
flags in conjunction with each other.   

As an aside, during this operation, when the two different methods are both enabled, only a single copy of the data is done. 
Any new buffer presented to the driver interface code, is copied into a **new** fixed array of identical size to the 
driver's buffer, and a std::shared_ptr<> is established for it.  The shared_ptr<> is added to the ring buffer (also 
referred to here as "the queue"), where the queue thread finds it, and uses the shared_ptr<> to both write the data 
to the output (.data) file, as well as to write the data to a process established and running 
which in this case is the linux **dd** utility, which in turn writes the output to a different .data file.     

Back to our example. Please look up the description for these three flags in the first sections of this document, 
as well as the **video_capture.json** configuration file, to make the connections between the flags, the file names, 
as well as the specific actions that are dependent on the pixel format chosen for this opration.    

The end result is that this is what happens to both data streams:    
  * The *-fn* flag causes the video stream data to be written to the file **video_capture.data** (regardless of which pixel format is used). 
  * The **-proc-redir** and **-use-other-proc** flags cause the video stream data to be piped to the process "**dd of=video_capture.dd.data 2>/dev/null**", which writes out the stream in the file **video_capture.dd.data**.  

**BOTH FILES SHOULD BE IDENTICAL**.  If they're not, then something is wrong.  

[(Back to the top)](#video-capture)

Let's run the program:     
 
     $ main_video_capture -fn -pr -fc 200 -use-other-proc -pf h264 
         Frame count is set to 200(int) = 200(string)
     Command line parsing:
         Logging of initialization lines is set to false.
         redirect stderr to file is set to true
         Stderr output from the process streamed to, will be redirected to /dev/null
         write-frames-to-file is set to true, file name is video_capture.data
         Profiling is set to enabled, timeslice = 800.
         Video frame grabber device name is set to /dev/video0
         Video pixel format is set to: V4L2_PIX_FMT_H264: H264 with start codes
         use_other_proc is set to true
         test_suspend_resume is set to false
         Video frame grabber name is set to v4l2
         Log level is set to 0 = DBUG
         Frame count is set to 200(int) = 200(string)
     
     Log file: video_capture_log.txt
     driver: frame: 1920 x 1080
     driver: pixel format set to 0x34363248 - H264: H264 with start codes
     $

The output on the diplay shows the options selected. The first thing to check are the 
existence and size of both output .data files.     
     
     $ ls -ltrh 
         .  .  .  .
     -rwxr-xr-x 1 andrew andrew  645144 Nov  1 08:54 main_video_capture
     -rw-r--r-- 1 andrew andrew    8340 Nov  2 03:28 video_capture_log.txt
     -rw-r--r-- 1 andrew andrew 8319479 Nov  2 03:28 video_capture.dd.data
     -rw-r--r-- 1 andrew andrew 8319479 Nov  2 03:28 video_capture.data

This shows that both files have an identical size (which is a good thing). If the sizes were 
not identical in size (to the byte), this would indicate a problem.  Next, use the linux **cmp** 
utility to compare both files byte for byte. The **cmp** utility is terse, and if everything compared well, it 
stays silent and exits with a 0 exit code.    
     
     $ cmp video_capture.dd.data video_capture.data; echo $?
     0

[(Back to the top)](#video-capture)

At this point, the test is done, and the result was **success!**. If you're like me, though, you check 
a few more things just to make sure that the **cmp** utility is still working after all these years (joking).   

Next, you can convert one of the .data files to a .mp4 file so it can be viewed.  Either of the files can 
be used, because **cmp** compared them and found them identical.     

There's an undocumented (by me) script installed in the same directory that the program is run out of, called 
**convert_raw_capture.bash**.   You should NOT run this file, but just pick one of the **ffmpeg** commands listed 
in it (based on whether the pixel format used during the run was **h264** or **yuyv**):     

     $ cat convert_raw_capture.bash 
     #!/bin/bash
     
     # This converts the H264 output from video_capture.dd.data to video_capture_dd.mp4.
     
     ffmpeg -nostdin -y -f h264 -i video_capture.dd.data -vcodec copy video_capture_dd.mp4.
     
     # Alternatively, this converts the YUYV output from video_capture.dd.data to video_capture_dd.mp4.
     
     ffmpeg  -nostdin -y -f rawvideo -vcodec rawvideo -s 640x480 -r 25 -pix_fmt yuyv422 -i video_capture.dd.data \
            -c:v libx264 -preset ultrafast -qp 0 video_capture_dd.mp4    

We're going to pick the first (**H264**) version of the command, since we used h264 pixel format during the test.    

     $
     $ ffmpeg -nostdin -y -f h264 -i video_capture.dd.data -vcodec copy video_capture_dd.mp4 
     ffmpeg version 4.3.4-0+deb11u1 Copyright (c) 2000-2021 the FFmpeg developers
       built with gcc 10 (Debian 10.2.1-6)
       configuration: --prefix=/usr --extra-version=0+deb11u1 --toolchain=hardened \
                      --libdir=/usr/lib/x86_64-linux-gnu --incdir=/usr/include/x86_64-linux-gnu --arch=amd64 \
                      --enable-gpl --disable-stripping --enable-avresample --disable-filter=resample \
                      --enable-gnutls --enable-ladspa --enable-libaom --enable-libass --enable-libbluray \
                      --enable-libbs2b --enable-libcaca --enable-libcdio --enable-libcodec2 --enable-libdav1d \
                      --enable-libflite --enable-libfontconfig --enable-libfreetype --enable-libfribidi \
                      --enable-libgme --enable-libgsm --enable-libjack --enable-libmp3lame --enable-libmysofa \
                      --enable-libopenjpeg --enable-libopenmpt --enable-libopus --enable-libpulse \
                      --enable-librabbitmq --enable-librsvg --enable-librubberband --enable-libshine \
                      --enable-libsnappy --enable-libsoxr --enable-libspeex --enable-libsrt --enable-libssh \
                      --enable-libtheora --enable-libtwolame --enable-libvidstab --enable-libvorbis \
                      --enable-libvpx --enable-libwavpack --enable-libwebp --enable-libx265 --enable-libxml2 \
                      --enable-libxvid --enable-libzmq --enable-libzvbi --enable-lv2 --enable-omx --enable-openal \
                      --enable-opencl --enable-opengl --enable-sdl2 --enable-pocketsphinx --enable-libmfx \
                      --enable-libdc1394 --enable-libdrm --enable-libiec61883 --enable-chromaprint --enable-frei0r \
                      --enable-libx264 --enable-shared
       libavutil      56. 51.100 / 56. 51.100
       libavcodec     58. 91.100 / 58. 91.100
       libavformat    58. 45.100 / 58. 45.100
       libavdevice    58. 10.100 / 58. 10.100
       libavfilter     7. 85.100 /  7. 85.100
       libavresample   4.  0.  0 /  4.  0.  0
       libswscale      5.  7.100 /  5.  7.100
       libswresample   3.  7.100 /  3.  7.100
       libpostproc    55.  7.100 / 55.  7.100
     [h264 @ 0x563e55abe2c0] Stream #0: not enough frames to estimate rate; consider increasing probesize
     Input #0, h264, from 'video_capture.dd.data':
       Duration: N/A, bitrate: N/A
         Stream #0:0: Video: h264 (Baseline), yuvj420p(pc, bt709, progressive), 1920x1080, 25 fps, 25 tbr, \
         1200k tbn, 50 tbc
     Output #0, mp4, to 'video_capture_dd.mp4':
       Metadata:
         encoder         : Lavf58.45.100
         Stream #0:0: Video: h264 (Baseline) (avc1 / 0x31637661), yuvj420p(pc, bt709, progressive), 1920x1080, \
         q=2-31, 25 fps, 25 tbr, 1200k tbn, 1200k tbc
     Stream mapping:
       Stream #0:0 -> #0:0 (copy)
     [mp4 @ 0x563e55ae2f40] Timestamps are unset in a packet for stream 0. This is deprecated and will stop \
     working in the future. Fix your code to set the timestamps properly
     frame=  200 fps=0.0 q=-1.0 Lsize=    8126kB time=00:00:07.96 bitrate=8363.0kbits/s speed= 316x    
     video:8124kB audio:0kB subtitle:0kB other streams:0kB global headers:0kB muxing overhead: 0.020638%
     $

[(Back to the top)](#video-capture)

Next, run the resulting mp4 file with a viewer, for a satisfying 8 second video clip of something (it's 
usually a wall in my case):     

     $ vlc video_capture_dd.mp4

And we're done.  The next section is quite unneccesary, but is provided for the sake of completeness.     

     $ cat video_capture_log.txt
     
     2022-11-02 03:28:43.998  video_capture INFO START OF NEW VIDEO CAPTURE RUN
     2022-11-02 03:28:43.998  video_capture DBUG Current option values after getting shared_ptr<> to Log::Logger:
          Log Level 0
          Log level string DBUG
          Log channel name video_capture
          Log file name video_capture_log.txt
          Output to console disabled
          Output to log file enabled
     
     2022-11-02 03:28:43.998  video_capture INFO 
     
     Logger setup is complete.
     
     2022-11-02 03:28:43.998  video_capture INFO 
     2022-11-02 03:28:43.998  video_capture INFO The last few lines of deferred output from app initialization are shown here.   ******
     2022-11-02 03:28:43.998  video_capture INFO For the full set of deferred lines, use the -loginit flag on the command line.  ******
     2022-11-02 03:28:43.998  video_capture INFO DELAYED: .  .  .  . . . .
     
     From JSON:  Getting available pixel formats for interface "v4l2":
             {  h264  other  yuyv    }
     From JSON:  Set logger channel-name to: video_capture
     From JSON:  Set logger file-name to: video_capture_log.txt
     From JSON:  Set default logger log level to: DBUG
     From JSON:  Enable writing raw video frames to output file: false
     From JSON:  Set raw video output file name to: video_capture.data
     From JSON:  Enable writing raw video frames to process: true
     From JSON:  Enable profiling: false
     From JSON:  Set milliseconds between profile snapshots to: 800
     From JSON:  Set default video-frame-grabber to: v4l2
     From JSON:  Set number of frames to grab (framecount) to: 20
     From JSON:  v4l2 is labeled as: V4L2
     From JSON:  Set v4l2 device name to /dev/video0
     From JSON:  Set v4l2 pixel format to V4L2_PIX_FMT_H264: H264 with start codes
     From JSON:  Set raw video output process command to: ffmpeg -nostdin -y -f h264 -i  pipe:0 -vcodec copy video_capture.mp4
     
     2022-11-02 03:28:43.998  video_capture INFO DELAYED: .  .  .  . . . .
     
     Command line parsing:
         Logging of initialization lines is set to false.
         redirect stderr to file is set to true
         Stderr output from the process streamed to, will be redirected to /dev/null
         write-frames-to-file is set to true, file name is video_capture.data
         Profiling is set to enabled, timeslice = 800.
         Video frame grabber device name is set to /dev/video0
         Video pixel format is set to: V4L2_PIX_FMT_H264: H264 with start codes
         use_other_proc is set to true
         test_suspend_resume is set to false
         Video frame grabber name is set to v4l2
         Log level is set to 0 = DBUG
         Frame count is set to 200(int) = 200(string)
     
     2022-11-02 03:28:43.998  video_capture DBUG main_video_capture:  started video profiler thread
     2022-11-02 03:28:43.998  video_capture DBUG main_video_capture:  the video capture thread will kick-start the video_profiler operations.
     2022-11-02 03:28:43.998  video_capture DBUG Profiler thread started...
     2022-11-02 03:28:43.998  video_capture NOTE Profiler thread: skipping first frame to establish a duration baseline.
     2022-11-02 03:28:43.998  video_capture DBUG main_video_capture: kick-starting the queue operations.
     2022-11-02 03:28:43.998  video_capture DBUG main_video_capture:  starting the video capture thread.
     2022-11-02 03:28:43.999  video_capture DBUG main_video_capture:  kick-starting the video capture operations.
     2022-11-02 03:28:43.999  video_capture DBUG Created/truncated output file "video_capture.data"
     2022-11-02 03:28:43.999  video_capture DBUG In VideoCapture::raw_buffer_queue_handler(): created video_capture.data.
     2022-11-02 03:28:43.999  video_capture DBUG 
     raw_buffer_queue_handler: Updated output process to:  dd of=video_capture.dd.data 2> /dev/null
     2022-11-02 03:28:43.999  video_capture INFO Video Capture thread: Requesting the v4l2 frame-grabber.
     2022-11-02 03:28:43.999  video_capture INFO Video Capture thread: The list of available frame grabbers in the json config file is: opencv v4l2 
     2022-11-02 03:28:43.999  video_capture INFO Video Capture thread: Picking the v4l2 frame-grabber.
     2022-11-02 03:28:43.999  video_capture INFO Video Capture thread: Interface used is v4l2
     2022-11-02 03:28:43.999  video_capture DBUG Started the process "dd of=video_capture.dd.data 2> /dev/null".
     2022-11-02 03:28:43.999  video_capture DBUG In VideoCapture::raw_buffer_queue_handler(): Successfully started "dd of=video_capture.dd.data".
     2022-11-02 03:28:44.068  video_capture INFO Device /dev/video0
     2022-11-02 03:28:44.070  video_capture DBUG Set video format to (1920 x 1080), pixel format is V4L2_PIX_FMT_H264: H264 with start codes
     2022-11-02 03:28:44.070  video_capture DBUG driver: frame: 1920 x 1080
     2022-11-02 03:28:44.070  video_capture DBUG driver: pixel format set to 0x34363248 - H264: H264 with start codes
     2022-11-02 03:28:44.070  video_capture DBUG driver: bytes required: 4147200
     2022-11-02 03:28:44.070  video_capture DBUG driver: I/O METHOD: IO_METHOD_MMAP
     2022-11-02 03:28:44.262  video_capture DBUG vidcap_v4l2_driver_interface::run() - kick-starting the video_profiler operations.
     2022-11-02 03:28:44.263  video_capture NOTE Profiler info...
     2022-11-02 03:28:44.263  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:44.263  video_capture NOTE Number of frames received: 0
     2022-11-02 03:28:44.263  video_capture NOTE Current avg frame rate (per second): 0
     2022-11-02 03:28:45.063  video_capture NOTE Profiler info...
     2022-11-02 03:28:45.063  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:45.063  video_capture NOTE Number of frames received: 23
     2022-11-02 03:28:45.063  video_capture NOTE Current avg frame rate (per second): 30.2234
     2022-11-02 03:28:45.864  video_capture NOTE Profiler info...
     2022-11-02 03:28:45.864  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:45.864  video_capture NOTE Number of frames received: 44
     2022-11-02 03:28:45.864  video_capture NOTE Current avg frame rate (per second): 27.6208
     2022-11-02 03:28:46.664  video_capture NOTE Profiler info...
     2022-11-02 03:28:46.664  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:46.664  video_capture NOTE Number of frames received: 63
     2022-11-02 03:28:46.664  video_capture NOTE Current avg frame rate (per second): 26.6385
     2022-11-02 03:28:47.465  video_capture NOTE Profiler info...
     2022-11-02 03:28:47.465  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:47.465  video_capture NOTE Number of frames received: 84
     2022-11-02 03:28:47.465  video_capture NOTE Current avg frame rate (per second): 26.3158
     2022-11-02 03:28:48.265  video_capture NOTE Profiler info...
     2022-11-02 03:28:48.265  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:48.265  video_capture NOTE Number of frames received: 103
     2022-11-02 03:28:48.265  video_capture NOTE Current avg frame rate (per second): 25.9773
     2022-11-02 03:28:49.065  video_capture NOTE Profiler info...
     2022-11-02 03:28:49.066  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:49.066  video_capture NOTE Number of frames received: 123
     2022-11-02 03:28:49.066  video_capture NOTE Current avg frame rate (per second): 25.8132
     2022-11-02 03:28:49.866  video_capture NOTE Profiler info...
     2022-11-02 03:28:49.866  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:49.866  video_capture NOTE Number of frames received: 143
     2022-11-02 03:28:49.866  video_capture NOTE Current avg frame rate (per second): 25.7518
     2022-11-02 03:28:50.666  video_capture NOTE Profiler info...
     2022-11-02 03:28:50.666  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:50.666  video_capture NOTE Number of frames received: 163
     2022-11-02 03:28:50.667  video_capture NOTE Current avg frame rate (per second): 25.6088
     2022-11-02 03:28:51.467  video_capture NOTE Profiler info...
     2022-11-02 03:28:51.467  video_capture NOTE Shared pointers in the ring buffer: 0
     2022-11-02 03:28:51.467  video_capture NOTE Number of frames received: 183
     2022-11-02 03:28:51.467  video_capture NOTE Current avg frame rate (per second): 25.5551
     2022-11-02 03:28:52.103  video_capture INFO v4l2if_mainloop: CAPTURE TERMINATION REQUESTED.
     2022-11-02 03:28:52.104  video_capture DBUG MAIN: Video Capture thread is done. Cleanup and terminate.
     2022-11-02 03:28:52.104  video_capture DBUG main_video_capture:  terminating queue thread.
     2022-11-02 03:28:52.104  video_capture INFO Terminating the logger.

[(Back to the top)](#video-capture)

   ________   
    
    
# Please Note:
This is work in progress -- I'm writing code and uploading the sources to the repository while ensuring that everything is tested, building properly (at least on my system), and working. There are no Github "collaborators" established for the project (that would happen only in rare cases). In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.     
     
Thank you.     
     
### SEE ALSO:    

The [README.md file](../../README.md) in the root **Samples** folder.    
The [README.md file](../README.md) in the **source** folder.    
The [README file in the Video/src/plugins project](../Video/src/plugins/README.md).        
    
The file [LICENSE](../../LICENSE) in the root **Samples** folder, contains the legal language covering distribution and use of the sources in this project (that belong to me).    
The file [LICENSE](../3rdparty/JsonCpp/JsonCpp-8190e06-2022-07-15/jsoncpp/LICENSE) covering **JsonCpp**.    
The file [LICENSE.txt](../3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8-modified/LICENSE.txt) covering **LoggerCpp**.     
    
