
# Included from various ...linux-build.bash scripts

export basepath="${basepath:=UNDEFINED_BASE_PATH}"
scriptname="linux-build.bash"
loggercppdir="${basepath}/source/3rdparty/LoggerCpp"
loggercppsrcdirname="SRombauts-LoggerCpp-a0868a8"
LoggerCppSource_DIR="$loggercppdir/$loggercppsrcdirname"
SampleRoot_DIR="$basepath"

CMAKE_ECLIPSE_VERSION="4.22"

function base_linux_build_Usage
{
    scriptname=`basename ${0:-"script"}`

    printf "\n\nFUTURE IMPLEMENTATION: At this point, only two of these flags (--help and --nobuild)\n" >& 2
    printf "                       are implemented. 3rdparty objects always get built.\n" >& 2
    printf "\n\n" >& 2
    printf "Usage:   $scriptname [ flags ]\n" >& 2
    printf "Where flags are all optional, and may include the following:\n" >& 2
    printf "\n" >& 2
    printf "    -h (or --help):         Emit this output and quit\n" >& 2
    printf "    -d (or --debug):        Build in Debug mode (c++ -g flag, no optimization, and no stripping of symbols)\n" >& 2
    printf "    -n (or --nobuild):      Perform only "clean" and "cmake" runs, but supress the build itself\n" >& 2
    printf "    -g (or --generator):    Use a non-default cmake generator.  We use abbreviated names to avoid\n" >& 2
    printf "                            white space confusion. Current available generators are:\n\n" >& 2
    printf "                                  \"unixmake\" for -G \"Unix Makefiles\"\n\n" >& 2
    printf "              *** default ***     \"eclipsemake\" for -G \"Eclipse CDT4 - Unix Makefiles\"\n\n" >& 2
    printf "                                  \"ninja\" for -G \"Ninja\"\n\n" >& 2
    printf "                                  \"eclipseninja\" for -G \"Eclipse CDT4 - Ninja\"\n\n" >& 2
    printf "                            This list can expand as needed.  See \"cmake --help\" for more details\n" >& 2
    printf "\n\n" >& 2
    printf "Below is the current list of valid generators for your platform:\n" >& 2
    printf "\n" >& 2
    cmake --help | sed -e '1,/^The following generators are available/d' | \
                   sed -e 's,^\* ,  ,' >& 2
}

##########################################################################
#
# Usage message for the multiple linux-build.bash scripts
#
################################
function linux_build_Usage
{
    scriptname=`basename ${0:-"script"}`

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
    printf "                                  \"unixmake\" for -G \"Unix Makefiles\"\n\n" >& 2
    printf "              *** default ***     \"eclipsemake\" for -G \"Eclipse CDT4 - Unix Makefiles\"\n\n" >& 2
    printf "                                  \"ninja\" for -G \"Ninja\"\n\n" >& 2
    printf "                                  \"eclipseninja\" for -G \"Eclipse CDT4 - Ninja\"\n\n" >& 2
    printf "                            This list can expand as needed.  See \"cmake --help\" for more details:\n" >& 2
    printf "\n\n" >& 2
    printf "Below is the current list of valid generators for your platform:\n" >& 2
    printf "\n" >& 2
    cmake --help | sed -e '1,/^The following generators are available/d' | \
                   sed -e 's,^\* ,  ,' >& 2
}


