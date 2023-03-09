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

#include <parse_tools.hpp>
#include <video_capture_commandline.hpp>
#include <video_capture_globals.hpp>
#include <iostream>


bool Video::initial_commandline_parse(Util::CommandLine& cmdline, int argc, std::string argv0, std::ostream& strm)
{
    if(cmdline.isError())
    {
        strm << "\n" << argv0 << ": " << cmdline.getErrorString() << "\n";
        VidCapCommandLine::Usage(strm, argv0);
        return false;
    }

    if(cmdline.isHelp())
    {
        if (argc > 2)
        {
            strm << "\nWARNING: using the --help flag cancels all other flags and parameters.  Exiting...\n";
        }
        VidCapCommandLine::Usage(strm, argv0);
        return false;
    }
    return true;
}

