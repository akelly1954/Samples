# WORK IN PROGRESS 

# Video Capture Plugins

### Table of Contents
  1. [Early in program execution](#early-in-program-execution)    
  2. [Where to find things in the source](#where-to-find-things)    
     - [main program](#main-program)    
     - [JSON config file](#json-config-file)    
     - [JSON and runtime configuration](#json-and-runtime-configuration)    
     - [Logger operations](#logger-operations)    
     - [Command line parsing](#command-line-parsing)    
     - [Plugin loading and configuration](#plugin-loading-and-configuration)    
     - [Video capture thread - plugin initialization and runtime](#video-capture-thread---plugin-initialization-and-runtime)    
     - [The declaration of create() and destroy() used by the plugin factory](#the-declaration-of-create-and-destroy-used-by-the-plugin-factory)    
  3. [The Plugin Interface](#the-plugin-interface)    
      - [The plugin factory](#the-plugin-factory)    
      - [About thread synchronization in main_video_capture](#about-thread-synchronization-in-main_video_capture)    
      - [Very Briefly](#very-briefly)    
      - [Drawbacks to this approach](#drawbacks-to-this-approach) 
      - [The video_capture thread](#the-video_capture-thread)    
  4. [SEE ALSO](#see-also)    
   
## Early in program execution     
     
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
    
The more useful way to develop a video capture plugin is to run it once using the **-dr** command line flag when a working video capture plugin is being used (**v4l2** for example), and saving the **video_capture_log.txt** that was produced during the run, before running **main_video_capture** with the plugin you are developing (also using the **-dr** flag).  Comparing what is shown in the saved output file against the contents of your **video_capture_log.txt** is very very useful for debugging. (This would require you to have a USB webcam which works with the v4l2 interface used in this program).   

[(Back to the top)](#video-capture-plugins)

## Where to find things     
     
This might help finding where the sources for specific objects can be found.    
    
### main program: 
    Samples/source/Video/src/main_programs/main_video_capture.cpp 

### JSON config file: 
    Samples/source/Video/src/main_programs/video_capture.json 

### JSON and runtime configuration:   
    Samples/source/Video/src/main_programs/config_tools.cpp 
    Samples/source/Video/src/main_programs/config_tools.hpp
    Samples/source/Util/src/ConfigSingleton.cpp 
    Samples/source/Util/include/ConfigSingleton.hpp 

### Logger operations:    
    Samples/source/Video/src/main_programs/logger_tools.hpp
    Samples/source/Video/src/main_programs/logger_tools.cpp
    Samples/source/Util/src/MainLogger.cpp
    Samples/source/Util/include/MainLogger.hpp 

### Command line parsing: 
    Samples/source/Video/src/main_programs/parse_tools.cpp
    Samples/source/Video/src/main_programs/parse_tools.hpp
    Samples/source/Video/src/common/video_capture_commandline.cpp
    Samples/source/Video/include/video_capture_commandline.hpp
    Samples/source/Util/src/commandline.cpp
    Samples/source/Util/include/commandline.hpp 

[(Back to the top)](#video-capture-plugins)

### Plugin loading and configuration: 
    Samples/source/Video/src/common/vidcap_plugin_factory.cpp 
    Samples/source/Video/include/vidcap_plugin_factory.hpp 
    
(The above loading mechanism runs in the **main program thread**). 

### Video capture thread - plugin initialization and runtime:   
    Samples/source/Video/src/common/vidcap_capture_thread.cpp 
    Samples/source/Video/include/vidcap_capture_thread.hpp 
    Samples/source/Video/src/plugins/v4l2/vidcap_v4l2_driver_interface.cpp
    Samples/source/Video/include/plugins/vidcap_v4l2_driver_interface.hpp
    . . . and/or any other implemented plugin (opencv, etc) 
    
(The above **video_capture thread** initializes the already loaded plugin, and runs it to completion).   

### The declaration of create() and destroy() used by the plugin factory: 
    $ grep 'extern "C"' ... (all the Video source files) 
    Samples/source/Video/include/vidcap_plugin_factory.hpp:    extern "C" video_plugin_base* create(); 
    Samples/source/Video/include/vidcap_plugin_factory.hpp:    extern "C" void destroy(video_plugin_base* p); 
    Samples/source/Video/include/plugins/vidcap_v4l2_driver_interface.hpp:    extern "C" video_plugin_base* create() { 
    Samples/source/Video/include/plugins/vidcap_v4l2_driver_interface.hpp:    extern "C" void destroy(video_plugin_base* p) { 
     
    (each function definition in the plugin .hpp file has a matching declaration in its .cpp file - one set per plugin).
    
**NOTE**: Don't miss the last ten lines of **vidcap_v4l2_driver_interface.hpp** - if you got this far, this is probably what you're looking for...     

There are, of course, dozens more source files, but these are a good starting point.    

[(Back to the top)](#video-capture-plugins)

## The Plugin Interface

To explain the video capture plugin interface, we use the **vidcap_v4l2_driver_interface.hpp/.cpp** streaming interface which is a completely imlemented plugin which is working and is the prototype for other plugins.    
    
The video capture plugin involves a base class **class VideoCapture::video_plugin_base** which is a non-pure abstract object.  Some key items of the interface actually reside in the instantiated base class, although they are only accessible to the program by calling derived methods that are instantiated in the derived object which exists in the loaded plugin (for example **class VideoCapture::vidcap_v4l2_driver_interface : virtual public VideoCapture::video_plugin_base**).   

#### The plugin factory 

From the main thread, the **video_capture_plugin_factory::create_factory()** method locates the requested plugin by file/path name, loads it, and sets up the basic mechanism for starting up the loaded plugin.   
    
The system's **::dlopen()** and **::dlsym()** load the video capture plugin, and match up the two functions that are required by this interface:
    
    extern "C" video_plugin_base* create();
    extern "C" void destroy(video_plugin_base* p);   
    
See [The declaration of create() and destroy() used by the plugin factory](#the-declaration-of-create-and-destroy-used-by-the-plugin-factory) above.   

The base class (video_plugin_base - again, instantiated in the main executable, not the plugin) is initialized with some information about the loaded plugin,  Most importantly, the "interface pointer": 

    static video_plugin_base* video_plugin_base::vidcap_v4l2_driver_interface
    
is set in the base class to the address of the derived plugin object: 

    class vidcap_v4l2_driver_interface : virtual public video_plugin_base; 

which has just been loaded into the executable. (Getting to the derived methods of the loaded plugin later on during execution, simply becomes a matter of using the interface pointer with a derived virtual method).   
    
**Developer TODO**: Implement the **create()** and **destroy()** methods in your new plugin (use the **class VideoCapture::vidcap_v4l2_driver_interface** definition as an example).    
     
**Please NOTE**:  The plugin factory **SHOULD NOT** be modified at all in this exercise, unless it is to fix a workaround for a well-known bug which you can see in **video_capture_plugin_factory::destroy_factory()** which I'm currently blaming on **::dlclose()** (not sure about that though).    

[(Back to the top)](#video-capture-plugins)

## About thread synchronization in main_video_capture

#### Very briefly 

The **video_capture thread** (which among other things starts the video capture plugin running), also starts the **profiling thread**, as well as the **test thread** which optionally exercises the **suspend/resume** mechanism.  By the time the **video_capture thread** starts, the **raw queue thread** (VideoCapture::raw_buffer_queue_handler) has already been started from main().    
    
What all of these threads do once they are started, is **"wait"** on an object type **Util::condition_data** (Samples/Util/include/condition_data.hpp) which encapsulates an **std::condition_variable** object.  You will see comments in the code referring to "kick-starting" threads - this basically means that the condition variable in each of the condition_data objects is going to be "satisfied" using either condition_data::flush() or condition_data::send_ready(), which allows the thread to begin operations.  That is the mechanism used to synchronize the start of operations of each of the threads discussed here. 

#### Drawbacks to this approach










#### The video_capture thread 


























#### ... to be continued - still work in progress.



   __________________    
   
    
# Please Note:
This is work in progress -- I'm writing code and uploading the sources to the repository while ensuring that everything is tested, building properly (at least on my system), and working. There are no Github "collaborators" established for the project (that would happen only in rare cases). In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.     
     
Thank you.     
     
### SEE ALSO:    

The [README.md file in the root Samples folder](../../../../README.md).     
The [README.md file in the source/ folder](../../../../source/README.md).    
The [README.md file in the Video project](../../../Video/README.md).     
The [LICENSE](./LICENSE) in the root **Samples** folder covering the entirety of the **Samples** project..    
The [LICENSE](source/3rdparty/JsonCpp/JsonCpp-8190e06-2022-07-15/jsoncpp/LICENSE) covering **JsonCpp**.    
The [LICENSE](source/3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8-modified/LICENSE.txt) covering **LoggerCpp**.     
    

