#include <Utility.hpp>
#include <EnetUtil.hpp>
#include <MainLogger.hpp>
#include <map>
#include <utility>

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


int main(int argc, char *argv[])
{
    using namespace Util;

    std::string channelName = "MainEnetUtil_ipAddresses";

    Log::Config::Vector configList;
    MainLogger::initialize( configList,
                            channelName,
                            Log::Log::Level::eDebug,
                            MainLogger::enableConsole,
                            MainLogger::disableLogFile
                          );

    Log::Logger logger(channelName.c_str());
    std::map<std::string, std::pair<std::string, std::string>> ipAddrMap =
                                        EnetUtil::get_all_self_ip_addresses(logger);

    if (ipAddrMap.size() == 0)
    {
        logger.error() << "IP address map is empty.";
        return 1;
    }

    for (auto const& ipaddrmap: ipAddrMap)
    {
        logger.notice() << "IP: " << ipaddrmap.first <<
                           " Interface: " << ipaddrmap.second.first <<
                           " Family: " << ipaddrmap.second.second;
    }

    logger.notice() << "";   // empty line

    std::string self_ip_address = EnetUtil::get_primary_self_ip_address(logger);
    logger.notice() << "Primary IP address: " << self_ip_address;

    if (self_ip_address.empty())
    {
        logger.error() << "Cannot get self MAC address: Primary IP address is empty.";
        return 1;
    }

    std::string interface = ipAddrMap[self_ip_address].first;
    logger.notice() << "Primary interface = " << interface;

    std::string primaryMacAddress = EnetUtil::get_self_mac_address(logger, interface);
    if (primaryMacAddress.empty())
    {
        logger.error() << "Cannot get self MAC address: get_self_mac_address() returned empty string.";
        return 1;
    }

    logger.notice() << "Primary MAC address: " << primaryMacAddress;

    return 0;
}


