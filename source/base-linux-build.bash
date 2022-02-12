#!/bin/bash

scriptname="linux-build.bash"

# Build 3rdpartyLoggerCpp if needed

# TODO: have to push this down to the various linux-build.bash scripts
loggercppdir="3rdparty/LoggerCpp"
loggercppsrcdir="SRombauts-LoggerCpp-a0868a8"
export LoggerCppSource_DIR="`realpath $loggercppdir/$loggercppsrcdir`"

if [ ! -d "${LoggerCppSource_DIR}" ]
then
    echo "ERROR: Could not find directory ${LoggerCppSource_DIR}"
    exit 1
fi

here="$PWD"

cd "$here/$loggercppdir"
if [ $? -ne 0 ]
then
    echo "Could not change directory to $here/$loggercppdir for localbuild.sh"
    exit 1
fi

bash localbuild.sh
if [ $? -ne 0 ]
then
    echo "ERROR: build of $loggercppdir failed.  Aborting..."
    exit 1
fi

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
        if [ $dir = "build" -o $dir = "cmake" ]
        then
            continue
        fi

        dlist="$dlist $dir"
    fi
done

basepath="`pwd`"

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
echo Directory Project List: $dlist
echo +++++++++++++++++++++++++++++++++++++++++++ 

buildtype="Debug"

if [ "$buildtype" = "Debug" ]
then
    gflag="-g"
else
    gflag=""
fi

for line in $dlist
do
    fullpath="${basepath}/${line}"
    dirname="$fullpath"

    if [ ! -r "$fullpath/$scriptname" ]
    then
        echo Skipping project build script $line
        continue
    fi

    cd "$dirname"
    if [ $? -ne 0 ]
    then
        echo "Could not change directory to $dirname for $line"
        exit 1
    fi

    echo + at: `pwd`
    echo + "LoggerCppSource_DIR = ${LoggerCppSource_DIR}"
    echo + Running bash "$scriptname" -c -d ${gflag} eclipsemake
    bash "$scriptname" -c -d ${gflag} eclipsemake
    if [ $? -ne 0 ]
    then
        echo BUILD FAILED
        exit 1
    fi
    echo
done

# Install for all projects
cd "$bldpath"
if [ $? -ne 0 ]
then
    echo "Could not change directory to $dirname for $bldpath"
    exit 1
fi

echo
echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo + At `pwd`: cmake --build . --target install --config $buildtype
echo +
echo + NOTE: YOU MAY GET INSTALL ERRORS IF THERE IS NOTHING TO INSTALL IN THIS PROJECT.
echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

echo + "LoggerCppSource_DIR = ${LoggerCppSource_DIR}"
cmake --build . --target install --config $buildtype
ret=$?
if [ $ret -ne 0 ]
then
    echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    echo + At `pwd`: ERROR:    Install Error
    echo +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    exit 1
fi

echo "Set LD_LIBRARY_PATH to where the all .so files are, as well as . and export it..." \
     > ${bldpath}/localrun/REMEMBER_TO_SET_LD_LIBRARY_PATH.txt

exit 0

