#include "Utility.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <cstdarg>
#include <cstring>
#include <chrono>
#include <sstream>
#include <ctime>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <locale>

using namespace std;
using namespace Util;

long Utility::getUTCTimeAsLong()
{
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return ms.count();
}

string Utility::getUTCTimeAsString(const char *format)
{
    std::time_t now = std::time(0);
    std::tm *now_tm = std::gmtime(&now);
    char buf[128]; // More than we need
    std::strftime(buf, 128, format, now_tm);
    return static_cast<const char *>(buf);
}

// Get a random int from within a range starting at "low", and
// "low" + "range".  If no "low" number is specified, 0 is used.
// For example, get_rand(10,3) gets you a random number between
// 3 and 12 (inclusive).
int Utility::get_rand(int range, int low)
{
    static bool srand_called = false;
    if (srand_called)
    {
        return (rand() % range) + low;
    }

    srand(time(NULL));
    srand_called = true;
    return (rand() % range) + low;
}

