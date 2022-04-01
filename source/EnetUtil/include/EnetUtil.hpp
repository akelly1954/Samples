#pragma once

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

#include <Utility.hpp>
#include <string>
#include <map>
#include <utility>
#include <MainLogger.hpp>

namespace EnetUtil
{
    // Used to get a list of IP addresses belonging to this system.
    // std::map.first is the string ip address.  The pair.first member
    // is the string interface name, and pair.second is the address family.
    // The first overload sets up a logger and calls the second overload.
    // Or, use your own.
    // Returns empty map if not found or any errors occurred.
    std::map<std::string, std::pair<std::string, std::string>> get_all_self_ip_addresses();
    std::map<std::string, std::pair<std::string, std::string>> get_all_self_ip_addresses(Log::Logger logger);

    // Used to get a single IP address belonging to this system
    // which is actually connected to the internet.
    // This is done by attempting a connection to '8.8.8.8' which
    // is google dns. The ip address parameter to the function is
    // the ip address to be used instead of 8.8.8.8
    // Returns empty string if not found or any errors occured.
    std::string get_primary_self_ip_address(Log::Logger logger,
                                               std::string useIpAddress = "8.8.8.8",
                                               uint16_t udpDnsPort = 53);

    // This is an overloaded version of the function for callers that do not
    // have a logger configured.
    std::string get_primary_self_ip_address( std::string useIpAddress = "8.8.8.8",
                                               uint16_t udpDnsPort = 53);

    // For any IP address belonging to this system which is actually connected
    // to the internet, get the MAC address for that interface. The goal is
    // to get the primary IP address' MAC address using the interface name for
    // that ip address.
    // Returns empty string if not found or any errors occured.
    std::string get_self_mac_address(Log::Logger logger, std::string interfaceName);

    // This is an overloaded version of the function for callers that do not
    // have a logger configured.
    std::string get_self_mac_address(std::string interfaceName);

} // namespace EnetUtil

