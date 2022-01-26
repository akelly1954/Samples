#pragma once
#include <memory>
#include <string>
#include <vector>
#include <mutex>

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
