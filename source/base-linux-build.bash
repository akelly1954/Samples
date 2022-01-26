#!/bin/bash

scriptname="linux-build.bash"

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

for line in $dlist
do

    fullpath="`pwd`/${line}"

    if [ ! -r "$fullpath/$scriptname" ]
    then
        # echo Skipping $line
        continue
    fi

    filename="$scriptname"
    dirname="$fullpath"

    cd "$dirname"
    if [ $? -ne 0 ]
    then
        echo "Could not change directory to $dirname for $fullpath"
        exit 1
    fi

    echo + at: `pwd`
    echo + Running bash "$scriptname" -c -d -g eclipsemake
    bash "$scriptname" -c -d -g eclipsemake
    if [ $? -ne 0 ]
    then
        echo BUILD FAILED
        exit 1
    fi
    echo
done
exit 0

