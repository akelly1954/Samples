#include "NtwkUtil.hpp"
#include "Utility.hpp"
#include <LoggerCpp/LoggerCpp.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <chrono>
#include <thread>
#include <stdexcept>

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

using namespace EnetUtil;

// SOCKET UTILITIES

// class NtwkUtil static objects:
std::mutex NtwkUtil::m_send_mutex;
std::mutex NtwkUtil::m_recv_mutex;

// The ip address and port number are used to set the proper values
// in the empty address structure for all the utilities used in EnetUtil.
// If the listen address (e.g. "192.168.0.102") is NULL, the INADDR_ANY value (0)
// will be used in the address structure.
bool NtwkUtil::setup_sockaddr_in(std::string ip_address,     // in
                                 uint16_t socket_port_number,         // in
                                 struct ::sockaddr *addr)              // out
{
    struct ::sockaddr_in *empty_addr = (::sockaddr_in *) addr;
    empty_addr->sin_family = AF_INET;

    ::in_addr_t x;
    if (ip_address.empty() || ip_address == "INADDR_ANY")
    {
        x = INADDR_ANY;
    }
    else
    {
        x = inet_addr(ip_address.c_str());
    }

    empty_addr->sin_addr.s_addr = x;
    empty_addr->sin_port = htons(socket_port_number);
    return true;

}

