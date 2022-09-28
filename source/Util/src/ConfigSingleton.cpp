
//   TODO:             WORK IN PROGRESS

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

#include <ConfigSingleton.hpp>

using namespace Config;

ConfigSingletonShrdPtr ConfigSingleton::sp_Instance;
std::mutex ConfigSingleton::s_mutex;
bool ConfigSingleton::s_enabled = false;

ConfigSingletonShrdPtr ConfigSingleton::instance()
{
	// No locking done here to make the code fast.
	// if s_enabled is true, it's not going to change
	// for the lifetime of the program, hence no locking
	// necessary and it's fast.
	if (ConfigSingleton::s_enabled)
	{
		return ConfigSingleton::sp_Instance;
	}

	// The instance has not yet been created
	std::lock_guard<std::mutex> lock(s_mutex);

	// Checking a second time because the lock wasn't in effect before.
	// This means that the first check does not involve locking, which
	// is faster for just about all calls ( (just about means the vast
	// majority) to this function.
	if (!s_enabled)
	{
		ConfigSingleton::sp_Instance = ConfigSingleton::create();

		// TODO: Lots of initialization is yet to be done here... also, this shouldn't be static
		if (!ConfigSingleton::initialize())
		{
			; // TODO: error code
		}
		s_enabled = true;
	}
	return sp_Instance;
}











