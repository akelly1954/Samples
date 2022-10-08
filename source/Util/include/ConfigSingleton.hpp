#pragma once

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

#include <json/json.h>
#include <iostream>
#include <mutex>
#include <memory>

namespace Config
{
    class ConfigSingleton;        // forward declaration
    using ConfigSingletonShrdPtr = std::shared_ptr<ConfigSingleton>;
    using IfsreamShrdPtr = std::shared_ptr<std::ifstream>;

    class ConfigSingleton : public std::enable_shared_from_this<ConfigSingleton>
    {
    private:
        // No public access to constructor. Have to use ::create()
        ConfigSingleton(void) = default;

        ConfigSingleton(const ConfigSingleton &) = delete;
        ConfigSingleton &operator=(ConfigSingleton const &) = delete;
        ConfigSingleton(ConfigSingleton &&) = delete;
        ConfigSingleton &operator=(ConfigSingleton &&) = delete;

    public:
        // this is the only option to construct the new object
        [[nodiscard]] static ConfigSingletonShrdPtr create(const std::string& filename, std::ostream& logstream);

        // This cannot be static (std::shared_from_this() needs "this->")
        ConfigSingletonShrdPtr get_shared_ptr();
        static Json::Value& JsonRoot()                    { return s_configRoot; }
        static Json::Value& GetJsonRootCopyRef()          { return s_editRoot; }
        static std::string JsonFileName()                 { return s_jsonfilename; }
        static ConfigSingletonShrdPtr instance();
        static bool initialize(std::ostream& logstream);

    public:
        // The caller can make changes to the internal copy of the JsonRoot
        // Node (the s_editRoot member is the editable copy). Once the changes
        // are done, this method - UpdateJsonConfigFile() is used to notify the
        // ConfigSingleton object that these changes are now ready to write to
        // disk (into the real json file - ::s_jsonfilename). After this operation
        // is finished, the caller can (should) restart their operation so that
        // the new values can take effect.
        bool UpdateJsonConfigFile(std::ostream& logstream, std::string UseTempFileName = std::string("tmp_") + s_jsonfilename);

    private:
        // static members
        static Json::Value s_configRoot;
        static std::string s_jsonfilename;
        static ConfigSingletonShrdPtr sp_Instance;
        static std::mutex s_mutex;
        static bool s_enabled;

        // This is a copy of the s_configRoot node (which is used for the
        // actual configuration by the calling app). The initial copy is made
        // during object construction (see ::create(). This s_editRoot node should
        // be used for making changes stepwise, and when ready, the using app will
        // call ::UpdateJsonConfigFile() which is to be used to update the json
        // file on disk, at which point the caller should "reset" their operation
        // to allow for their new values to take effect.
        static Json::Value s_editRoot;

    private:
        // This caller is requesting for this object to be done.
        // TODO: This member might go away if there are no child threads.
        bool m_finished = false;
    };

} // end of namespace Config

