#!/bin/bash

SampleRoot_DIR="`realpath ../..`"

CMAKE_ECLIPSE_VERSION="4.21"

bldpath="../../build"
rootsrc="`realpath ..`"

# Make sure the directories are there, or realpath will fail.
mkdir -p $bldpath
bldpath="`realpath $bldpath`"

export CXX="/usr/bin/g++"                   # This overcomes a cmake issue

PATH=.:~/bin:/sbin:/usr/sbin:$PATH

##########################################################################
#
# Usage:
#
################################
function Usage
{
    scriptname=`basename ${0:-"sript"}`

    printf "\n" >& 2
    printf "Usage:   $scriptname [ flags ]\n" >& 2
    printf "Where flags are all optional, and may include the following:\n" >& 2
    printf "\n" >& 2
    printf "    -h (or --help):         Emit this output and quit\n" >& 2
    printf "    -c (or --clean):        Clean the build directory before running the build\n" >& 2
    printf "    -d (or --debug):        Build in Debug mode (c++ -g flag, no optimization, and no stripping of symbols)\n" >& 2
    printf "    -n (or --nobuild):      Perform only "clean" and "cmake" runs, but supress the build itself\n" >& 2
    printf "    -g (or --generator):    Use a non-default cmake generator.  We use abbreviated names to avoid\n" >& 2
    printf "                            white space confusion. Current available generators are:\n\n" >& 2
    printf "                                  \"unixmake\" (the default) for -G \"Unix Makefiles\"\n\n" >& 2
    printf "                                  \"eclipsemake\" for -G \"Eclipse CDT4 - Unix Makefiles\"\n\n" >& 2
    printf "                                  \"ninja\" for -G \"Ninja\"\n\n" >& 2
    printf "                                  \"eclipseninja\" for -G \"Eclipse CDT4 - Ninja\"\n\n" >& 2
    printf "                            This list can expand as needed.  See \"cmake --help\" for more details:\n" >& 2
    cmake --help | sed '1,/^Generators/d' >& 2
}

##########################################################################
#
# Parse command line parameters
#
################################

# set an initial value for the flag
ARG_HELP="false"
ARG_DEBUG="false"
ARG_CLEAN="false"
ARG_CMAKE_GENERATOR="default"
ARG_NOBUILD="false"

# read the options
TEMP=`getopt -o "hcdng:" --long "help,clean,debug,nobuild,generator:" -n "$0" -- $@`

set -- $TEMP

# extract options and their arguments into variables.
while true
do
    case $1 in
    -h|--help)
        ARG_HELP="true"
        shift
        ;;
    -c|--clean)
        ARG_CLEAN=true
        shift
        ;;
    -d|--debug)
        ARG_DEBUG="true"
        shift
        ;;
    -n|--nobuild)
        ARG_NOBUILD="true"
        shift
        ;;
    -g|--generator)
        # The list of generators below can be expanded if needed.  See "cmake --help".

        #####################################
        # Makefile (default)
        ######################
        if [ "$2" = "'unixmake'" ]
        then
            # This is the default
            ARG_CMAKE_GENERATOR="Unix Makefiles"

        #####################################
        # Eclipse CDT4 - Unix Makefiles
        ######################
        elif [ "$2" = "'eclipsemake'" ]
        then
            ARG_CMAKE_GENERATOR="Eclipse CDT4 - Unix Makefiles"

        #####################################
        # Ninja
        ######################
        elif [ "$2" = "'ninja'" ]
        then
            ARG_CMAKE_GENERATOR="ninja"

        #####################################
        # Eclipse CDT4 - Ninja
        ######################
        elif [ "$2" = "'eclipseninja'" ]
        then
            ARG_CMAKE_GENERATOR="Eclipse CDT4 - Ninja"

        #####################################
        # error
        ######################
        else
            printf "Error: No such cmake -G generator: $2. \n"
            printf "Current valid generators are \"unixmake\" (the default) or \"eclipsemake\".\n"
            printf "(This list can be expanded in $0)\n"
            exit 1
        fi
        shift 2
        ;;
    --)
        shift
        break
        ;;
    *)
        Usage
        exit 1
        ;;
    esac
done

##########################################################################
#
# Help
#
################################
if [ "$ARG_HELP" = "true" ]
then
    Usage
    exit 0
fi

##########################################################################
#
# Find out the number of cpu's for parallel compiles
#
################################
#  CPU(s):                8
cpus="`lscpu | grep '^CPU(s):' | sed 's/.*: *//'`"

