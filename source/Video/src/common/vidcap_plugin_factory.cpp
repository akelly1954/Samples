
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

#include <vidcap_plugin_factory.hpp>
#include <iostream>
#include <dlfcn.h>
#include <unistd.h>

std::string VideoCapture::video_plugin_base::plugin_type = "undefined";
std::string VideoCapture::video_plugin_base::plugin_filename;
VideoCapture::video_plugin_base* VideoCapture::video_plugin_base::interface_ptr = nullptr;

bool VideoCapture::video_plugin_base::s_terminated = false;
bool VideoCapture::video_plugin_base::s_errorterminated = false;
bool VideoCapture::video_plugin_base::s_paused = false;

const char *v4l2_plugin_so = "libVideoPlugin_V4L2.so";

VideoCapture::video_plugin_base* VideoCapture::video_capture_factory(std::ostream& ostrm)
{
    // load the plugin library
    ostrm << "Plugin Factory: Starting video_capture_factory()\n";

    const char* dlsym_error = nullptr;
    void* v4l2plugin_handle = dlopen(v4l2_plugin_so, RTLD_LAZY);     // (RTLD_NOW | RTLD_GLOBAL));
    if (!v4l2plugin_handle) {
        dlsym_error = dlerror();
        ostrm << "Plugin Factory: Cannot load plugin library " << v4l2_plugin_so << ": " << dlsym_error << "\n";
        return nullptr;
    }
    ostrm << "Plugin Factory: dlopen(\"" << v4l2_plugin_so << "\"): SUCCESS\n";
    dlerror();      // reset errors

    // load the symbols
    create_t* create_plugin = (create_t*) dlsym(v4l2plugin_handle, "create");
    dlsym_error = dlerror();
    if (dlsym_error) {
        ostrm << "Plugin Factory: Cannot find symbol \"create\" in loaded plugin library " << v4l2_plugin_so << ": " << dlsym_error << "\n";
        return nullptr;
    }

    ostrm << "Plugin Factory: Find create() in " << v4l2_plugin_so << ": SUCCESS\n";
    dlerror();      // reset errors

    destroy_t* destroy_plugin = (destroy_t*) dlsym(v4l2plugin_handle, "destroy");
    dlsym_error = dlerror();
    if (dlsym_error) {
        ostrm << "Plugin Factory: Cannot find symbol \"destroy\" in loaded plugin library " << v4l2_plugin_so << ": " << dlsym_error << "\n";
        return nullptr;
    }
    ostrm << "Plugin Factory: Find destroy() in " << v4l2_plugin_so << ": SUCCESS" << "\n";

    // create an instance of the class
    ostrm << "Plugin Factory: Creating plugin" << "\n";
    video_plugin_base* vplugin = create_plugin();

    // use the class
    ostrm << "Plugin Factory: Updating the " << v4l2_plugin_so << " plugin with type/interface information." << "\n";

    vplugin->set_plugin_type("v4l2");
    vplugin->set_plugin_filename(v4l2_plugin_so);
    vplugin->set_plugin_interface_pointer(vplugin);

    std::cout << "Plugin Factory: The plugin type is: " << vplugin->get_type() << std::endl;
    ostrm << "Plugin Factory: The plugin type is: " << vplugin->get_type() << "\n";
    ostrm << "Plugin Factory: The plugin filename is: " << vplugin->get_filename() << "\n";
    ostrm << "Plugin Factory: The plugin interface ptr is: " << vplugin->get_interface_pointer() << "\n";

    // Set the static interface pointer (see vidcap_plugin_factory.hpp/.cpp
    VideoCapture::video_plugin_base::interface_ptr = vplugin;

    // TODO: XXX ostrm << "Plugin Factory: Destroying the loaded " << v4l2_plugin_so << " plugin\n";

    // destroy the class
    // TODO: XXX destroy_plugin(vplugin);

    // unload the triangle library
    // TODO: XXX ostrm << "Plugin Factory: dlclose()'ing the plugin\n";
    // TODO: XXX dlclose(v4l2plugin_handle);

    return vplugin;
}

