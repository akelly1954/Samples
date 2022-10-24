#include <commandline.hpp>
#include <algorithm>

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

////////////////////////////////////////////////
// class Util::CommandLine members
////////////////////////////////////////////////

void Util::CommandLine::parseCommandLine(void)
{
    std::string argv0 = m_strArgv[0];
    std::string argv1 = (m_strArgv.size() == 1? "-h" : m_strArgv[1].c_str());

    if (argv1 == "--help" || argv1 == "-h" || argv1 == "-help" || argv1 == "help")
    {
        m_isHelp = true;
        return;
    }

    int i = 1;
    do
    {
        if (i >= m_argc) break;

        std::string currentArg = m_strArgv[i];

        if (currentArg.length() == 0)
        {
            i++;
            continue;  // ignore weird empty strings
        }
        else if (std::find(m_allowedFlags.begin(), m_allowedFlags.end(), currentArg) != m_allowedFlags.end())
        {
            // This is a new flag
            if (i < m_argc-1)
            {
                std::string nextarg = m_strArgv[i+1];

                // Is the next arg another flag?
                if (std::find(m_allowedFlags.begin(), m_allowedFlags.end(), nextarg) == m_allowedFlags.end())
                {
                    if (nextarg[0] == '-')
                    {
                        // is this a flag or a value?
                        if (std::find(m_allowedFlags.begin(), m_allowedFlags.end(), nextarg) == m_allowedFlags.end())
                        {
                            // not a flag, so it's the parameter provided for the previous flag.
                            m_cmdMap[currentArg] = nextarg;
                            i++;
                        }
                        else
                        {
                            setError( std::string("ERROR (1): Unrecognized flag used: ") + nextarg );
                            return;
                        }
                    }
                    else
                    {
                        // This is the parameter provided for the previous flag.
                        m_cmdMap[currentArg] = nextarg;
                        i++;
                    }
                }
                else
                {
                    // This is a flag with no parameter
                    m_cmdMap[currentArg] = "";  // i is not incremented because no parameter there.
                }
            }
            else
            {
                // This is a flag at the last parameter on the command line.
                // There is no parameter
                m_cmdMap[currentArg] = "";
                break;
            }
        }
        else
        {
            // Current flag is not recognized
            if (currentArg[0] == '-')
            {
                setError( std::string("ERROR (2): Unrecognized flag used: ") + currentArg );
                return;
            }
            else
            {
                setError( std::string("ERROR (3): Unknown command line parameter used: ") + currentArg );
                return;
            }
        }
        i++;
    } while (i < m_argc);
}

