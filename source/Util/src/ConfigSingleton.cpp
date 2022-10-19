
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
#include <JsonCppUtil.hpp>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace Config;

////////////////////////////////////////////////////
// Some utility functions
////////////////////////////////////////////////////

bool checkFileExists(std::string fname)
{
    struct ::stat statbuf;
    return ::stat(fname.c_str(),  &statbuf) == 0;
}

bool checkFileIsNotEmpty(std::string fname)
{
    struct ::stat statbuf;

    // Fail if non-existent
    if (::stat(fname.c_str(),  &statbuf) != 0) return false;

    if (statbuf.st_size == 0) return false;

    return true;
}

////////////////////////////////////////////////////
// class ConfigSingleton members
////////////////////////////////////////////////////

// statics
Json::Value ConfigSingleton::s_configRoot;    // THE root JSON node
Json::Value ConfigSingleton::s_editRoot;      // Editable copy of the root JSON node
ConfigSingletonShrdPtr ConfigSingleton::sp_Instance;
std::mutex ConfigSingleton::s_mutex;
bool ConfigSingleton::s_enabled = false;
std::string ConfigSingleton::s_jsonfilename = std::string("default_config_filename.json");

ConfigSingletonShrdPtr ConfigSingleton::create(const std::string& filename, std::ostream& logstream)
{
    if (ConfigSingleton::s_enabled)
    {
        return ConfigSingleton::sp_Instance->shared_from_this();
    }

    std::lock_guard<std::mutex> lock(ConfigSingleton::s_mutex);

    // second identical check in case of race condition (because we're locked now)
    if (ConfigSingleton::s_enabled)
    {
        return ConfigSingleton::sp_Instance->shared_from_this();
    }

    std::ifstream ifs(filename);
    ConfigSingleton::s_jsonfilename = filename;

    ifs.open(filename);
    if (! ifs.is_open())
    {
        // JsonCpp does not check this, but will fail with a syntax error on the first read
        std::string excpstr = std::string("Error initializing ConfigSingleton object: Could not open json file ") +
                                          ConfigSingleton::s_jsonfilename;
        throw std::runtime_error(excpstr);
        // and exit()
    }

    ConfigSingleton::sp_Instance = ConfigSingletonShrdPtr(new ConfigSingleton());

    if (!ConfigSingleton::initialize(logstream))
    {
        throw std::runtime_error("Error initializing ConfigSingleton object");
    }

    ConfigSingleton::s_enabled = true;
    return ConfigSingleton::sp_Instance;
}

ConfigSingletonShrdPtr ConfigSingleton::get_shared_ptr()
{
    std::lock_guard<std::mutex> lock(ConfigSingleton::s_mutex);
    if (! ConfigSingleton::s_enabled)
    {
        throw std::runtime_error("ConfigSingleton::get_shared_ptr() called before ::create()");
    }
    return sp_Instance->shared_from_this();
}

ConfigSingletonShrdPtr ConfigSingleton::instance()
{
    return ConfigSingleton::sp_Instance;
}

bool ConfigSingleton::initialize(std::ostream& logstream)
{
    // temporary root
    Json::Value root;

    std::ifstream ifs(ConfigSingleton::s_jsonfilename);
    if (! ifs.is_open())
    {
        // JsonCpp does not check this, but will fail with a syntax error on the first read
        std::string excpstr = std::string("Error initializing ConfigSingleton object: Could not open json file ") +
                                          ConfigSingleton::s_jsonfilename;
        throw std::runtime_error(excpstr);
        // and exit()
    }

    if (UtilJsonCpp::checkjsonsyntax(logstream, ifs, root) == EXIT_FAILURE)
    {
        ifs.close();
        return false;
    }

    // store the temporary root into the real one
    ConfigSingleton::s_configRoot = root;

    // Make the initial copy of the JSON root object to make the editable copy.
    ConfigSingleton::s_editRoot = Config::ConfigSingleton::s_configRoot;

    ifs.close();
    return true;
}

