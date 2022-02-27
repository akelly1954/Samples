# Samples

# Welcome.  

If you are here to look at how to get things done (one way, anyways) or you are here to evaluate my skills for whatever purpose,
you might want to start with either libraries of objects, or with some main programs that use them, and work your way out from there.

## Intetesting main programs: 

These programs are used to test objects, as well as to stress-test them.  They don't always have a useful purpose beyond that.

**main_ntwk_basic_sock_server.cpp**   
found in *...Samples/source/EnetUtil/src/main_programs/*   
Currently, this is the program which uses most of the objects in these sources.  Threads, socket connections, condition variables, circular buffers (aka ring buffers or bounded buffers), and more.  This is a deeper dive into the code. Note: this object is still being modified and added to. Currently awaiting the basic client that exercises it.  

**main_condition_data.cpp**   
found in *...Samples/source/Util/src/main_programs*   
Exercises the condition_data object extensively for use from multiple threads. 
This program was used while testing the LoggerCpp library at the time when it was eveluated for use in these sources.

**main_circular_buffer.cpp**   
found in *...Samples/source/Util/src/main_programs*   
Exercises the ring buffer and shows functionality.

**NtwkFixedArray.hpp**     
found in *...Samples/source/EnetUtil/include/*   
This is a template based object which encapsulates an std::array<T,N> which is where the data is kept for most of the network based
objects covered here. The current implementation uses an std::array<T,N> where **T** is **uint8_t** (for std::array<uint8_t,N>), and **N** is the size of the std::array<> (number of elements). In addition to other facets, it demonstrates a decent implementation of how to use the **shared_from_this()** mechanism properly.  By having **shared_ptr<>**'s holding fixed size buffers, a program (like **main_ntwk_basic_sock_server.cpp** in this case) can move the data around by moving the **shared_ptr<>**'s around instead of having to copy the data from one array buffer to the next.  Worth a look. (Who uses std::array<>'s anyways?) 

**Includes:** *...Samples/source/EnetUtil/include/* and *...Samples/source/Util/include/* show the definition of objects used in the above programs.

**Shell scripts:**   
base-linux-build.bash (in *...Samples/source/*)   
linux-build.bash   (in *...Samples/source/EnetUtil/*)    
linux-build.bash   (in *...Samples/source/Util/*)     
bash_env.sh (in *...Samples/source/shell_env/*)   

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
This is work in progress -- I'm uploading my sources to the repository while ensuring that the code is tested, building properly (at least on my system), and working. So for a period of time I've got restrictions on interactions with the repositories that allow one to clone and/or download the code (to which you are welcome as per the LICENSE) but am not yet welcoming collaborators. Right now I only have some basic libraries and main programs that use/exercise them and there are more objects coming.  As soon as I introduce code that does something more useful, I'll remove the restrictions.  In the meantime, if there's something critically important you need to communicate, please email me at **andrew@akelly.com**.

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









