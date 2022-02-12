#!/bin/bash
#
#  Adapted from the original build.sh:
#
# Copyright (c) 2013-2018 Sébastien Rombauts (sebastien.rombauts@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)

srcdir=SRombauts-LoggerCpp-a0868a8

# this prevents doxygen from running every time for some reason...
export TRAVIS=1

mkdir -p build
cd build

cmake ../${srcdir}
if [ $? -ne 0 ]
then
    echo "ERROR: cmake ../${srcdir} failed.  Aborting..."
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]
then
    echo "ERROR: cmake --build failed.  Aborting..."
    "Could not change directory to $here/$loggercppdir for localbuild.sh"
    exit 1
fi

exit 0