##########################################################################
#
# Set up cmake options
#
################################
if [ "$ARG_DEBUG" = "true" ]
then
    CMAKE_SET_D="CMAKE_BUILD_TYPE=Debug"
else
    CMAKE_SET_D="CMAKE_BUILD_TYPE=Release"
fi
unset ARG_DEBUG

if [ "${ARG_CMAKE_GENERATOR}" != "default" ]
then
    CMAKE_SET_G="${ARG_CMAKE_GENERATOR}"
else
    CMAKE_SET_G="Unix Makefiles"
fi
unset ARG_CMAKE_GENERATOR

if [ "$ARG_CLEAN" = "true" ];
then
    # Notice that this will not remove .cproject or .project
    for dir in ${bldpath}/include ${bldpath}/lib ${bldpath}/localrun ${bldpath}/bin
    do
        echo "Removing the contents of $dir if it exists"
        rm -rf ${dir}/* > /dev/null 2>& 1
    done
fi

##########################################################################
#
# Configure and run cmake on the build directory
#
################################
mkdir -p "$bldpath/localrun"
ret=$?

if [ $ret -ne 0 ]
then
    echo "Cannot create build directory $bldpath/localrun."
    echo "Exiting..."
    exit 1
fi

# All the src/build directories
dirs[1]="${rootsrc} ${bldpath}"

retval=0
for dir in "${dirs[1]}"
do
    set -- $dir
    src=$1
    bld=$2

    cd $bld
    ret=$?
    if [ $ret -ne 0 ]
    then
        echo "Cannot chdir to build directory ${bld}."
        echo "Exiting..."
        exit 1
    fi

    echo
    echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    echo + At ${bld}: creating cmake artifacts for 
    echo + ${src}
    echo +
    echo + cmake -G"\"${CMAKE_SET_G}\"" \\
    echo +       -D"\"${CMAKE_SET_D}\"" \\
    echo +       -DOpenCV_DIR:PATH="\"${OpenCV_DIR}\"" \\
    echo +       -DCMAKE_INSTALL_PREFIX:PATH="\"${bldpath}\"" \\
    echo +       -DSampleRoot_DIR:PATH="\"${SampleRoot_DIR}\"" \\
    echo +       -DCMAKE_ECLIPSE_VERSION="\"${CMAKE_ECLIPSE_VERSION}\"" \\
    echo +       $src
    echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    #####################################################
    # These two options are currently set for development.  
    # In production, real paths need to be set instead.
    #
    # -DCMAKE_SKIP_RPATH=ON
    # -DCMAKE_SKIP_INSTALL_RPATH=ON
    #################

    cmake -G"${CMAKE_SET_G}" \
          -D"${CMAKE_SET_D}" \
          -DCMAKE_SKIP_RPATH=ON \
          -DCMAKE_SKIP_INSTALL_RPATH=ON \
          -DCMAKE_INSTALL_PREFIX:PATH="${bldpath}" \
          -DOpenCV_DIR:PATH="${OpenCV_DIR}" \
          -DSampleRoot_DIR:PATH="${SampleRoot_DIR}" \
          -DCMAKE_ECLIPSE_VERSION="${CMAKE_ECLIPSE_VERSION}" \
          $src
    ret=$?

    if [ $ret -ne 0 ]
    then
        echo
        echo "ERROR:  Cmake of ${src} failed. Exiting..."
        exit 1
    fi

    echo
    echo NOTE: All build artifacts for ${src} can be found under: 
    echo
    echo "     " ${bld}
    echo

    ret=0
    if [ "$ARG_NOBUILD" = "true" ]
    then
        echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        echo "Build of sources not run (--nobuild)"
        echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    else
        echo
        echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        echo + At ${bld}: cmake --build . --config $btype
        echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        cmake --build . --config $btype
        ret=$?
        if [ $ret -ne 0 ]
        then
            echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            echo + At ${bld}: ERROR:    Build Error
            echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            exit 1
        fi

        echo
        echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        echo + At ${bld}: cmake --build . --target install --config $btype
        echo +
        echo + NOTE: YOU MAY GET INSTALL ERRORS IF THERE IS NOTHING TO INSTALL IN THIS PROJECT.
        echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        cmake --build . --target install --config $btype
        ret=$?
        if [ $ret -ne 0 ]
        then
            echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            echo + At ${bld}: ERROR:    Install Error
            echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        fi
        echo "Set LD_LIBRARY_PATH to where the all .so files are, as well as . and export it..." \
             > ${bldpath}/localrun/REMEMBER_TO_SET_LD_LIBRARY_PATH.txt
    fi
done

