#!/bin/bash

export basepath=`realpath "$PWD/.."`
envpath="$basepath/source/shell_env/bash_env.sh"
if [ ! -s "$envpath" ]
then
    echo + "ERROR: Cannot find $envpath. Aborting..."
    exit 1
fi

. "$envpath"


if [ "$1" = "-h" -o "$1" = "--help" ]
then
    base_linux_build_Usage
    exit 0
fi

nobuild=""
if [ "$1" = "-n" -o "$1" = "--nobuild" ]
then
    nobuild="--nobuild"
fi

#####################################################
# Check LoggerCpp
#####################################################

if [ ! -d "${LoggerCppSource_DIR}" ]
then
    echo "ERROR: Could not find directory ${LoggerCppSource_DIR}"
    exit 1
fi

here="$PWD"
echo + At: $PWD

cd "$loggercppdir"
if [ $? -ne 0 ]
then
    echo "Could not change directory to $loggercppdir for localbuild.sh"
    exit 1
fi

# 3rdparty gets built regardless of whether --nobuild was invoked.
echo "Building 3rdparty package LoggerCpp"
#
bash localbuild.sh cleanall > lastbuild_log.txt 2>& 1
if [ $? -ne 0 ]
then
    echo "ERROR: build of $loggercppdir failed.  Aborting..."
    exit 1
fi
echo "SUCCESS: LoggerCpp build is done: log file is in $loggercppdir/lastbuild_log.txt."

cd "$here"
if [ $? -ne 0 ]
then
    echo "Could not change directory to $here"
    exit 1
fi

#####################################################
# Check JsonCpp
#####################################################

if [ ! -d "${JsonCppSource_DIR}" ]
then
    echo "ERROR: Could not find directory ${JsonCppSource_DIR}"
    exit 1
fi

here="$PWD"
echo + At: $PWD

cd "$jsoncppdir"
if [ $? -ne 0 ]
then
    echo "Could not change directory to $jsoncppdir for localbuild.sh"
    exit 1
fi

# 3rdparty gets built regardless of whether --nobuild was invoked.
echo "Building 3rdparty package JsonCpp"
#
bash localbuild.sh cleanall > lastbuild_log.txt 2>& 1
if [ $? -ne 0 ]
then
    echo "ERROR: build of $jsoncppdir failed.  Aborting..."
    exit 1
fi
echo "SUCCESS: JsonCpp build is done: log file is in $jsoncppdir/lastbuild_log.txt."

cd "$here"
if [ $? -ne 0 ]
then
    echo "Could not change directory to $here"
    exit 1
fi

dlist=""
lslist="*"
for dir in $lslist
do
    if [ -d $dir ]
    then
        if [ $dir = "build" -o $dir = "cmake" -o $dir = "shell_env" -o $dir = "3rdparty" ]
        then
            continue
        fi

        dlist="$dlist $dir"
    fi
done

srcbasepath="$PWD"

# All but the "build" have to exist, so this is ok
bldpath=`realpath ../build`

if [ -d "$bldpath" ]
then
    echo +++++++++++++++++++++++++++++++++++++++++++ 
    echo Removing $bldpath
    echo +++++++++++++++++++++++++++++++++++++++++++ 
    echo
    rm -rf "$bldpath"
fi

echo +++++++++++++++++++++++++++++++++++++++++++ 
echo Directory List: $dlist
echo +++++++++++++++++++++++++++++++++++++++++++ 

gflag="-g eclipsemake"

buildtype="Debug"
dflag=""
if [ "$buildtype" = "Debug" ]
then
    dflag="-d"
else
    dflag=""
fi

# In phase generate, only --nobuild option is built.
# Phase "build builds and installs everything.
for phase in "generate" "build"
do
    if [ "$phase" = "generate" -a "$nobuild" = "" ]
    then
        continue
    elif [ "$phase" = "build" -a "$nobuild" = "--nobuild" ]
    then
        continue
    fi

    for line in $dlist
    do
        fullpath="${srcbasepath}/${line}"
        dirname="$fullpath"

        if [ ! -r "$fullpath/$scriptname" ]
        then
            echo Skipping project directory \"$line\" 
            continue
        fi

        cd "$dirname"
        if [ $? -ne 0 ]
        then
            echo "Could not change directory to $dirname for $line"
            exit 1
        fi

        echo + at: $PWD
        echo + "LoggerCppSource_DIR = ${LoggerCppSource_DIR}"
        echo + "JsonCppSource_DIR = ${JsonCppSource_DIR}"
        echo + Running bash "$scriptname" -c $dflag gflag $nobuild 
        bash "$scriptname" -c $dflag $gflag $nobuild 
        if [ $? -ne 0 ]
        then
            echo BUILD FAILED
            exit 1
        fi
        echo
    done    # end of dlist for loop
done    # end of phase for loop

if [ "$nobuild" = "--nobuild" ]
then
    echo
    echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    echo + 
    echo + All CMake artifacts have been set up.  No-build option was specifed.
    echo + Exiting.
    echo + 
    echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    exit 0
fi

# Install for all projects
cd "$bldpath"
if [ $? -ne 0 ]
then
    echo "Could not change directory to $dirname for $bldpath"
    exit 1
fi

echo
echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo + At $PWD: cmake --build . --target install --config $buildtype
echo +
echo + NOTE: YOU MAY GET INSTALL ERRORS IF THERE IS NOTHING TO INSTALL IN THIS PROJECT.
echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

echo + "LoggerCppSource_DIR = ${LoggerCppSource_DIR}"
echo + "JsonCppSource_DIR = ${JsonCppSource_DIR}"
cmake --build . --target install --config $buildtype
ret=$?
if [ $ret -ne 0 ]
then
    echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    echo + At $PWD: ERROR:    Install Error
    echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    exit 1
fi

echo "Set LD_LIBRARY_PATH to where the all .so files are, as well as . and export it..." \
     > ${bldpath}/localrun/REMEMBER_TO_SET_LD_LIBRARY_PATH.txt
chmod 444 ${bldpath}/localrun/REMEMBER_TO_SET_LD_LIBRARY_PATH.txt

exit 0

