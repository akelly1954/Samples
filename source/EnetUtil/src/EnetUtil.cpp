#include "Utility.hpp"
#include "EnetUtil.hpp"
#include <iostream>
#include <errno.h>
//#include <sys/types.h>

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


/////////////////////////////////////////////////////////////////////////////////
// On the assumption that the process self mac address does not change during its
// lifetime...  This static is not visible outside the scope of this source file.
// There usually is more than one mac address.  
static std::vector<std::string> static_self_mac_addresses;

// This utility lock is static, used by get_self_mac_address below
// This static is not visible outside the scope of this source file.
static std::recursive_mutex mac_address_mutex;

// Returns empty vector if not found
std::vector<std::string> EnetUtil::get_all_self_mac_addresses()
{
    using namespace Util;

    if (static_self_mac_addresses.size() != 0)
    {
        // Once set, we don't allow the system's mac addresses to change for the lifetime of the process.
        std::cerr << "ERROR.  The MAC address object is not empty." << std::endl;
        return static_self_mac_addresses;
    }

    std::lock_guard<std::recursive_mutex> lock(mac_address_mutex);

    // Belt and suspenders (this is about the lock btw --
    // No need to lock/unlock if there were an error. So do it again once locked.
    if (static_self_mac_addresses.size() != 0)
    {
        // Once set, we don't allow the system's mac addresses to change for the lifetime of the process.
        std::cerr << "ERROR.  The MAC address object is not empty." << std::endl;
        return static_self_mac_addresses;
    }

    const int MIN_MAC_ADDR_LENGTH = 12;
    std::string macAddress;
    std::vector<std::string> macAddressVector;

    std::string command = "ip link show | awk '$0 ~ /ether/ {print $2}'";
    std::string tmpstr;
    char var[16 * 1024];      // This just moves the stack pointer. Size is not an issue here.

    auto fp = popen(command.c_str(), "r");

    if (fp == NULL)
    {
        std::cerr << "ERROR.  errno: " << Utility::get_errno_message(errno) << std::endl;
        if (macAddressVector.size() == 0)
        {
            return macAddressVector;   // it's empty
        }
    }
    else
    {
        while (fgets(var, sizeof (var) - 1, fp) != NULL)
        {
            tmpstr = const_cast<const char *>(var);
            tmpstr = Utility::trim(tmpstr, " \t\n\v\f\r");
            tmpstr = Utility::replace_all(tmpstr, ":", "");  // Remove all : chars - we'll add them back later on....

            if (tmpstr.length() != MIN_MAC_ADDR_LENGTH)
            {
                continue;
            }
            Utility::to_upper(tmpstr);
            macAddress = tmpstr.substr(0, 2) + ":" + tmpstr.substr(2, 2) + ":" + tmpstr.substr(4, 2) + ":"
                    + tmpstr.substr(6, 2) + ":" + tmpstr.substr(8, 2) + ":" + tmpstr.substr(10, 2);
            macAddressVector.push_back(macAddress);
        }
    }

    if (fp != NULL) pclose(fp);
    if (macAddressVector.size() == 0)
    {
        return macAddressVector;   // it's empty
    }

    static_self_mac_addresses = macAddressVector;
    return static_self_mac_addresses;
}

