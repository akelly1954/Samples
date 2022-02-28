#include <ntwk_basic_sock_server/ntwk_queue_thread.hpp>
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

std::vector<std::thread> socket_connection_thread::s_workers;

void socket_connection_thread::handler(int socketfd, int threadno, Log::Logger logger)
{
    // FOR DEBUG    std::cout << "socket_connection_thread::handler(): started thread for connection "
    //                        << threadno << ", fd = " << socketfd << std::endl;

    logger.debug() << "socket_connection_thread::handler(" << threadno << "): Beginning of thread for connection " << threadno << ", fd = " << socketfd;

    bool finished = false;
    while (!finished)
    {
        // This extra scope gets out of context at the end of file, which
        // destructs the shared_ptr<>, so that a new one is created here:
        {
            std::shared_ptr<fixed_uint8_array_t> sp_data = fixed_uint8_array_t::create();

            int num_elements_received = NtwkUtil::enet_receive(logger, socketfd, sp_data->data(), sp_data->data().size());

            logger.debug() << "socket_connection_thread::handler(" << threadno << "): Received " <<
                    num_elements_received << " bytes on fd " << socketfd;

            if (num_elements_received == 0)  // EOF
            {
                close(socketfd);
                return;  // The shared_ptr<> will be destructed
            }
            else
            {
                if (! sp_data->set_num_valid_elements(num_elements_received))
                {
                    logger.error() << "socket_connection_thread::handler: Error in setting socket_connection_thread::handler(" << threadno <<
                            ") num_valid_elements. Got  " << num_elements_received << " bytes on fd " << socketfd <<
                            ". Aborting...";
                    finished = true;
                }
            }
            logger.debug() << "socket_connection_thread::handler(" << threadno << "): After setting number of elements to " <<
                    num_elements_received << " bytes on fd " << socketfd;

            // Add the shared_ptr the queue. The condition variable will signal ready to the thread.
            // The shared_ptr then goes out of scope and is deleted (but ringbuf has a copy).
            queue_thread::s_ringbuf.put(sp_data, queue_thread::s_queue_condvar);
        }
    }
    close(socketfd);
}

void socket_connection_thread::start (int accpt_socket, int threadno, const char *logChannelName)
{
	// This affects the whole process:  signal(SIGPIPE, SIG_IGN);

	assert(logChannelName);

	Log::Logger logger(logChannelName);
	logger.debug() << "socket_connection_handler(): starting a connection handler thread: ";
	logger.debug() << "fd = " << accpt_socket << ", thread number: " << threadno << ", log channel: " << logChannelName;
    try
    {
        // The logger is passed to the new thread because it has to be instantiated in
        // the main thread (right here) before it is used from inside the new thread.
    	socket_connection_thread::s_workers.push_back( std::thread( socket_connection_thread::handler, accpt_socket, threadno, logger));
    }
    catch (std::exception &exp)
    {
        logger.error() << "Got exception in socket_connection_handler() starting thread " <<
                          threadno << " for socket fd " << accpt_socket << ": " << exp.what();
    }
    catch (...)
    {
        logger.error() << "General exception occurred in socket_connection_handler() starting thread " <<
                          threadno << " for socket fd " << accpt_socket;
    }

    logger.debug() << "socket_connection_handler(): started thread " <<
                          threadno << " for socket fd " << accpt_socket;
}



