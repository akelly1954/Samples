#include "commandline.hpp"
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

    if (m_strArgv.size() == 1 &&
       (argv1 == "--help" || argv1 == "-h" || argv1 == "-help" || argv1 == "help") )
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


////////////////////////////////////////////////
// Free standing functions in the Util namespace
////////////////////////////////////////////////

std::string Util::parseGetHelp(Util::Command_Map cmdmap)
{
    std::string retstring;
    auto itr = cmdmap.find("help");
    if (itr != cmdmap.end())
    {
        retstring = itr->second;
    }
    return retstring;
}

std::string Util::parseGetError(Util::Command_Map cmdmap)
{
    std::string retstring;
    auto itr = cmdmap.find("error");
    if (itr != cmdmap.end())
    {
        retstring = itr->second;
    }
    return retstring;
}

Util::Command_Map Util::parseSetup(int argc, const char *argv[], const std::vector<std::string>& allowedFlags)
{
    Util::Command_Map cmdmap;
    Util::Command_Map errormap;

    std::string argv0 = argv[0];
    std::string argv1 = (argv[1] == nullptr? "-h" : argv[1]);

    if (argc > 1 && (argv1 == "--help" || argv1 == "-h" || argv1 == "help"))
    {
        cmdmap["help"] = "help";
        return cmdmap;
    }

    int i = 1;
    do
    {
        if (i >= argc) break;

        std::string currentArg = argv[i];

        if (currentArg.length() == 0)
        {
            i++;
            continue;  // ignore weird empty strings
        }
        else if (std::find(allowedFlags.begin(), allowedFlags.end(), currentArg) != allowedFlags.end())
        {
            // This is a new flag
            if (i < argc-1)
            {
                std::string nextarg = argv[i+1];

                // Is the next arg another flag?
                if (std::find(allowedFlags.begin(), allowedFlags.end(), nextarg) == allowedFlags.end())
                {
                    if (nextarg[0] == '-')
                    {
                        errormap["error"] = std::string("ERROR (1): Unrecognized flag used: ") + nextarg;
                        return errormap;
                    }
                    else
                    {
                        // This is the parameter provided for the previous flag.
                        cmdmap[currentArg] = nextarg;
                        i++;
                    }
                }
                else
                {
                    // This is a flag with no parameter
                    cmdmap[currentArg] = "";  // i is not incremented because no parameter there.
                }
            }
            else
            {
                // This is a flag at the last parameter on the command line.
                // There is no parameter
                cmdmap[currentArg] = "";
                break;
            }
        }
        else
        {
            // Current flag is not recognized
            if (currentArg[0] == '-')
            {
                errormap["error"] = std::string("ERROR (2): Unrecognized flag used: ") + currentArg;
                return errormap;
            }
            else
            {
                errormap["error"] = std::string("ERROR (3): Unknown command line parameter used: ") + currentArg;
                return errormap;
            }
        }
        i++;
    } while (i < argc);

    return cmdmap;
}

Util::Command_Map Util::getCLMap(int argc, const char *argv[], const std::vector<std::string>& allowedFlags)
{
    Util::Command_Map cmdmap;

    int i = 1;
    do
    {
        if (i >= argc) break;

        std::string currentArg = const_cast<const char *>(argv[i]);

        if (currentArg.length() == 0)
        {
            continue;  // ignore weird empty strings
        }
        else if (std::find(allowedFlags.begin(), allowedFlags.end(), currentArg) != allowedFlags.end())
        {
            // This is a new flag
            if (i < argc-1)
            {
                std::string nextarg = const_cast<const char *>(argv[i+1]);
                if (std::find(allowedFlags.begin(), allowedFlags.end(), nextarg) == allowedFlags.end()) // Is the next arg another flag?
                {
                    cmdmap[currentArg] = nextarg;
                    i++;
                }
                else
                {
                    // This is a flag with no parameter
                    cmdmap[currentArg] = "";  // i is not incremented because no parameter there.
                }
            }
            else
            {
                // This is a flag at the last parameter on the command line.
                // There is no parameter
                cmdmap[currentArg] = "";
                break;
            }
        }
        i++;
    } while (i < argc);
    return cmdmap;
}


Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, unsigned short& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, bool& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, int& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, long& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, unsigned long& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, long long& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, std::string& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, float& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, double& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const Util::Command_Map& cmdmap, std::string flag, long double& var)
{
    return get_template_arg(cmdmap, flag, var);
}
