#
# Need CMAKE 3.10 and above to use --std=c++17
# On entry, relies on ${SampleRoot_DIR} to be set to the path for the repository root.
#
cmake_minimum_required(VERSION 3.10)

include (${SampleRoot_DIR}/source/cmake/tools.cmake)

if (NOT WIN32)
    # set (QtSample_LIB "${SampleRoot_DIR}/build/QtSample/libQtSample.so")
    set (QtSample_LIB "")

    set (QtSample_LIBTYPE "SHARED")

    find_package( Threads )
else()
    message (STATUS "WINDOWS system detected. - NOT IMPLEMENTED FOR THIS PROJECT YET" )
endif()

set (QtSample "QtSample${DBG}")
set (QtSample_HEADERS "${SampleRoot_DIR}/source/QtSample/include")

