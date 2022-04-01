
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

// The sources here are based on postgs from stackoverflow.com, as well as the example
// shown in the getifaddrs(3) man page. 

#include <Utility.hpp>
#include <EnetUtil.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <string.h>
#include <errno.h>
// #include <stdio.h>  //printf
#include <MainLogger.hpp>

// Overload for callers with no logger set up.
std::map<std::string, std::pair<std::string, std::string>>
    EnetUtil::get_all_self_ip_addresses()
{
    using namespace Util;

    std::string channelName = "EnetUtil_ipAddresses";

    Log::Config::Vector configList;
    MainLogger::initialize( configList,
                            channelName,
                            Log::Log::Level::eDebug,
                            MainLogger::enableConsole,
                            MainLogger::disableLogFile
                          );

    Log::Logger logger(channelName.c_str());
    return get_all_self_ip_addresses(logger);
}

// Used to get a list of IP addresses belonging to this system.
// Returns empty map if not found or any errors occured.
std::map<std::string, std::pair<std::string, std::string>>
    EnetUtil::get_all_self_ip_addresses(Log::Logger logger)
{
    std::map<std::string, std::pair<std::string, std::string>> emptyMap;
    std::map<std::string, std::pair<std::string, std::string>> resultMap;

    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    if (::getifaddrs(&ifAddrStruct) != 0)
    {
        logger.notice() << "getifaddrs() failed: " << Util::Utility::get_errno_message(errno);
        return emptyMap;
    }

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            // This is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;

            char addressBuffer[INET_ADDRSTRLEN];
            if (::inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN) == NULL)
            {
                logger.notice() << "inet_ntop() failed (AF_INET): " << Util::Utility::get_errno_message(errno);
                return emptyMap;
            }

            resultMap[addressBuffer] =
                    std::pair<std::string, std::string>(const_cast<const char *>(ifa->ifa_name),"AF_INET");
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            // This is a valid IP6 Address
            tmpAddrPtr= &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;

            char addressBuffer[INET6_ADDRSTRLEN];
            if (::inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN) == NULL)
            {
                logger.notice() << "inet_ntop() failed (AF_INET6): " << Util::Utility::get_errno_message(errno);
                return emptyMap;
            }
            resultMap[addressBuffer] =
                    std::pair<std::string, std::string>(const_cast<const char *>(ifa->ifa_name),"AF_INET6");
        }
    }
    if (ifAddrStruct!=NULL) ::freeifaddrs(ifAddrStruct);

    return resultMap;
}

// This is an overloaded version of the function for callers that do not
// have a logger configured.
std::string EnetUtil::get_primary_self_ip_address( std::string useIpAddress, uint16_t udpDnsPort)
{
    using namespace Util;

    std::string channelName = "self_ip_address";

    Log::Config::Vector configList;
    MainLogger::initialize( configList,
                            channelName,
                            Log::Log::Level::eDebug,
                            MainLogger::enableConsole,
                            MainLogger::disableLogFile
                          );

    Log::Logger logger(channelName.c_str());

    return get_primary_self_ip_address(logger, useIpAddress, udpDnsPort);
}

// Used to get a single IP address belonging to this system
// which is actually connected to the internet.
// This is done by attempting a connection to '8.8.8.8' which
// is google dns. The ip address parameter to the function is
// the ip address to be used instead of 8.8.8.8
// Returns empty string if not found or any errors occured.
std::string EnetUtil::get_primary_self_ip_address(  Log::Logger logger,
                                                    std::string useIpAddress,
                                                    uint16_t udpDnsPort)
{
    std::string errorString;
    std::string resultString;

    int errnocopy = 0;
    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    errnocopy = errno;
    if (sock == -1)
    {
        logger.notice() << "::socket() failed: " << Util::Utility::get_errno_message(errnocopy);
        return errorString;
    }

    struct sockaddr_in serv;
    ::memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(useIpAddress.c_str());
    serv.sin_port = htons(udpDnsPort);

    int err = ::connect(sock, (const sockaddr*) &serv, sizeof(serv));
    errnocopy = errno;
    if (err == -1)
    {
        ::close(sock);
        logger.notice() << "::connect() failed: " << Util::Utility::get_errno_message(errnocopy);
        return errorString;
    }

    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = ::getsockname(sock, (sockaddr*) &name, &namelen);
    errnocopy = errno;
    if (err == -1)
    {
        ::close(sock);
        logger.notice() << "::getsockname() failed: " << Util::Utility::get_errno_message(errnocopy);
        return errorString;
    }

    char buffer[128];
    ::memset(buffer, 0, sizeof(buffer));

    const char* p = ::inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer));
    errnocopy = errno;
    if (p == NULL)
    {
        ::close(sock);
        logger.notice() << "::inet_ntop() failed: " << Util::Utility::get_errno_message(errnocopy);
        return errorString;
    }

    ::close(sock);
    resultString = const_cast<const char *>(buffer);

    return resultString;
}


// For any IP address belonging to this system which is actually connected
// to the internet, get the MAC address for that interface. The goal is
// to get the primary IP address' MAC address using the interface name for
// that ip address.
// Returns empty string if not found or any errors occured.
std::string EnetUtil::get_self_mac_address(Log::Logger logger, std::string interfaceName)
{
    std::string errorString;
    std::string resultString;
    int errnocopy = 0;

    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    errnocopy = errno;
    if (sock == -1)
    {
        logger.notice() << "get_self_mac_address() - ::socket() failed: " << Util::Utility::get_errno_message(errnocopy);
        return errorString;
    }

    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , interfaceName.c_str() , IFNAMSIZ-1);

    int err = ::ioctl(sock, SIOCGIFHWADDR, &ifr);
    errnocopy = errno;
    if (err == -1)
    {
        ::close(sock);
        logger.notice() << "get_self_mac_address() - ::ioctl() failed: " << Util::Utility::get_errno_message(errnocopy);
        return errorString;
    }

    ::close(sock);

    unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    char buffer[64];
    ::snprintf(buffer, sizeof (buffer), "Mac : %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n" ,
                                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    resultString = const_cast<const char *>(buffer);
    return resultString;

}

// This is an overloaded version of the function for callers that do not
// have a logger configured.
std::string EnetUtil::get_self_mac_address(std::string interfaceName)
{
    using namespace Util;

    std::string channelName = "self_mac_address";

    Log::Config::Vector configList;
    MainLogger::initialize( configList,
                            channelName,
                            Log::Log::Level::eDebug,
                            MainLogger::enableConsole,
                            MainLogger::disableLogFile
                          );

    Log::Logger logger(channelName.c_str());

    return get_self_mac_address(logger, interfaceName);
}

