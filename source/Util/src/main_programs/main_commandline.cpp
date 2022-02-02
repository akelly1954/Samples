#include "commandline.hpp"
#include <iostream>
#include <iomanip>      // std::setprecision
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2022 Andrew Kelly
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////

//
// At the end of the file there is an ifdef'ed section showing how to run
// this program from the command line.
//

static int intParam1 = -1;
static long longParam1 = -1;
static long long longlongParam1 = -1;
static float floatParam1 = -1;
static double doubleParam1 = -1;
static long double longdoubleParam1 = -1;
static std::string stringParam1;

// You write this
void Usage(std::string command)
{
    std::cout << "Usage:    " << command << " --help (or -h or help)" << std::endl;
    std::cout << "Or:       " << command << "\n"
    			 "                  -ii IntParam \n" <<
				 "                  -i1 LongParam \n" <<
				 "                  -i2 LongLongParam \n" <<
				 "                  -ff FloatParam \n" <<
				 "                  -fd DoubleParam \n" <<
				 "                  -fl LongDoubleParam \n" <<
				 "                  -st StringParam \n" <<
				 "(all params are optional, although that's not useful...)" <<
				 std::endl;
}

// You write this
bool parse(int argc, char *argv[])
{
    using namespace Util;
    const std::map<std::string,std::string> cmdmap = getCLMap(argc, argv);
    std::map<std::string,bool> specified;

    specified["-ii"] = getArg(cmdmap, "-ii", intParam1);
	specified["-i1"] = getArg(cmdmap, "-i1", longParam1);
    specified["-i2"] = getArg(cmdmap, "-i2", longlongParam1);
	specified["-st"] = getArg(cmdmap, "-st", stringParam1);
	specified["-ff"] = getArg(cmdmap, "-ff", floatParam1);
	specified["-fd"] = getArg(cmdmap, "-fd", doubleParam1);
	specified["-fl"] = getArg(cmdmap, "-fl", longdoubleParam1);

#ifdef FOR_DEBUG
	for (auto it = specified.begin(); it != specified.end(); ++it)
	{
		std::cout << it->first.c_str() << " = " << (it->second? "true" : "false") << std::endl;
	}
#endif // FOR_DEBUG

    // If any of the parameters were specified, it's ok
    bool ret = false;
    std::for_each(specified.begin(), specified.end(), [&ret](auto member) { if (member.second) { ret = true; }});
    return ret;
}

int main(int argc, char *argv[])
{
    using namespace std;

    std::string argv0 = const_cast<const char *>(argv[0]);

    // If no parameters were supplied, or help was requested:
    if (argc <= 1 || (argc > 1 &&
    		(std::string(const_cast<const char *>(argv[1])) == "--help" ||
  	    	 std::string(const_cast<const char *>(argv[1])) == "-h" ||
  	    	 std::string(const_cast<const char *>(argv[1])) == "help")
		)
    )
    {
        Usage(argv[0]);
        return 0;
    }

    bool parseres = parse(argc, argv);
    if (! parseres)
    {
        Usage(argv0);
        return 1;
    }

    std::cout << "\n";
    std::cout << "int intParam1 = " << intParam1 << std::endl;
    std::cout << "long longParam1 = " << longParam1 << std::endl;
    std::cout << "long long longlongParam1 = " << longlongParam1 << std::endl;

    std::cout.unsetf ( std::ios::floatfield );
    std::cout << std::setprecision(4);
    std::cout.setf( std::ios::fixed, std:: ios::floatfield );
	std::cout << "float floatParam1 = " << floatParam1 << std::endl;

	std::cout.unsetf ( std::ios::floatfield );
    std::cout << std::setprecision(8);
    std::cout.setf( std::ios::fixed, std:: ios::floatfield );
	std::cout << "double doubleParam1 = " << doubleParam1 << std::endl;

    std::cout.unsetf ( std::ios::floatfield );
    std::cout << std::setprecision(10);
    std::cout.setf( std::ios::fixed, std:: ios::floatfield );
	std::cout << "long double longdoubleParam1 = " << longdoubleParam1 << std::endl;

    std::cout << "std::string stringParam1 = \"" << stringParam1 << "\"" << std::endl;

    std::cout << std::endl;

    return 0;
}

#ifdef ExampleCommandLine

# Using bash shell with LD_LIBRARY_PATH undefined:
$
$ cd build/localrun
$ LD_LIBRARY_PATH=".:" ./main_commandline -ff 883.987546 -fd 94596.049853 \
                -fl 2736572527.8273666 -i1 -03498504 -i2 67878766 -ii -445  \
                -st "Last night I dreamt I went to Manderley again."

int intParam1 = -445
long longParam1 = -3498504
long long longlongParam1 = 67878766
float floatParam1 = 883.9875
double doubleParam1 = 94596.04985300
long double longdoubleParam1 = 2736572527.8273666000
std::string stringParam1 = "Last night I dreamt I went to Manderley again."

$

#endif // ExampleCommandLine




