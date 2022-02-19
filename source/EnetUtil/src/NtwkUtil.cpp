#include "NtwkUtil.hpp"
#include "Utility.hpp"
#include <LoggerCpp/LoggerCpp.h>
#include "unistd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <chrono>
#include <thread>
#include <stdexcept>

using namespace EnetUtil;

// class NtwkUtil static objects:
std::recursive_mutex NtwkUtil::m_recursive_mutex;
std::mutex NtwkUtil::m_mutex;

int NtwkUtil::enetSend(Log::Logger& logger,
						int fd,	                // file descriptor to socket
						std::array<uint8_t,NtwkUtil::NtwkUtilBufferSize>& array_element_buffer,	// data and length
						std::recursive_mutex& mutex,
						int flag)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    const int sleepmsec = 3000;
    const int retries = 3;

    int sockreturn = -1;
    int errnocopy = 0;
    for (int i = 1; i <= retries; i++)
    {
    	//             fd          data buffer              num bytes
        sockreturn = send(fd, array_element_buffer.data(), array_element_buffer.size(), MSG_NOSIGNAL);
        errnocopy = errno;
        if (sockreturn >= 0)
        {
            break;
        } else
        {
            // std::string errstr = Util::Utility::get_errno_message(errnocopy);
            std::string lstr = "NtwkUtil::enetSend: Failed to write to socket: "
            				+ Util::Utility::get_errno_message(errnocopy)
                            + " socket fd = " + std::to_string(fd);
            logger.error() << lstr;
			if (i < retries)
			{
            	logger.notice() << "After write error, retrying in " + (sleepmsec / 1000) << " second(s)...";
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepmsec));
        }
    }

    // TODO: GET THIS OUT OF HERE
	std::shared_ptr<fixed_size_array<uint8_t, NtwkUtilBufferSize>> sp1 = fixed_size_array<uint8_t,NtwkUtilBufferSize>::create();
	logger.info() << "Created sp to fixed array with " << std::to_string(sp1->num_elements()) << " elements";

    return sockreturn;
}

// The socket read may throw an exception
// recvbuf is assumed to have no data
int NtwkUtil::enetReceive(	Log::Logger& logger,
							int fd,
							std::array<uint8_t,NtwkUtil::NtwkUtilBufferSize>& array_element_buffer,	// data and length
							size_t requestsize)
{
    int bytesreceived = 0;

    try
    {
    	// Try not to overflow the array bounds. recvbuf.first is undefined (empty buffer) - use
    	// the array size as the limit.
    	size_t actual_requestsize = (requestsize > array_element_buffer.size()? array_element_buffer.size() : requestsize);

    	if (actual_requestsize < 1)
    	{
    		logger.error() << "NtwkUtil::enetReceive: have room for less than 1 byte in empty buffer";
    		throw std::out_of_range("NtwkUtil::enetReceive: have room for less than 1 byte");
    		return -1;  // Shouldn't get here...
    	}

    	if (fd < 0)
    	{
    		logger.error() << "NtwkUtil::enetReceive: got receive request with invalid socket file descriptor";
    		throw std::runtime_error("NtwkUtil::enetReceive: got receive request with invalid socket file descriptor");
    		return -1;  // Shouldn't get here...
    	}

		signal(SIGPIPE, SIG_IGN);
    	bool endOfFile = false;
        do
        {
        	int errnocopy = 0;
            int num = read(fd, array_element_buffer.data(), actual_requestsize - bytesreceived);
            errnocopy = errno;  // will be used if needed
            if (num > 0)
            {
                bytesreceived += num;
            }
            else if (num < 0)
            {
                logger.error() << "NtwkUtil::enetReceive: socket read error: " << Util::Utility::get_errno_message(errnocopy);
        		throw std::runtime_error(
        				std::string("NtwkUtil::enetReceive: socket read error: ") + Util::Utility::get_errno_message(errnocopy));
            }
            else // num = 0
            {
                logger.notice() << "NtwkUtil::enetReceive: got end of file/disconnect on socket read...";
            	endOfFile = true;
            }

        } while (!endOfFile && (bytesreceived < actual_requestsize));

    } catch (std::exception &exp)
    {
        logger.error() << "Got exception in NtwkUtil::enetReceive after read(s) from socket: " << exp.what();
    } catch (...)
    {
    	logger.error() << "General exception occurred in NtwkUtil::enetReceive after read(s) from socket";
    }
    return bytesreceived;
}

