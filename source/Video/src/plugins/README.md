# WORK IN PROGRESS 

# Video Capture Plugins

### Background     
     
The video capture plugin is loaded at runtime based on its entry in the JSON configuration file.  Since the 
plugin remains loaded for the duration of execution of the **main_video_capture** executable, we try to load it
into the **main()** program as early as possible in the program execution (and error-exit if there are problems).        
     
There are some competing needs at the beginning of main() execution:

1. Whether the program has been invoked with the **-h** or **--help** flag: if that happens, the only thing that
main() does is to emit the Usage screen and exit.    
   
2. Creation of the log file, which also needs to happen as early as possible during program execution. This winds up being delayed for a bit.       
    
3. Parsing of the JSON config file in order to initialize the **vcGlobals** object to proper initial defaults.  
     
4. Loading the video capture plugin (documented in this README).  
    
5. Full parsing of the command line:  This has to happen last in the initialization sequence since command line flags override the settings from the JSON config file.
     
These items of priority are resolved in the order shown above, except for the log file, which is created only after the full parsing of the command line, since the command line flags can override log file handling specified in the JSON file.    
    
Before log file creation, information meant for the log file is collected in std::string's and std::stringstream's  as appropriate, and is then streamed into the log file based on the runtime options chosen (above) right after the file is created.    

There is one command line flag (**-loginit**) which affects how much output from the five items listed above is actually written to the log file.  This flag is useful for debugging any/all of the initial steps listed above, and causes  the output collected before the log file is created to actually be written into the log file. If the flag is not specified on the command line, most of that collected output is discarded (it is quite verbose).          
     
Lastly, **the most relevant command line flag** for the task of developing and debugging a video capture plugin, is the (**-dr**) flag.  It collects the **current** runtime configuration of the running program after all initialization items (see above) have been completed without errors.  It shows the current (runtime) value of every relevant item, where to find it in the code - which **vcGlobals** member(s) affect it, which section in the JSON config file is being used, as well as which JSON file fields are relevant, and lastly, which command line flags can affect it. (BTW, the --loginit command line flag is not needed for this exercise).     
    
The more useful way to develop a plugin is to run it once using the **-dr** command line flag when a working video capture plugin is being used (**v4l2** for example), and saving the **video_capture_log.txt** that was produced during the run, before running **main_video_capture** with the plugin you are developing (also using the **-dr** flag).  Comparing what is shown in the saved output file against the contents of your **video_capture_log.txt** is very very useful for debugging.    
     
### Where to find things     
     
This might help finding where the sources for specific objects can be found.    
    
**main program**: 
``Samples/source/Video/src/main_programs/main_video_capture.cpp`` 

**json config file**: 
``Samples/source/Video/src/main_programs/video_capture.json``

**JSON and runtime configuration**:   
 
    Samples/source/Video/src/main_programs/config_tools.cpp 
    Samples/source/Video/src/main_programs/config_tools.hpp
    Samples/source/Util/src/ConfigSingleton.cpp 
    Samples/source/Util/include/ConfigSingleton.hpp 
    
**Logger operations**:    

    Samples/source/Video/src/main_programs/logger_tools.hpp
    Samples/source/Video/src/main_programs/logger_tools.cpp
    Samples/source/Util/src/MainLogger.cpp
    Samples/source/Util/include/MainLogger.hpp 

**Command line parsing**: 

    Samples/source/Video/src/main_programs/parse_tools.cpp
    Samples/source/Video/src/main_programs/parse_tools.hpp
    Samples/source/Video/src/common/video_capture_commandline.cpp
    Samples/source/Video/include/video_capture_commandline.hpp
    Samples/source/Util/src/commandline.cpp
    Samples/source/Util/include/commandline.hpp 

**Plugin loading and configuration**: 

     Samples/source/Video/src/common/vidcap_plugin_factory.cpp
     Samples/source/Video/include/vidcap_plugin_factory.hpp

**Video capture thread, plugin initialization and running**: 

     Samples/source/Video/src/common/vidcap_capture_thread.cpp 
     Samples/source/Video/include/vidcap_capture_thread.hpp
     
There are, of course, dozens more source files, but these are a good starting point.        
      
   __________________   
   
    
    
# Please Note:
This is work in progress -- I'm writing code and uploading the sources to the repository while ensuring that everything is tested, building properly (at least on my system), and working. There are no Github "collaborators" established for the project (that would happen only in rare cases). In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.     
     
Thank you.     
     
**SEE ALSO:**    

The [README.md file in the root Samples folder](../../../../README.md).     
The [README.md file in the source/ folder](../../../../source/README.md).    
The [README file in the Video project](../../../Video/README.md).     
The [LICENSE](./LICENSE) in the root **Samples** folder covering the entirety of the **Samples** project..    
The [LICENSE](source/3rdparty/JsonCpp/JsonCpp-8190e06-2022-07-15/jsoncpp/LICENSE) covering **JsonCpp**.    
The [LICENSE](source/3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8-modified/LICENSE.txt) covering **LoggerCpp**.     
    

