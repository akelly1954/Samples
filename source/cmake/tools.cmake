#
# Need CMAKE 3.10 and above to use --std=c++17
# On entry, relies on:
#
#   ${SampleRoot_DIR} - to be set to the path for the repository root
#   ${LoggerCppSource_DIR} to be set to the LoggerCpp "src/" directory path (where include/ can be found).
#   ${JsonCppSource_DIR} to be set to the JsonCpp "src/" directory path (where include/ can be found).
#
cmake_minimum_required(VERSION 3.10)

set (CMAKE_VERBOSE_MAKEFILE ON)

if (NOT WIN32)
    message (STATUS "Linux system detected." )

    set (DBG "")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${LoggerCppSource_DIR}/include")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${JsonCppSource_DIR}/include")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fPIC -std=c++17")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D \"DBG_BUILD=\\\"${DBG}\\\"\"" )

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")

    set (CMAKE_C_COMPILER /usr/bin/gcc)
    set (CMAKE_CXX_COMPILER /usr/bin/g++)
    set (CMAKE_CXX_STANDARD 17)
    set (CMAKE_CXX_STANDARD_REQUIRED ON)
    set (CMAKE_CXX_EXTENSIONS OFF)

    find_package( Threads )
else()
    message (STATUS "WINDOWS system detected. - NOT IMPLEMENTED FOR THIS PROJECT YET" )
endif()

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set( TOOLS_INCLUDED:BOOL ON )

set (EnetUtil_HEADERS         "${SampleRoot_DIR}/source/EnetUtil/include" )
set (Util_HEADERS             "${SampleRoot_DIR}/source/Util/include")
set (VideoPlugin_V4L2_HEADERS "${SampleRoot_DIR}/source/Video/include" )    ##########   ${OpenCV_INCLUDE_DIRS} )
set (Video_HEADERS            "${SampleRoot_DIR}/source/Video/include" ) ########   ${OpenCV_INCLUDE_DIRS} )

set( LoggerCpp_HEADERS        "${LoggerCppSource_DIR}/include" )
set( LoggerCpp_BASE           "${SampleRoot_DIR}/source/3rdparty/LoggerCpp" )
set( LoggerCpp_LIB            "${LoggerCpp_BASE}/build/libLoggerCpp.a")

set( JsonCpp_HEADERS          "${JsonCppSource_DIR}/include" )
set( JsonCpp_BASE             "${SampleRoot_DIR}/source/3rdparty/JsonCpp" )
set( JsonCpp_LIB              "${JsonCpp_BASE}/build/lib/libjsoncpp.a")

set( LoggerCpp_LIBTYPE "STATIC")
set( JsonCpp_LIBTYPE "SHARED")

