#
# Need CMAKE 3.10 and above to use --std=c++17
# On entry, relies on ${SampleRoot_DIR} to be set to the path for the repository root
# and ${LoggerCppSource_DIR} to be set to the LoggerCpp "src/" directory path (where include/ 
# can be found).
#
cmake_minimum_required(VERSION 3.10)

set (CMAKE_VERBOSE_MAKEFILE ON)

if (NOT WIN32)
    message (STATUS "Linux system detected." )

    set (DBG "")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${LoggerCppSource_DIR}/include")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/lib/gcc/x86_64-linux-gnu/10/include")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/x86_64-linux-gnu")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/c++/10")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/x86_64-linux-gnu/c++/10")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/c++/10/backward")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D \"DBG_BUILD=\\\"${DBG}\\\"\"" )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -ldl")

    set (CMAKE_C_COMPILER /usr/bin/gcc)
    set (CMAKE_CXX_COMPILER /usr/bin/g++)
    set (CMAKE_CXX_STANDARD 17)
    set (CMAKE_CXX_STANDARD_REQUIRED ON)
    set (CMAKE_CXX_EXTENSIONS OFF)

    find_package( Threads )
else()
    message (STATUS "WINDOWS system detected. - NOT IMPLEMENTED FOR THIS PROJECT YET" )
endif()

set( TOOLS_INCLUDED:BOOL ON )

set( LoggerCpp_HEADERS "${LoggerCppSource_DIR}/include" )

set( LoggerCpp_BASE "${SampleRoot_DIR}/source/3rdparty/LoggerCpp" )
set( LoggerCpp_LIB "${LoggerCpp_BASE}/build/libLoggerCpp.a")
set( LoggerCpp_LIBTYPE "STATIC")

