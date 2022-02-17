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

using namespace std;
using namespace EnetUtil;

struct nothing {
	int nothing1;
} nothing;

#ifdef NOBUILD
// class NtwkUtil static objects:
const size_t NtwkUtil::NtwkUtilBufferSize = 8192;
std::recursive_mutex NtwkUtil::m_recursive_mutex;
std::mutex NtwkUtil::m_mutex;

int NtwkUtil::enetSend(Log::Logger& logger,
						int fd,									// file descriptor to socket
						SizedUint8Array sendbuf,				// data and length
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
    	//             fd          data buffer         num bytes
        sockreturn = send(fd, sendbuf.second.get(), sendbuf.first, MSG_NOSIGNAL);
        errnocopy = errno;
        if (sockreturn >= 0)
        {
            break;
        } else
        {
            std::string errstr = Util::Utility::get_errno_message(errnocopy);
            std::string lstr = "NtwkUtil::enetSend: Failed to write to socket: " + errstr
                            + " socket = " + to_string(fd);
            logger.error() << lstr;
			if (i < retries)
			{
            	logger.notice() << "After write error, retrying in " + (sleepmsec / 1000) << " second(s)...";
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepmsec));
        }
    }
    return sockreturn;
}

// The socket read may throw an exception
// recvbuf is assumed to have no data
void NtwkUtil::enetReceive(Log::Logger& logger, int fd, SizedUint8Array recvbuf, size_t requestsize)
{
    try
    {
    	// Try not to overflow the array bounds. recvbuf.first is undefined (empty buffer) - use
    	// the array size as the limit.
    	size_t arrsize = recvbuf.second).get()->size();
    	size_t actual_requestsize = (requestsize > recvbuf.second->size()? recvbuf.second->size() : requestsize);

    	if (actual_requestsize < 4)
    	{
    		logger.error() << "NtwkUtil::enetReceive: have room for less than 4 bytes in empty buffer";
    		throw std::out_of_range("NtwkUtil::enetReceive: have room for less than 4 bytes");
    		return;
    	}

		signal(SIGPIPE, SIG_IGN);
        int bytesreceived = 0;
        do
        {
        	bool EOF = false;
        	int errnocopy = 0;
            int num = read(fd, recvbuf.second.data(), actual_requestsize - bytesreceived);
            errnocopy = errno;  // will be used if needed
            if (num > 0)
            {
                bytesreceived += num;
            }
            else if (num < 0)
            {
                logger.error() << "NtwkUtil::enetReceive: socket read error: " << Util::Utility::get_errno_message(errnocopy);
        		throw std::out_of_range("NtwkUtil::enetReceive: have room for less than 4 bytes");
            }
            else // num = 0
            {
                logger.notice() << "NtwkUtil::enetReceive: got end of file on socket read...";
            	EOF = true;
            }

        } while (!EOF && (bytesreceived < actual_requestsize));

        recvbuf.first = bytesreceived;
    } catch (std::exception &exp)
    {
        logger.error() << "Got exception in NtwkUtil::enetReceive after read(s) from socket: " << exp.what();
    } catch (...)
    {
    	logger.error() << "General exception occurred in NtwkUtil::enetReceive after read(s) from socket";
    }
}
#endif // NOBUILD

