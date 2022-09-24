#!/bin/bash 

export PATH=.:$PATH
export LD_LIBRARY_PATH=".:${LD_LIBRARY_PATH}"

$*
