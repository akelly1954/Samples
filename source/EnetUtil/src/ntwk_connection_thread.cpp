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

// One of these objects is instantiated in every thread which is started from
// start() below for every client connection which is accepted.
auto thread_connection_handler = [](int socketfd, int threadno, Log::Logger logger)
{
    // FOR DEBUG    std::cout << "socket_connection_thread::handler(): started thread for connection "
    //                        << threadno << ", fd = " << socketfd << std::endl;

	// logger.debug() << "socket_connection_thread::handler(" << threadno << "): Beginning of thread for connection " << threadno << ", fd = " << socketfd;

    /////////////////
    // Loop through all enet_receive()'s for this connection and write
	// the data to a unique file name (associated with thread number)
    /////////////////

	std::string output_filename = std::string("tests/output_") +
								  socket_connection_thread::get_seq_num_string(threadno) +
								  ".data";

	FILE *output_stream = NULL;
	int errnocopy = 0;
	bool finished = false;
    while (!finished)
    {
        // This extra scope gets out of context at the end of file, which
        // destructs the shared_ptr<>, so that a new one is created here:
        {
            std::shared_ptr<fixed_uint8_array_t> sp_data = fixed_uint8_array_t::create();

            int num_elements_received = NtwkUtil::enet_receive(logger, socketfd, sp_data->data(), sp_data->data().size());

            // logger.debug() << "socket_connection_thread::handler(" << threadno << "): Received " <<
            //         num_elements_received << " bytes on fd " << socketfd;

            if (num_elements_received == 0)  // EOF
            {
                finished = true;
                continue;
            }
            else
            {
            	if (! sp_data->set_num_valid_elements(num_elements_received))
                {
                    logger.error() << "socket_connection_thread::handler: Error in setting socket_connection_thread::handler(" << threadno <<
                            ") num_valid_elements. Got  " << num_elements_received << " elements on fd " << socketfd <<
                            ". Aborting...";
                    finished = true;
                    continue;
                }
				// logger.debug() << "socket_connection_thread::handler(" << threadno << "): " <<
				// 		"Set number of valid elements to " <<
				// 		num_elements_received << " bytes on fd " << socketfd;

				bool finished = false;

				//
				// Write to file
				//
				if (output_stream == NULL)
				{
					// File has not been created yet.
					if ((output_stream = ::fopen (output_filename.c_str(), "w")) == NULL)
					{
						errnocopy = errno;
						logger.error() << "Cannot create/truncate output file (thread " << threadno << ") \"" <<
										  output_filename << "\": " << Util::Utility::get_errno_message(errnocopy);
						finished = true;
						continue;
					}
					// logger.debug() << "Created/truncated output file (thread " << threadno << ") \"" << output_filename << "\"";

				}

				size_t elementswritten = std::fwrite(&sp_data->data()[0], sizeof(uint8_t), sp_data->num_valid_elements(), output_stream);
				errnocopy = errno;

				if (elementswritten != sp_data->num_valid_elements())
				{
					errnocopy = errno;
					logger.error() << "Error writing output file (thread " << threadno << ") \"" <<
									  output_filename << "\": " << Util::Utility::get_errno_message(errnocopy);
					finished = true;
					continue;
				}
			}
		}
	}

	// CLEANUP.

    if (output_stream != NULL) fclose(output_stream);
    close(socketfd);
};


void socket_connection_thread::start (int accpt_socket, int threadno, const char *logChannelName)
{
	assert(logChannelName);

	Log::Logger logger(logChannelName);
	// logger.debug() << "socket_connection_handler(): starting a connection handler thread: ";
	// logger.debug() << "fd = " << accpt_socket << ", thread number: " << threadno << ", log channel: " << logChannelName;

	try
    {
		std::thread *sp_queue_thread = NULL;

		// The next two sets of braces are for scope. The first set protects the vector
		// critical region, and the second protects the map critical region.
		{
			// The logger is passed to the new thread because it has to be instantiated in
			// the main thread (right here) before it is used from inside the new thread.
			std::lock_guard<std::mutex> lock(s_vector_mutex);
			socket_connection_thread::s_connection_workers.push_back(
						std::thread( thread_connection_handler, accpt_socket, threadno, logger)
					);
		}
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

    // logger.debug() << "socket_connection_handler(): started thread " <<
    //                       threadno << " for socket fd " << accpt_socket;
}

std::string socket_connection_thread::get_seq_num_string(long num)
{
	std::ostringstream lstr;
	lstr << std::setfill('0') << std::setw(6) << num;
	return lstr.str();
}

