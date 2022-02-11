#!/bin/bash
#
#  Adapted from the original build.sh:
#
# Copyright (c) 2013-2018 Sébastien Rombauts (sebastien.rombauts@gmail.com)
#
# Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
# or copy at http://opensource.org/licenses/MIT)

srcdir=SRombauts-LoggerCpp-a0868a8

mkdir -p build
cd build
cmake ../${srcdir}
cmake --build .
# ctest .
