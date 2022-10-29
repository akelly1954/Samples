# Video Capture 

Stream video frames from the source (a camera), through the linux driver, to (in this case) the process **main_video_capture**, which uses objects from this project (**Video**), as well as objects from the other projects in this repository.

## Some Notes

There are many many options and configurations that control how to get video from a camera to an end destination of a video pipeline.  As an example, run the help feature of the **ffmpeg** utility like this: **ffmpeg --help full**.  The scope is huge. In this project, the surface is barely skimmed, but it does a lot to demonstrate how to move data down the video pipline (currently only under linux), to be saved in a file, or piped to a cooperating application or utility (**ffmpeg** is used extensively), where it can be streamed to another process on the current system, or any other system on a connected network.  For exampele, in this project, the **main_video_player** uses **ffmpeg** internally to produce an mp4 file that any video player can pick up and display it on a screen.     
     
## Starting Slowly

To get started, let's take a look on how to run the utility with a bit of setup and a couple of exmples.  Let's assume we have a camera connected to a USB port on our system, and which we know to be working.  For example, run the **vlc** utility, and under the **Media --> Open Capture Device** menu, set the capture device to "/dev/video0" (on linux).  If the device drop-down menu does not show any entries, then the setup for the camera is not yet complete, or a few other possibilities that we're not going to cover here (disconnect/reconnect the usb line to the system, etc).  The actual device name can be something else, depending on the camera configuration.   
     
**So first**:     
     
The output shown below is from the utility run like this: **main_video_capture --help**.  Please take a look at the options to familiarize yourself with them, and we can then show a couple of examples:     
     
     $ main_video_capture --help   
            
     Usage:
     
       main_video_capture --help (or -h or help)
     
     Or:
     
       main_video_capture  
         
       [ -fn [ file-name ] ]     Turns on the "write-to-file" functionality (see JSON file).  The file-name
                                 parameter is the file which will be created to hold image frames. If it exists,
                                 the file will be truncated. If the file name is omitted, the default name
                                 "video_capture.data" will be used. (By default, the "write-to-file" capability
                                 is turned off in favor of the "write-to-process" member in the JSON config file.   
                                  
       [ -pr [ timeslice_ms]     Enable profiler stats. If specified, the optional parameter is the number
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

**NEXT**: Read the **source/README.md** information for more info. 









