#include "commandline.hpp"
#include <iostream>
#include <iomanip>      // std::setprecision

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

static int intParam1 = -1;
static std::string stringParam1;
static int intParam2 = -1;
static int intParam3 = -1;
static double doubleParam1 = -1;

// You write this
void Usage(std::string command)
{
    std::cout << "Usage:    " << command << " --help" << std::endl;
    std::cout << "Or:       " << command << " -p doubleParam1 -w intParam2 -h intParam3 [-n intParam1 | -f file_to_stream_from]" << std::endl;
    std::cout << "If intParam1 and stringParam1 are both omitted, intParam1 defaults to 0." << std::endl;
}

// You write this
bool parse(int argc, char *argv[])
{
    using namespace Util;
    const std::map<std::string,std::string> cmdmap = getCLMap(argc, argv);

    getArg(cmdmap, "-n", intParam1);
    getArg(cmdmap, "-f", stringParam1);
    getArg(cmdmap, "-p", doubleParam1);
    getArg(cmdmap, "-w", intParam2);
    getArg(cmdmap, "-h", intParam3);

    if (stringParam1.length() == 0 && intParam1 == -1) intParam1 = 0;

    // Ensure that at least one of these parameters is given a value
    if (doubleParam1 == -1 && intParam2 == -1 && intParam3 == -1) return false;

    return true;
}

int main(int argc, char *argv[])
{
    using namespace std;

    std::string argv0 = const_cast<const char *>(argv[0]);

    // not much parsing needed for this...
    if (argc > 1 && std::string(const_cast<const char *>(argv[1])) == "--help")
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

    std::cout << "Parse returned " << parseres << std::endl;
    std::cout << "stringParam1 = \"" << stringParam1 << "\"" << std::endl;
    std::cout << "intParam1 = " << intParam1 << std::endl;
    std::cout << "intParam2 = " << intParam2 << std::endl;
    std::cout << "intParam3 = " << intParam3 << std::endl;

    std::cout.unsetf ( std::ios::floatfield );
    std::cout << std::setprecision(8);
    std::cout.setf( std::ios::fixed, std:: ios::floatfield );
    std::cout << "doubleParam1 = " << doubleParam1 << std::endl;
    std::cout << std::endl;

    return 0;
}

#ifdef ExampleCommandLine

# Using bash shell with LD_LIBRARY_PATH undefined:
$
$ cd build/localrun
$ LD_LIBRARY_PATH=".:" ./main_commandline -p 299883.90087546 -w 03498504 -h 67878766 -n -445 \
                       -f "this is the way for all good men to come to the aid of their country"
Parse returned 1
stringParam1 = "this is the way for all good men to come to the aid of their country"
intParam1 = -445
intParam2 = 3498504
intParam3 = 67878766
doubleParam1 = 299883.90087546
$

#endif // ExampleCommandLine




