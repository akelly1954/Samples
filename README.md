# Samples

# Welcome.  

If you are here to look at how to get things done (one way, anyways) or you are here to evaluate my skills for whatever purpose,
you might want to start with either libraries of objects, or with some main programs that use them, and work your way out from there.

## Intetesting main programs: 

These programs are used to test objects, as well as to stress-test them.  They don't always have a useful purpose beyond that.

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









