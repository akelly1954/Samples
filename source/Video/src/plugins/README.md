
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

The base class (video_plugin_base - instantiated in the main executable after the plugin is loaded) is initialized with some information about the loaded plugin,  Most importantly, the "interface pointer": 

    static VideoCapture::video_plugin_base* VideoCapture::video_plugin_base::interface_ptr;
    
is set in the base class to the address of the derived plugin object (using the v4l2 plugin as an example): 

    class vidcap_v4l2_driver_interface : virtual public video_plugin_base; 

which has just been loaded into the executable. (Getting to the derived methods of the loaded plugin later on during execution, simply becomes a matter of using the interface pointer with an instantiated derived virtual method).   
    
Some of the initialization done in the factory after the interface pointer has been set, is to call a few methods in the loaded plugin to get from the plugin its own idea of what type of data it uses, and what its capabilities are (for example - pixel formats' capabilities - see the virtual *probe_pixel_format_caps()* method in the plugin's derived object).     
     
**Developer TODO**: Implement the **create()** and **destroy()** methods in your new plugin (use the **class VideoCapture::vidcap_v4l2_driver_interface** definition as an example).    
     
**Please NOTE**:  The plugin factory **SHOULD NOT** be modified at all in this exercise, unless it is to fix a workaround for a well-known bug which you can see in **video_capture_plugin_factory::destroy_factory()** which I'm currently blaming on **::dlclose()** (not 100% sure about that though).    

[(Back to the top)](#video-capture-plugins)

#### About thread synchronization in main_video_capture

##### Very briefly 

The **video_capture thread** (which among other things starts the video capture plugin running), first kick-starts the **test thread** which optionally exercises the **suspend/resume** mechanism (this thread was started from main()).  By the time the **video_capture thread** starts, the **raw queue thread** (VideoCapture::raw_buffer_queue_handler) had already been started from main() as well.    
    
What all of these threads do once they are started, is **WAIT** -- (**Util::condition_data::wait_for_ready()** -- see Samples/Util/include/condition_data.hpp). This object encapsulates an **std::condition_variable** object.  You will see comments in the code referring to "kick-starting" threads - this basically means that the condition variable in each of the condition_data objects is going to be "satisfied" using either condition_data::flush() or condition_data::send_ready(), which allows the thread to begin operations.  That is the mechanism used to synchronize the start of operations of each of the threads discussed here. 

##### Drawbacks to this approach

The potential problem with using condition variables to synchronize between threads is that unless the state of each thread is meticulously managed, the thread can very easily hang.  This is because not much in the thread execution (or coming from other threads) can interrupt the "wait_for_ready()" call which keeps the thread in question from continuing.     
     
This means that the mechanism used to terminate this thread because of say, errors in other threads or any other reason, uses a **boolean** flag specific to each thread using this mechanism (typically you'll see **static bool s_terminated;** definition as a member of a class definition).   
    
Before using the **condition_data** variable (wait_for_ready()), the code must be locked, the termination boolean checked, and only if it is not set to **true**, can the thread call the wait_for_ready() method be made.   (See also the **set_terminated()** method in each of these threads to complete the picture).    

[(Back to the top)](#video-capture-plugins)

#### The video_capture thread 

Once this thread is started by **main()**, it first **WAIT**s for the condition_data object to let it continue.  It then goes through initialization of its own data, including preparing the output called for by the **-dr** command line flag, and more.    

At this point in the execution, the thread calls the first actual method in the video capture plugin:     

    video_plugin_base::interface_ptr->initialize();

This method initializes the plugin's copy of the logger's shared_ptr<> and sets the actual string with which the ::popen() call will be made (including stderr redirection).    

Next, it calls:    

    video_plugin_base::interface_ptr->run();

which does all the video capture work.  Soon after this plugin method returns, the program will be terminated.    

Within the video_capture plugin's ->run() method, early on, the **profiling thread** is kick-started right after actual video-capturing is started.  We kick-start the **profiling thread** as late as possible in order to prevent the profiler's statistics from being skewed as a result of waiting for the frames to start showing up.    
    
Also, within the video_capture plugin's ->run() method the following base class facilities are used:

(1) If profiling is enabled, once a buffer contains a complete video frame, the profiling statistics are incremented by one (this is thread-safe -- all locking is handled outside of this context):    

    if (!isterminated() && Video::vcGlobals::profiling_enabled) 
        lret = increment_one_frame();    // member of class vidcap_v4l2_driver_interface 

(2) Once a buffer contains a complete video frame, it is shipped to the raw buffer queue (this is thread-safe -- all locking is handled outside of this context):    

      // params are not shown here 
      vidcap_v4l2_driver_interface::v4l2if_process_image() 
      { 
          . . . 
          add_buffer_to_raw_queue();  // member of class vidcap_v4l2_driver_interface 
          . . . 
      } 
    
Lastly, please note that there is a distinction between normal termination and error termination.  As a rule, all error terminations also set the "normal termination" flag.  See the various plugin methods that call **set_terminated()** and **set_error_terminated()**.

[(Back to the top)](#video-capture-plugins)


#### This document will be further developed, but has the essentials in it for now.

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
    