// For non-listening processes (setsockopt(), bind() etc are not needed).
// Returns socket file descriptor, or -1 on error.
// We use the logger because we want to capture errno as early as possible
// after an important system call(socket(), listen(), bind(), etc) and log it.
int NtwkUtil::client_socket_connect(Log::Logger& logger, struct ::sockaddr *address)
{
    struct ::sockaddr_in *address_struct = (::sockaddr_in *) address;

    int errnocopy = 0;            // Captures errno right after system call
    int socket_fd;

    // Creating socket file descriptor
    if ((socket_fd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        errnocopy = errno;
        logger.error() << "In NtwkUtil::client_socket_connect: socket() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }

    if (::connect(socket_fd, (struct ::sockaddr *) address_struct, sizeof(::sockaddr_in)) != 0)
    {
        errnocopy = errno;
        logger.error() << "In NtwkUtil::client_socket_connect): socket connect() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }

    return socket_fd;
}

// Returns socket file descriptor, or -1 on error.
// We use the logger because we want to capture errno as early as possible
// after an important system call(socket(), listen(), bind(), etc) and log it.
int NtwkUtil::server_listen(Log::Logger& logger, struct ::sockaddr *address, int backlog_size)
{
    struct ::sockaddr_in *address_struct = (::sockaddr_in *) address;

    int errnocopy = 0;            // Captures errno right after system call
    int socket_fd = -1;         // socket file descriptor

    // For setsockopt(2), this parameter should be non-zero to enable a boolean option,
    // or zero if the option is to be disabled
    int optval = 1;

    // signal(SIGPIPE, SIG_IGN);  // this will affect all running threads

    // Creating socket file descriptor
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        errnocopy = errno;
        logger.error() << "In NtwkUtil::server_listen(): socket() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)))
    {
        errnocopy = errno;
        logger.error() << "In NtwkUtil::server_listen(): setsockopt() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }

    if (bind(socket_fd, (struct sockaddr *)address_struct, sizeof(::sockaddr_in)) < 0)
    {
        errnocopy = errno;
        logger.error() << "In NtwkUtil::server_listen(): bind() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }

    if (listen(socket_fd, backlog_size) < 0)
    {
        errnocopy = errno;
        logger.error() << "In NtwkUtil::serverListen(): listen() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }
    return socket_fd;
}

// Accepts a single connection request.  It is expected to be called
// from within a loop.  For each loop cycle, the caller would
// then start a thread to deal with the request right after this call,
// if and only if the file descriptor is a valid positive file descriptor
// from a successful accept() system call, and the ::sockaddr_in structure
// is valid.
int NtwkUtil::server_accept(Log::Logger& logger, int listen_socket_fd, struct ::sockaddr *address, int retries)
{
    struct ::sockaddr_in *address_struct = (::sockaddr_in *) address;

    int errnocopy = 0;            // Captures errno right after system call
    int accept_socket_fd = -1;
    int address_length = sizeof(::sockaddr_in);

    while (--retries >= 0)
    {
        if ((accept_socket_fd = accept(listen_socket_fd,
                                       (struct sockaddr *) address_struct,
                                       (socklen_t *) &address_length)) < 0)
        {
            errnocopy = errno;
            Util::Utility::get_errno_message(errnocopy);
            logger.error() << "In server_accept(): accept() failed on fd " <<
                              listen_socket_fd << ": " << Util::Utility::get_errno_message(errnocopy);
            continue;
        }
        return accept_socket_fd;
    }

    logger.error() << "NtwkUtil::server_accept(): No retries left.  Aborting...";
    return -1;
}

int NtwkUtil::enet_send(Log::Logger& logger,
                        int fd,                    // file descriptor to socket
                        arrayUint8& array_element_buffer,    // data and length of buffer
                        size_t actual_size,
                        int flag)
{
    // std::lock_guard<std::mutex> lock(NtwkUtil::m_send_mutex);

    int sockreturn = -1;
    int errnocopy = 0;

    //                fd       data buffer              num bytes      flag
    sockreturn = send(fd, array_element_buffer.data(), actual_size, MSG_NOSIGNAL);
    errnocopy = errno;

    if (sockreturn >= 0)
    {
        // logger.debug() << "NtwkUtil::enet_send: sent " << sockreturn << " bytes on socket fd = " << fd;
    }
    else
    {
        logger.error() << "NtwkUtil::enet_send: Failed to write to socket: " <<
                          Util::Utility::get_errno_message(errnocopy) <<
                          ", socket fd = " << fd;
    }
    return sockreturn;
}

// The socket read may throw an exception
// recvbuf is assumed to have no data
int NtwkUtil::enet_receive( Log::Logger& logger,
                            int fd,
                            arrayUint8 & array_element_buffer,    // data and length
                            size_t requestsize)   // requestsize can be smaller than the array<>::size()
{
    // std::lock_guard<std::mutex> lock(NtwkUtil::m_recv_mutex);
    int bytesreceived = 0;

    try
    {
        // Try not to overflow the array bounds. recvbuf.first is undefined (empty buffer) - use
        // the array size as the limit.
        size_t actual_requestsize = (requestsize > array_element_buffer.size()? array_element_buffer.size() : requestsize);

        if (actual_requestsize < 1)
        {
            logger.error() << "NtwkUtil::enet_receive: have no room in the receive buffer";
            throw std::out_of_range("NtwkUtil::enet_receive: have no room in the receive buffer");
            return -1;  // Shouldn't get here...
        }

        if (fd < 0)
        {
            logger.error() << "NtwkUtil::enet_receive: got receive request with invalid socket file descriptor";
            throw std::runtime_error("NtwkUtil::enet_receive: got receive request with invalid socket file descriptor");
            return -1;  // Shouldn't get here...
        }

        int errnocopy = 0;

        // Removed MSG_WAITALL from the flags (4th) parameter to recv() since it causes
        // a deadlock with the client when the last send from the client while it is sending
        // the contents of a file, contains less bytes than the fixed buffer size requested.
        // (MSG_WAITALL tells the system to wait until the full size requested has arrived. Deadlock.)
        int num = recv(fd, array_element_buffer.data(), actual_requestsize, 0);
        errnocopy = errno;  // will be used if needed
        if (num > 0)
        {
            bytesreceived += num;
        }
        else if (num < 0)
        {
            logger.error() << "NtwkUtil::enet_receive: socket read error: " << Util::Utility::get_errno_message(errnocopy);
            throw std::runtime_error(
                    std::string("NtwkUtil::enet_receive: socket read error: ") + Util::Utility::get_errno_message(errnocopy));
            return -1;  // Should never even get here....
        }

        // if (num == 0)  // Remove soon - this just for a debugger breakpoint
        //      return bytesreceived;
        // logger.debug() << "NtwkUtil::enet_receive: got end of file/disconnect on socket read...";

        return bytesreceived;

    } catch (std::exception &exp)
    {
        logger.error() << "Got exception in NtwkUtil::enet_receive after recv() from socket: " << exp.what();
    } catch (...)
    {
        logger.error() << "General exception occurred in NtwkUtil::enet_receive after recv() from socket";
    }
    // should only get here from the catch{} blocks
    return -1;
}

// Get a string message from the network
// retstring is an existing std::string - contents overwritten
// socket_fd - open connection to the remote system
bool NtwkUtil::get_ntwk_message(Log::Logger& logger, int socket_fd, std::string& retstring)
{
    // get server response (single buffer with const char * message)
    arrayUint8 responsebuffer;
    memset (&responsebuffer[0], 0, responsebuffer.size());

    int ret = NtwkUtil::enet_receive(logger, socket_fd, responsebuffer, responsebuffer.size());

    if (ret == 0)// EOF
    {
        retstring = "NtwkUtil::get_ntwk_message Error: Got EOF (0 bytes) from remote connection. Socket fd = ";
        retstring += std::to_string(socket_fd);
        return false;
    }
    else if (ret == -1)
    {
        retstring =  "NtwkUtil::get_ntwk_message Error: trying to get remote connection message. Socket fd = ";
        retstring += std::to_string(socket_fd);
        return false;
    }

    retstring = std::string((const char *) responsebuffer.data());

    return true;
}


bool NtwkUtil::send_ntwk_message(Log::Logger& logger, int socket_fd, std::string& message)
{
    arrayUint8 messagebuf;

    size_t num = message.size() < messagebuf.size()?  message.size() : messagebuf.size();
    ::memset(messagebuf.data(), 0, messagebuf.size());
    strncpy((char *) (&messagebuf[0]), message.c_str(), num);
    if (num < messagebuf.size())
    {
        messagebuf[num] = '\0';        // String terminating zero byte.
    }
    else
    {
        messagebuf[messagebuf.size()-1] = '\0';        // String terminating zero byte.
    }

    int ret = NtwkUtil::enet_send(logger, socket_fd, messagebuf, messagebuf.size(), MSG_NOSIGNAL);
    if (ret < 0)
    {
        logger.error() << "NtwkUtil::send_ntwk_message: Error sending message to connection fd " << socket_fd;
        return false;
    }
    else if (ret == 0)
    {
        logger.error() << "NtwkUtil::send_ntwk_message: No message data was sent to connection fd " << socket_fd;
        return false;
    }

    logger.debug() << "NtwkUtil::send_ntwk_message: Message sent to connection fd " << socket_fd << ": \"" << message << "\"";
    return true;
}


// END OF SOCKET UTILITIES


