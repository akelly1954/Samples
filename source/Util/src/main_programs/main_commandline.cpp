#include "commandline.hpp"
#include <iostream>

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
    std::cout << "stringParam1 = " << stringParam1 << std::endl;
    std::cout << "intParam1 = " << intParam1 << std::endl;
    std::cout << "intParam2 = " << intParam2 << std::endl;
    std::cout << "intParam3 = " << intParam3 << std::endl;
    std::cout << "doubleParam1 = " << doubleParam1 << std::endl;
    std::cout << std::endl;

    return 0;
}
