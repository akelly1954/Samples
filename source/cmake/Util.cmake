#
# Need CMAKE 3.10 and above to use --std=c++17
# On entry, relies on ${SampleRoot_DIR} to be set to the path for the repository root.
#
cmake_minimum_required(VERSION 3.10)

set (CMAKE_VERBOSE_MAKEFILE ON)

if (NOT WIN32)
    message (STATUS "Linux system detected." )

    set (DBG "")
    set( LINKOPTIONS "stdc++fs" )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D \"DBG_BUILD=\\\"${DBG}\\\"\"" )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -ldl")

    set (CMAKE_CXX_COMPILER /usr/bin/g++)
    set (CMAKE_CXX_STANDARD 17)
    set (CMAKE_CXX_STANDARD_REQUIRED ON)
    set (CMAKE_CXX_EXTENSIONS OFF)

    set (Util_LIB "${SampleRoot_DIR}/build/Util/libUtil.so")
    set (LIBTYPE "SHARED")

    find_package( Threads )
else()
    message (STATUS "WINDOWS system detected. - NOT IMPLEMENTED FOR THIS PROJECT YET" )
endif()

set (Util "Util${DBG}")
set (Util_HEADERS "${SampleRoot_DIR}/source/Util/include")

