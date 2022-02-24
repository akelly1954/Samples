#!/bin/bash
#
#  Adapted from the original build.sh:
#
# Copyright (c) 2013-2018 Sébastien Rombauts (sebastien.rombauts@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)

CmakeGenerator='Eclipse CDT4 - Unix Makefiles'

export basepath="`realpath ../../..`"
envpath="$basepath/source/shell_env/bash_env.sh"
if [ ! -s "$envpath" ]
then
    echo + "ERROR: Cannot find shell environment file $envpath. Aborting..."
    exit 1
fi

. "$envpath"

if [ ! -d "${LoggerCppSource_DIR}" ]
then
    echo "ERROR: Could not find directory ${LoggerCppSource_DIR}"
    exit 1
fi

# this prevents doxygen from running every time for some reason...
export TRAVIS=1

mkdir -p build
cd build

cmake -G"$CmakeGenerator" -DCMAKE_ECLIPSE_VERSION="${CMAKE_ECLIPSE_VERSION}" ../${loggercppsrcdirname}
if [ $? -ne 0 ]
then
    echo "ERROR: cmake -G"$CmakeGenerator" -DCMAKE_ECLIPSE_VERSION="${CMAKE_ECLIPSE_VERSION}" ../${loggercppsrcdirname} failed.  Aborting..."
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]
then
    echo "ERROR: cmake --build failed.  Aborting..."
    exit 1
fi

exit 0
