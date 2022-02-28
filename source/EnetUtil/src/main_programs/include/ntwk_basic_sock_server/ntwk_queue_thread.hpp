
#include <Utility.hpp>
#include <circular_buffer.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <memory>
#include <thread>


namespace EnetUtil {


	class queue_thread
	{
	public:

		static void handler(Log::Logger logger);

		static void start (const char *logChannelName = "queue_thread");

		// The circular buffer queue has each shared_ptr<> added to it.  Each shared_ptr
		// points to a fixed_array object which holds data from a single socket read()
		// call, when it's ready.  The queue_handler thread pops out each member when it's
		// ready, and processes it.
		static const int s_queuesize;
		static Util::circular_buffer<std::shared_ptr<fixed_uint8_array_t>> s_ringbuf;

		// Adding a buffer to the queue with this condition variable as a second parameter
		// to ::put(), will allow the waiting queue thread to be suspended until there are
		// buffers waiting in the queue
		static Util::condition_data<int> s_queue_condvar;

		static std::thread s_queue_thread;
	};  // end of class queue_thread

} // end of namespace EnetUtil