bool ConfigSingleton::UpdateJsonConfigFile(std::ostream& logstream, std::string tempfilename)
{
    std::lock_guard<std::mutex> lock(ConfigSingleton::s_mutex);

    int errnocopy = 0;

    // First, unlink (remove) previous temp file if it exists.
    // We don't check for errors since the file may not be there,
    // which would create an inappropriate error condition.  If
    // it doesn't work, the next std::ofstream operation will
    // fail appropriately.
    ::unlink(tempfilename.c_str());

    // Now, write out the current copy of the json file to a temp file
    std::ofstream tmpcfgfile(tempfilename, std::ofstream::trunc | std::ofstream::out);
    if ( ! tmpcfgfile.is_open())
    {
        errnocopy = errno;

        // JsonCpp does not check this, but will fail with a syntax error on the first read
        logstream << "ERROR: UpdateJsonConfigFile() could not open/create the new json file " << tempfilename
                       << ": " << strerror(errnocopy) << ", aborted...\n";
        tmpcfgfile.close();
        return false;
    }

    tmpcfgfile << ConfigSingleton::s_editRoot;
    tmpcfgfile.close();

    // At this point, the modified json has been written into tempfilename.
    // The most important file here is the old json file, which we have to save
    // before replacing it with the new json file.
    std::string savejsonfilename = std::string("save_") + ConfigSingleton::JsonFileName();

    // Get rid of the old/existing copy.
    if (checkFileExists(savejsonfilename))
    {
        logstream << "File exists: " << savejsonfilename << "\n";
        logstream << "Unlinking " << savejsonfilename;
        ::unlink(savejsonfilename.c_str());
    }
    else
    {
        logstream << "File does not exist: " << savejsonfilename << "\n";
    }

    if (checkFileExists(savejsonfilename))
    {
        logstream << "File still exists: " << savejsonfilename << ". Aborted.\n";
        return false;
    }

    // Save the existing json file by linking it (hard link) to the now, non-existent save_ file.
    logstream << "Linking (::link()) the original json file "
                  << ConfigSingleton::s_jsonfilename << " to the new saved copy "
                  << savejsonfilename << ".\n";
    if (::link(ConfigSingleton::s_jsonfilename.c_str(), savejsonfilename.c_str()) != 0)
    {
        errnocopy = errno;
        logstream << "ERROR: UpdateJsonConfigFile() could not ::link() the original json file "
                       << ConfigSingleton::s_jsonfilename << " to the new saved copy "
                       << savejsonfilename << ": " << strerror(errnocopy) << ", aborted...\n";
        return false;
    }

    // Original json config file saved successfully. Now, unlink it (the original):
    logstream << "Unlinking (removing) the original file " << ConfigSingleton::s_jsonfilename << " so that "
                   << "the new json file " << tempfilename << " can replace it.\n";
    if (::unlink(ConfigSingleton::s_jsonfilename.c_str()) != 0)
    {
        errnocopy = errno;
        logstream << "ERROR: UpdateJsonConfigFile() could not ::unlink() file " << ConfigSingleton::s_jsonfilename
                       << ": " << strerror(errnocopy) << "\n";
        return false;
    }

    // Now link the new json file - tempfilename - to the original filename:
    // Save the existing json file by linking it (hard link) to the now, non-existent permanent json file.
    logstream << "UpdateJsonConfigFile() linking (hard link) the new json file "
                   << tempfilename << " to the correct permanent location "
                   << ConfigSingleton::s_jsonfilename << ".\n";
    if (::link(tempfilename.c_str(), ConfigSingleton::s_jsonfilename.c_str()) != 0)
    {
        errnocopy = errno;
        logstream << "ERROR: UpdateJsonConfigFile() could not ::link() the new json file "
                       << tempfilename << " to the correct permanent location "
                       << ConfigSingleton::s_jsonfilename << ": " << strerror(errnocopy) << ", aborted...\n";
        return false;
    }

    // Check to make sure the new file is in place and has content.
    if (! checkFileIsNotEmpty(ConfigSingleton::s_jsonfilename))
    {
        errnocopy = errno;
        logstream << "ERROR: UpdateJsonConfigFile(): after update, found file "
                       << ConfigSingleton::s_jsonfilename << " is non-existent or empty. Aborted...\n";
        return false;
    }

    // Finally, unlink the temporary (new) json config file.
    ::unlink(tempfilename.c_str());

    return true;
}





