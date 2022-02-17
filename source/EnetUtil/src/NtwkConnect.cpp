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

// TODO: Move some member functions out of .hpp to here

