#include <ntwk_basic_sock_server/ntwk_queue_thread.hpp>
#include <Utility.hpp>
#include <circular_buffer.hpp>
#include <commandline.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <iomanip>
#include <iostream>
#include <sstream>

//    std::cout << std::setfill('0') << std::setw(5) << 25;

#include <assert.h>

using namespace EnetUtil;

// class queue_thread statics

const int queue_thread::s_queuesize = 1000;
Util::circular_buffer<std::shared_ptr<fixed_uint8_array_t>> queue_thread::s_ringbuf(s_queuesize);
Util::condition_data<int> queue_thread::s_queue_condvar(0);
std::thread queue_thread::s_queue_thread;  // This gets set in ::start()

std::string queue_thread::get_seq_num_string(long num)
{
	std::ostringstream lstr;
	lstr << std::setfill('0') << std::setw(6) << num;
	return lstr.str();
}

void queue_thread::handler(Log::Logger logger)
{
    // FOR DEBUG    std::cout << "thread_handler(): started thread for connection "
    //                        << threadno << ", fd = " << socketfd << std::endl;

    logger.notice() << "queue_handler: thread running, waiting for the circular " <<
                       "buffer queue to have buffers ready to be processed.";

    bool finished = false;

    size_t sequential_num = 1;
    for (sequential_num = 1; !finished; sequential_num++)
    {
        // TODO: might use queue_condvar.get_data() for potential info
        // provided when the item was put in the queue.
        s_queue_condvar.wait_for_ready();

        while (!s_ringbuf.empty())
        {
            std::shared_ptr<fixed_uint8_array_t> data_sp = s_ringbuf.get();
            std::string output_filename = std::string("data_") +
            							  get_seq_num_string(sequential_num) +
										  ".txt";
            logger.debug() << "queue_handler(): Queue ready:  Got object with " <<
                    data_sp->num_valid_elements() << " valid elements in a fixed array of size " <<
                    data_sp->data().size();
            logger.debug() << "queue_handler(): Writing to file:  " << output_filename;
        }
    }
}

void queue_thread::start (const char *logChannelName)
{
	// This is still running in the main thread
	Log::Logger logger(logChannelName);
	logger.notice() << "start_queue_handler(): starting a circular buffer handler thread";

    try
    {
        // The logger is passed to the new thread because it has to be instantiated in
        // the main thread (right here) before it is used from inside the new thread.
        s_queue_thread = std::thread( queue_thread::handler, logger);
    }
    catch (std::exception &exp)
    {
        logger.error() << "Got exception in start_queue_handler() starting thread " <<
                          " for queue_handler(): " << exp.what();
    }
    catch (...)
    {
        logger.error() << "General exception occurred in start_queue_handler() starting " <<
                          "thread for queue_handler().";
    }

    logger.notice() << "start_queue_handler(): started thread for queue_handler()";
}


bool queue_thread::write_to_file(Log::Logger& logger,
								 std::shared_ptr<fixed_uint8_array_t> data_sp,
								 std::string output_file)
{
	if (data_sp->num_valid_elements() == 0)
	{
        logger.error() << "queue_thread::write_to_file: Got an empty buffer to write to " << output_file;
	}

/*
		if (readypair.first > 0)
    {
        char buffer[readypair.first+1];
        void *from = readypair.second.data();
        strncpy(buffer, static_cast<const char *>(from), readypair.first);
        buffer[readypair.first] = '\0';
        fwrite(buffer, readypair.first, 1, stdout);
    } */
	return true;
}

