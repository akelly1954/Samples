#pragma once

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

#include <mutex>
#include <memory>

namespace Config
{
	class ConfigSingleton;		// forward declaration
	using ConfigSingletonShrdPtr = std::shared_ptr<ConfigSingleton>;

	class ConfigSingleton : public std::enable_shared_from_this<ConfigSingleton>
	{
	private:
		// No public access to constructor. Have to use ::create()
		ConfigSingleton() = default;
	    ConfigSingleton(const ConfigSingleton &) = delete;
	    ConfigSingleton &operator=(ConfigSingleton const &) = delete;
	    ConfigSingleton(ConfigSingleton &&) = delete;
	    ConfigSingleton &operator=(ConfigSingleton &&) = delete;

	public:

		[[nodiscard]] static std::shared_ptr<ConfigSingleton> create(void)
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			return std::shared_ptr<ConfigSingleton>(new ConfigSingleton());
		}

		ConfigSingletonShrdPtr get_shared_ptr()
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			return this->shared_from_this();
		}

	public:
		static ConfigSingletonShrdPtr instance();
		static bool initialize(void) { return true; }// TODO: right now nothing. Later on, something.

	private:
		// static members
		static ConfigSingletonShrdPtr sp_Instance;
		static std::mutex s_mutex;
		static bool s_enabled;


	private:
		bool m_finished = false;
	};












} // end of namespace Config

