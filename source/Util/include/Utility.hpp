#pragma once
#include <memory>
#include <string>
#include <vector>
#include <mutex>

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

using namespace std;

namespace Util {

	// This is where odds and ends go
	class Utility {

	private:
		// Not allowed:
		Utility(void) = delete;
		Utility(const Utility &) = delete;
		Utility &operator=(Utility const &) = delete;
		Utility(Utility &&) = delete;
		Utility &operator=(Utility &&) = delete;

	public:
		static long getUTCTimeAsLong();
		static string getUTCTimeAsString(const char *format = "%Y-%m-%dT%XZ");

		// Get a random int from within a range starting at "low", and
		// "low" + "range".  If no "low" number is specified, 0 is used.
		// For example, get_rand(10,3) gets you a random number between
		// 3 and 12 (inclusive).
		static int get_rand(int range, int low = 0);
	};

} // namespace Util
