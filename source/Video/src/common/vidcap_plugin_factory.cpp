
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

/////////////////////////////////////////////////////////////////
// NOTE: This is a different thread to the main thread.
/////////////////////////////////////////////////////////////////

#include <LoggerCpp/LoggerCpp.h>
#include <vidcap_plugin_factory.hpp>
#include <iostream>
#include <dlfcn.h>
#include <unistd.h>

const char *v4l2_plugin_so = "libVideoPlugin_V4L2.so";

// TODO: XXX   void VideoCapture::video_capture_factory(Log::Logger logger)
void video_capture_factory(Log::Logger logger)
{
    // TODO: XXX  using namespace VideoCapture;
::sleep(4);
logger.info() << "XXXX Starting video_capture_factory()";
    // load the plugin library
    const char* dlsym_error = nullptr;
    void* v4l2plugin_handle = dlopen(v4l2_plugin_so, RTLD_LAZY);                 // (RTLD_NOW | RTLD_GLOBAL));    // TODO: XXX     , RTLD_LAZY);
    if (!v4l2plugin_handle) {
        dlsym_error = dlerror();
        std::cerr << "Cannot load plugin library " << v4l2_plugin_so << ": " << dlsym_error << std::endl;
        logger.info() << "XXXX Cannot load plugin library: " << dlsym_error;
        return;
    }

    // reset errors
    dlerror();

    // load the symbols
    create_t* create_plugin = (create_t*) dlsym(v4l2plugin_handle, "create");
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load plugin library " << v4l2_plugin_so << ": " << dlsym_error << std::endl;
        logger.info() << "Cannot load symbol create: " << dlsym_error;
        return;
    }

    destroy_t* destroy_plugin = (destroy_t*) dlsym(v4l2plugin_handle, "destroy");
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol destroy: " << dlsym_error << std::endl;
        logger.info() << "Cannot load symbol destroy: " << dlsym_error;
        return;
    }

    logger.info() << "XXXX Creating plugin";
    // create an instance of the class
    video_plugin_base* vplugin = create_plugin(logger);

    // use the class
    logger.info() << "XXXX Using the plugin";
    vplugin->set_plugin_type("v4l2");
    std::cout << "The plugin type is: " << vplugin->get_type() << std::endl;
    logger.info() << "The plugin type is: " << vplugin->get_type();

    logger.info() << "XXXX Destroying the plugin";

    // destroy the class
    destroy_plugin(vplugin);

    logger.info() << "XXXX dlclose()'ing the plugin";
    // unload the triangle library
    ::sleep(4);
    dlclose(v4l2plugin_handle);
}

