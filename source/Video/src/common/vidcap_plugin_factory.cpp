
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

// Static data

void* VideoCapture::video_capture_plugin_factory::s_plugin_handle = nullptr;
VideoCapture::video_plugin_base* VideoCapture::video_capture_plugin_factory::s_vplugin_handle = nullptr;
VideoCapture::destroy_t* VideoCapture::video_capture_plugin_factory::s_destroy_function_handle = nullptr;

VideoCapture::video_plugin_base*
VideoCapture::video_capture_plugin_factory::create_factory(std::ostream& ostrm)
{
    using namespace Video;
    Json::Value& Root = Config::ConfigSingleton::instance()->JsonRoot();

    // First get the interface type and plugin filename from the JSON file
    vcGlobals::video_grabber_name = Root["Config"]["Video"]["preferred-interface"].asString();
    vcGlobals::str_plugin_file_name = Root["Config"]
                                              ["Video"]
                                                  ["frame-capture"]
                                                      [vcGlobals::video_grabber_name]
                                                          ["plugin-file-name"].asString();

    // load the plugin library (the json config singleton has to
    // have been created before this).

    ostrm << "Plugin Factory: Starting video_capture_factory()\n";

    const char* dlsym_error = dlerror();    // clear out prior errors
    s_plugin_handle = dlopen(vcGlobals::str_plugin_file_name.c_str(), RTLD_LAZY);     // (RTLD_NOW | RTLD_GLOBAL));
    if (!s_plugin_handle) {
        dlsym_error = dlerror();
        ostrm << "Plugin Factory: Cannot load plugin library " << vcGlobals::str_plugin_file_name << ": " << dlsym_error << "\n";
        return nullptr;
    }
    ostrm << "Plugin Factory: dlopen(\"" << vcGlobals::str_plugin_file_name << "\"): SUCCESS\n";
    dlerror();      // reset errors

    // load the symbols

    ////////////////////////////
    // create()
    ////////////////////////////
    create_t* create_plugin = (create_t*) dlsym(s_plugin_handle, "create");
    dlsym_error = dlerror();
    if (dlsym_error) {
        ostrm << "Plugin Factory: Cannot find symbol \"create\" in loaded plugin library " << vcGlobals::str_plugin_file_name << ": " << dlsym_error << "\n";
        return nullptr;
    }
    ostrm << "Plugin Factory: Find create() in " << vcGlobals::str_plugin_file_name << ": SUCCESS\n";
    dlerror();      // reset errors

    ////////////////////////////
    // destroy()
    ////////////////////////////
    s_destroy_function_handle = (destroy_t*) dlsym(s_plugin_handle, "destroy");
    dlsym_error = dlerror();
    if (dlsym_error) {
        ostrm << "Plugin Factory: Cannot find symbol \"destroy\" in loaded plugin library " << vcGlobals::str_plugin_file_name << ": " << dlsym_error << "\n";
        return nullptr;
    }
    ostrm << "Plugin Factory: Find destroy() in " << vcGlobals::str_plugin_file_name << ": SUCCESS" << "\n";

    // create an instance of the class
    ostrm << "Plugin Factory: Creating plugin" << "\n";
    video_plugin_base* s_vplugin_handle = create_plugin();

    // All of the operations below serve as a sanity test on the plugin's conformance
    // to the plugin policy.
    s_vplugin_handle->set_plugin_type(vcGlobals::video_grabber_name);
    s_vplugin_handle->set_plugin_filename(vcGlobals::str_plugin_file_name);
    s_vplugin_handle->set_plugin_interface_pointer(s_vplugin_handle);

    std::cerr << "Plugin Factory: The plugin type is: " << s_vplugin_handle->get_type() << std::endl;
    ostrm << "Plugin Factory: The plugin type is: " << s_vplugin_handle->get_type() << "\n";
    ostrm << "Plugin Factory: The plugin filename is: " << s_vplugin_handle->get_filename() << "\n";
    ostrm << "Plugin Factory: The plugin interface ptr is: " << s_vplugin_handle->get_interface_pointer() << "\n";

    // Get the pixel formats supported by this plugin:
    s_vplugin_handle->probe_pixel_format_caps(Video::pixel_format::s_pixformat);

    Video::pixel_format::pixfmt_setup();
    ostrm << "Plugin Factory: ";
    Video::pixel_format::displayPixelFormatConfig(ostrm);

    // Set the static interface pointer (see vidcap_plugin_factory.hpp/.cpp
    VideoCapture::video_plugin_base::interface_ptr = s_vplugin_handle;

    return s_vplugin_handle;
}

void VideoCapture::video_capture_plugin_factory::destroy_factory(std::ostream& ostrm)
{
    // TODO: BUG: change to false - Currently dlclose() sometimes seems to hang at the end
    //            of the program after all threads unwind. This does affect functionality.
    // The "true" value is currently preventing dlclose from being called.
    // When fixed, the done_already status should be changed to false.
    // static bool done_already = false;
    static bool done_already = true;

    if (done_already) return;

    ostrm << "Plugin Factory: Destroying the loaded " << Video::vcGlobals::str_plugin_file_name << " plugin\n";

    // destroy the class
    s_destroy_function_handle(s_vplugin_handle);

    // unload the plugin library
    ostrm << "video_capture_plugin_factory::destroy_factory():  dlclose()'ing the plugin\n";
    dlclose(s_plugin_handle);
    done_already = true;
}

