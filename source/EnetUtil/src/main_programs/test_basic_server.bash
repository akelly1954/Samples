#!/usr/bin/bash

# To set up this test:
#
# in ...Samples/build/localrun/tests:
#
# Prepare a bunch of input files to be transfered, of varying sizes - say seven or eight files.
# Call them inputXXX.data - where XXXX is different for each file. My smallest file was a few bytes,
# and my biggest one was 23Mb or so.  Protect them from being removed by accident (chmod 444 inp*.data)
#
# cd to ...Samples/build/localrun, and start the server (main_ntwk_basic_sock_server) on a different terminal.
#
# You can "tail -f" the log file in localrune:  tail -f main_basic_socket_server_log.txt
#
# cd to ...Samples/build/localrun/tests on yet another terminal, and run the tests:
#
#               bash ./test_basic_server.bash
#
# Be mindful of disk space on the system running the server.  You can consume gigabytes of space in 
# less than a minute.
#

localrun=`realpath ..`
export LD_LIBRARY_PATH=${localrun}:${LD_LIBRARY_PATH}

rm output_*.data

i=0
list1=`ls -1 inp*.data | head -4`
list2=`ls -1 inp*.data | tail -4`

while [ $i -lt 50 ]
do
    for file in $list1
    do
        ../main_client_for_basic_server -fn $file &
    done

    nc=`ps xa | grep main_client_for_basic_server | wc -l`
    echo $((nc-1)) clients are running concurrently

    for file in $list2
    do
        ../main_client_for_basic_server -fn $file &
    done

    nc=`ps xa | grep main_client_for_basic_server | wc -l`
    echo $((nc-1)) clients are running concurrently

    i=$((i+1))
    date

done

echo
echo Waiting for jobs to finish...
echo

wait

