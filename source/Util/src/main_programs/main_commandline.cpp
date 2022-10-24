#include <commandline.hpp>
#include <Utility.hpp>
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
// TODO: enhance this main program to account for changes in the command line structure.
//

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
std::string Usage(std::string command)
{
    std::stringstream sstrm;
    sstrm << "\n";
    sstrm << "Usage:    " << command << " --help (or -h or help)" << std::endl;
    sstrm << "Or:       " << command << "\n"
             "                  -ii IntParam \n" <<
             "                  -i1 LongParam \n" <<
             "                  -i2 LongLongParam \n" <<
             "                  -ff FloatParam \n" <<
             "                  -fd DoubleParam \n" <<
             "                  -fl LongDoubleParam \n" <<
             "                  -st StringParam \n" <<
             "\n";
    return sstrm.str();
}

// You write this
bool parse(int argc, const char *argv[])
{
    using namespace Util;

    std::string argv0 = argv[0];

    const std::vector<std::string> allowedFlags ={ "-ii", "-i1", "-i2", "-ff", "-fd", "-fl", "-st" };
    CommandLine cmdline(argc, argv, allowedFlags /*, Usage(argv0) */ );

    if(cmdline.isError())
    {
        std::cout << "\n" << argv0 << ": " << cmdline.getErrorString() << std::endl;
        return false;
    }

    if(cmdline.isHelp())
    {
        // No error, just help
        if (argc > 2)
        {
            std::cout << "\nWARNING: using the --help flag cancels all other flags and parameters.  Exiting...\n" << std::endl;
        }
        return false;
    }

    std::map<std::string,bool> specified;

    specified["-ii"] = cmdline.get_template_arg("-ii", intParam1);
    specified["-i1"] = cmdline.get_template_arg("-i1", longParam1);
    specified["-i2"] = cmdline.get_template_arg("-i2", longlongParam1);
    specified["-st"] = cmdline.get_template_arg("-st", stringParam1);
    specified["-ff"] = cmdline.get_template_arg("-ff", floatParam1);
    specified["-fd"] = cmdline.get_template_arg("-fd", doubleParam1);
    specified["-fl"] = cmdline.get_template_arg("-fl", longdoubleParam1);

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

int main(int argc, const char *argv[])
{
    std::string argv0 = argv[0];

    if (! parse(argc, argv))
    {
        std::stringstream mstr;
        std::cout << Usage(argv0) << std::endl;
        return EXIT_FAILURE;
    }

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

    return EXIT_FAILURE;
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

$ # The above seven lines are the output
$

#endif // ExampleCommandLine




