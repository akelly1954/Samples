#!/bin/bash
#

CmakeGenerator='Eclipse CDT4 - Unix Makefiles'

cleanall=${1:-"no"}

if [ $cleanall != "no" ]
then
    echo + "JsonCpp/localbuild.sh:  The build directory will be removed first"
    rm -rf build
fi

export basepath="`realpath ../../..`"
envpath="$basepath/source/shell_env/bash_env.sh"
if [ ! -s "$envpath" ]
then
    echo + "ERROR: Cannot find shell environment file $envpath. Aborting..."
    exit 1
fi

. "$envpath"

if [ ! -d "${JsonCppSource_DIR}" ]
then
    echo "ERROR: Could not find directory ${JsonCppSource_DIR}"
    exit 1
fi

mkdir -p build
cd build

CMAKE_VERBOSE_MAKEFILE:BOOL=ON

cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -G"$CmakeGenerator" -DCMAKE_ECLIPSE_VERSION="${CMAKE_ECLIPSE_VERSION}" ../${jsoncppsrcdirname}
if [ $? -ne 0 ]
then
    echo "ERROR: cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -G"$CmakeGenerator" -DCMAKE_ECLIPSE_VERSION="${CMAKE_ECLIPSE_VERSION}" ../${jsoncppsrcdirname} failed.  Aborting..."
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]
then
    echo "ERROR: cmake --build . failed.  Aborting..."
    exit 1
fi

cp -r ../JsonCpp-8190e06-2022-07-15/jsoncpp/include .
if [ $? -ne 0 ]
then
    echo "ERROR: copying include files failed.  Aborting..."
    exit 1
fi

exit 0
