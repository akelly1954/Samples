#include <ntwk_basic_sock_server/ntwk_connection_thread.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

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

// class socket_connection_thread statics

std::mutex socket_connection_thread::s_vector_mutex;
std::vector<std::thread> socket_connection_thread::s_connection_workers;

// This function is in a new thread which is started for every accepted connection
// from start() below.
void thread_connection_handler(int socketfd, int threadno, Util::LoggerSPtr loggerp)
{
    using Util::Utility;

    // FOR DEBUG    std::cout << "socket_connection_thread::handler(): started thread for connection "
    //                        << threadno << ", fd = " << socketfd << std::endl;

    // TODO: Commented out for DEBUG
    // loggerp->debug() << "socket_connection_thread::handler(" << threadno <<
    //                   "): Beginning of thread for connection " << threadno << ", fd = " << socketfd;

    /////////////////
    // First get the initial client message with the bytecount and filename.
    // Then, loop through all enet_receive()'s for this connection and write
    // the data to a unique file name (associated with thread number)
    /////////////////

    // get initial client message in a string
    std::string message;
    if (NtwkUtil::get_ntwk_message(loggerp, socketfd, message))
    {
        loggerp->debug() << "thread_connection_handler: Initial client message: " << message;
    }
    else
    {
        loggerp->error() << message;
        if (socketfd >= 0) ::close(socketfd);
        return;
    }

    std::vector<std::string> remote_message_vector = Utility::split(message, "|");

    // for (std::string const& str: remote_message_vector)
    // {
    //     loggerp->debug() << str;
    // }

    if (remote_message_vector.size() != 2)
    {
        loggerp->error() << "thread_connection_handler: ERROR: Initial client message expects two fields.  Received " <<
                          remote_message_vector.size() << ". Terminating connection...";
        if (socketfd >= 0) ::close(socketfd);
        return;
    }

    std::string remote_filename = remote_message_vector[0];
    std::string remote_bytecount_string = remote_message_vector[1];
    size_t remote_bytecount = strtoul(remote_bytecount_string.c_str(), NULL, 10);

    std::string output_filename = std::string("tests/output_") +
                                  socket_connection_thread::get_seq_num_string(threadno) +
                                  "." +
                                  remote_filename;

    loggerp->debug() << "thread_connection_handler: byte count from remote = " <<
                       std::to_string(remote_bytecount) << ", local server output to \"" <<
                       output_filename << "\"";

    FILE *output_stream = NULL;
    long long bytesremaining = remote_bytecount;
    size_t totalbyteswritten = 0;
    int errnocopy = 0;
    bool finished = false;
    while (!finished)
    {
        std::shared_ptr<fixed_uint8_array_t> sp_data = fixed_uint8_array_t::create();

        int num_elements_received = NtwkUtil::enet_receive(loggerp, socketfd, sp_data->data(), sp_data->data().size());
        // loggerp->debug() << "socket_connection_thread::handler(" << threadno << "): Read " <<
        //     num_elements_received << " bytes on fd " << socketfd << ", remaining: " << bytesremaining;

        if (num_elements_received == 0)// EOF
        {
            finished = true;
            continue;
        }
        else
        {
            if (! sp_data->set_num_valid_elements(num_elements_received))
            {
                loggerp->error() << "socket_connection_thread::handler: Error in setting socket_connection_thread::handler(" << threadno <<
                ") num_valid_elements. Got  " << num_elements_received << " elements on fd " << socketfd <<
                ". Aborting...";
                finished = true;
                continue;
            }
            //loggerp->debug() << "socket_connection_thread::handler(" << threadno << "): " <<
            //                    "Set number of valid elements to " <<
            //                    num_elements_received << " bytes on fd " << socketfd;

            finished = false;

            //
            // Write to file
            //
            if (output_stream == NULL)
            {
                // File has not been created yet.
                if ((output_stream = ::fopen (output_filename.c_str(), "w")) == NULL)
                {
                    errnocopy = errno;
                    loggerp->error() << "Cannot create/truncate output file (thread " << threadno << ") \"" <<
                    output_filename << "\": " << Utility::get_errno_message(errnocopy);
                    finished = true;
                    continue;
                }
                // loggerp->debug() << "Created/truncated output file (thread " << threadno << ") \"" << output_filename << "\"";
            }

            size_t elementswritten = std::fwrite(sp_data->data().data(), sizeof(uint8_t), sp_data->num_valid_elements(), output_stream);
            errnocopy = errno;
            size_t byteswritten = (elementswritten * sizeof(uint8_t));
            totalbyteswritten += byteswritten;
            fflush(output_stream);

            if (elementswritten != sp_data->num_valid_elements())
            {
                errnocopy = errno;
                loggerp->error() << "Error writing output file (thread " << threadno << ") \"" <<
                output_filename << "\": " << Utility::get_errno_message(errnocopy);
                finished = true;
                continue;
            }

            bytesremaining -= byteswritten;
            // loggerp->debug() << "Wrote " << (elementswritten * sizeof(uint8_t)) << " bytes into " << output_filename << ". Bytes remaining: " << bytesremaining;
            if (bytesremaining <= 0)
            {
                finished = true;
                continue;
            }
        }
    }

    if (output_stream != NULL)
    {
        fflush(output_stream);
    }

    // Respond to the file transfer
    std::string response =
        std::string("OK|") + std::to_string(threadno) + std::string("|") + output_filename + std::string("|") + std::to_string(totalbyteswritten);

    // No need to check return - the function writes to the
    // log file, and we are done anyways.
    NtwkUtil::send_ntwk_message(loggerp, socketfd, response);

    // CLEANUP.

    if (output_stream != NULL)
    {
        fclose(output_stream);
    }
    close(socketfd);
};

void socket_connection_thread::start(int accpt_socket, int threadno,  Util::LoggerSPtr loggerp)
{
    // TODO: Need to check this out
    loggerp->setLevel(Log::Log::eDebug);

    // TODO: Commented out for debug
    // loggerp->debug() << "socket_connection_handler(): starting a connection handler thread: ";
    // loggerp->debug() << "fd = " << accpt_socket << ", thread number: " << threadno;  //  << ", log channel: " << logChannelName;

    try
    {
        // This next set of braces is for scope. Protecting the workers vector.
        {
            // The logger is passed to the new thread because it has to be instantiated in
            // the main thread (right here) before it is used from inside the new thread.
            std::lock_guard<std::mutex> lock(s_vector_mutex);
            socket_connection_thread::s_connection_workers.push_back(
                    std::thread(thread_connection_handler, accpt_socket, threadno, loggerp));
        }
    } catch (std::exception &exp)
    {
        loggerp->error()
                << "Got exception in socket_connection_handler() starting thread "
                << threadno << " for socket fd " << accpt_socket << ": "
                << exp.what();
    } catch (...)
    {
        loggerp->error()
                << "General exception occurred in socket_connection_handler() starting thread "
                << threadno << " for socket fd " << accpt_socket;
    }

    // loggerp->debug() << "socket_connection_handler(): started thread " <<
    //                       threadno << " for socket fd " << accpt_socket;
}

std::string socket_connection_thread::get_seq_num_string(long num)
{
    std::ostringstream lstr;
    lstr << std::setfill('0') << std::setw(6) << num;
    return lstr.str();
}

