#pragma once

// COPYRIGHTS:
//
// Significant portions of this code were taken from the documentation provided by
// kernel.org in the "V4L2 API" section called "V4L2 video capture example" -
//
//          kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c.html
//
// - and modified (as of 3-10-2022). From the text provided there and from
// https://linuxtv.org/docs.php, the portions of code used can be used and distributed
// without restrictions.
//

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

#include <vidcap_capture_thread.hpp>
#include <vidcap_plugin_factory.hpp>
#include <MainLogger.hpp>
#include <Utility.hpp>
#include <condition_data.hpp>
#include <video_capture_globals.hpp>
#include <ConfigSingleton.hpp>
#include <vidcap_profiler_thread.hpp>
#include <vidcap_raw_queue_thread.hpp>

// Much of the VideoCapture::vidcap_v4l2_driver_interface object includes code
// used in the v4l2_capture.c source obtained from the linux.org documentation online.
// Most of the actual variable names and some C macros are still used in the object
// declared below, in order to help in future issues that are unknown at this time.
//
// Please see the "Reference" section (directory) in this project.  The README.md file
// shows the connection to the above mentioned C main program, and the C++ object declared
// below.

namespace VideoCapture
{
    #define CLEAR(x) memset(&(x), 0, sizeof(x))

    enum io_method {
            IO_METHOD_READ = 0,
            IO_METHOD_MMAP,
            IO_METHOD_USERPTR,
    };

    struct buffer {
            void   *start;
            size_t  length;
    };

    class vidcap_v4l2_driver_interface : virtual public video_plugin_base
    {
    public:
        vidcap_v4l2_driver_interface();
        virtual ~vidcap_v4l2_driver_interface() = default;


    public:
        ///////////////////////////////////////////////////////////////////////////
        // These functions are part of the plugin interface, declared in base class
        // video_plugin_base (in vidcap_capture_thread.hpp/.cpp.  See also the two
        // functions that are used by the plugin factory - create() and destroy()
        // defined at the very bottom of this file.
        ///////////////////////////////////////////////////////////////////////////

        virtual void initialize();
        virtual void run();

        virtual std::string get_type() const                { return video_plugin_base::plugin_type; }
        virtual std::string get_filename() const            { return video_plugin_base::plugin_filename; }

        // set_terminated() and get_interface_pointer() are not abstract, but are
        // part of the interface. The base class version is called explicitly from here.
        video_plugin_base* get_interface_pointer() const    { return video_plugin_base::interface_ptr; }
        void set_terminated(bool t)                         { video_plugin_base::set_terminated(t); };

        virtual bool probe_pixel_format_caps(std::map<std::string,std::string>& pixformat_map);
        virtual std::string get_popen_process_string()      { return video_plugin_base::popen_process_string; };
        virtual void start_streaming(int framecount = 0)    { video_plugin_base::base_start_streaming(framecount); };
        virtual bool isterminated(void)                     { return video_plugin_base::s_terminated; };
        virtual void set_error_terminated (bool t)          { video_plugin_base::s_errorterminated = t; set_terminated(t); };
        virtual bool iserror_terminated(void)               { return video_plugin_base::s_errorterminated; };
        virtual void set_paused(bool t)                     { video_plugin_base::set_base_paused(t); };
        virtual bool ispaused(void)                         { return video_plugin_base::is_base_paused(); };

        // These methods are not virtual: they use the base static function explicitly:
        void start_profiling()                              { video_plugin_base::start_profiling(); }
        long long increment_one_frame()                     { return video_plugin_base::increment_one_frame(); }
        void add_buffer_to_raw_queue(void *p, size_t bsize) { return video_plugin_base::add_buffer_to_raw_queue(p, bsize); }

        //////////////    End of mandatory plugin interface methods    //////////////////

        int  v4l2if_xioctl(int fh, int request, void *arg);
        void v4l2if_process_image(void *p, int size);
        bool v4l2if_read_frame(void);
        bool v4l2if_mainloop(void);
        bool v4l2if_stop_capturing(void);
        void v4l2if_cleanup_stop_capturing(void);       // ignores return values of system calls
        bool v4l2if_start_capturing(void);
        bool v4l2if_uninit_device(void);
        void v4l2if_cleanup_uninit_device(void);        // ignores return values of system calls
        bool v4l2if_init_read(unsigned int buffer_size);
        bool v4l2if_init_mmap(void);
        bool v4l2if_init_userp(unsigned int buffer_size);
        bool v4l2if_init_device(void);
        bool v4l2if_close_device(void);
        void v4l2if_cleanup_close_device(void);         // ignores return values of system calls
        bool v4l2if_open_device(void);

        void v4l2if_errno_exit(const char *s, int errnocopy);
        void v4l2if_error_exit(const char *s);
        void v4l2if_exit(const char *s);

    private:
        std::shared_ptr<Log::Logger> loggerp;           // Pointer to THE logger object in the main_thread

    private:
        enum io_method  io = IO_METHOD_MMAP;
        std::vector<const char *> string_io_methods ={ "IO_METHOD_READ", "IO_METHOD_MMAP", "IO_METHOD_USERPTR" };
        int             fd = -1;
        struct buffer   *buffers = NULL;
        unsigned int    numbufs = 0;
        bool            m_errorterminated = false;
    };

    ///////////////////////////////////////////////////////////////////////////
    // These two functions are declared/defined here and used by the plugin factory. They are
    // geared to be easily recognized by the plugin factory (vidcap_plugin_factory.hpp/.cpp)
    // and are called from the factory after the plugin is loaded with dlopen().
    //
    // SEE ALSO declarations in vidcap_plugin_factory.hpp.
    ///////////////////////////////////////////////////////////////////////////

    // the class factories
    extern "C" video_plugin_base* create() {
        return new vidcap_v4l2_driver_interface();
    }

    extern "C" void destroy(video_plugin_base* p) {
        if (p) delete p;
    }

} // end of namespace VideoCapture

