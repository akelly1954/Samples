
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

#include <JsonCppUtil.hpp>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>

void UtilJsonCpp::indent(std::ostream& strm, int numspaces)
{
    static bool initialized = false;
    static char spaceline[256];
    static std::string spaces;

    if (!initialized)
    {
        // Set up a long line of spaces so that std::string.substr() can use it below.
        ::memset(&spaceline[0], ' ', sizeof(spaceline)-1);
        spaceline[sizeof(spaceline)-1] = '\0';
        initialized = true;
        spaces = const_cast<const char *>(&spaceline[0]);
    }

    strm << spaces.substr(0, numspaces);
}

void UtilJsonCpp::streamroot(std::ostream& strm, Json::Value& root)
{
    Json::Value::iterator itr;
    std::string name;
    static int idnt = 0;

    idnt += 4;  // this function is recursive

    // DEBUG:
    // strm << "\n-----------------------------------------------------\n";
    // strm << "Got:\n" << root << "\n";
    // strm << "\n-----------------------------------------------------\n\n";

    for (itr = root.begin(); itr != root.end(); itr++)
    {
        name = itr.key().asString();
        strm << "---- key string is:" << name << "\n";
        strm << "---- has member names? " << (UtilJsonCpp::checkJsonValueHasGetMemberNames(itr)? "YES": "NO") << "\n";
        strm << "---- object type: " << (UtilJsonCpp::ValueJsonTypeString(itr)) << "\n";

        if ((*itr).isInt())
        {
            indent(strm, idnt); strm << "member: " << name << ", type: int, value=" << (*itr).asInt() << "\n";
        }
        else if ((*itr).isDouble())
        {
            indent(strm, idnt); strm << "member: " << name << ", type: double, value="
                      << (*itr).asDouble() << "\n";
        }
        else if ((*itr).isBool())
        {
            indent(strm, idnt); strm << "member: " << name << ", type: bool, value="
                      << (*itr).asBool() << "\n";
        }
        else if ((*itr).isArray())
        {
            Json::Value::iterator innerItr = (*itr).begin();

            int size = (*itr).size();
            indent(strm, idnt); strm << "member: " << name << ", array size: " << size << ", type: object, value = " << "\n";

            for (innerItr = (*itr).begin(); innerItr != (*itr).end(); innerItr++)
            {
                if (itr->type() == Json::ValueType::arrayValue)
                {
                    indent(strm, idnt+4); strm << "array member[" << innerItr.key().asString() << "] = "
                                                         << (*innerItr).asString() << "\n";
                }
                else
                {
                    UtilJsonCpp::streamroot(strm,(*innerItr));
                    strm << "\n"; idnt -= 4;
                }
            }
        }
        else if ((*itr).isInt64())
        {
            indent(strm, idnt); strm << "member: " << name << ", type: int64, value=" << (*itr).asInt64() << "\n";
        }
        else if ((*itr).isString())
        {
            indent(strm, idnt); strm << "member: " << name << ", type: string, value=" << (*itr).asString() << "\n";
        }
        // TODO: Not sure about this.
        // else if ((*itr).type() == Json::ValueType::arrayValue)
        // {
        //    strm << itr.key().asString() << "\n";
        // }
        else if ((*itr).isObject())
        {
            indent(strm, idnt); strm << "member: " << name << ", type: object, values:" << "\n";
            UtilJsonCpp::streamroot(strm,(*itr));
            strm << "\n"; idnt -= 4;
        }
        else
        {
            indent(strm, idnt); strm << "member: " << name << ", type: unknown type " << "\n";
        }
    }
}

int UtilJsonCpp::checkjsonsyntax(std::ostream& logstream, std::istream& istrm, Json::Value& root)
{
    if (istrm.bad()|| istrm.eof())
    {
        // JsonCpp does not check this, but will fail with a syntax error on the first read
        throw std::runtime_error("Error initializing ConfigSingleton object: json stream is bad or at eof.\n");
        // and exit()
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    JSONCPP_STRING errs;
    if (! parseFromStream(builder, istrm, &root, &errs)) {
        logstream << "JsonCpp parse errors: " << errs << "\n";
        return EXIT_FAILURE;
    }

    logstream << "\n\nCHECKING JSON FILE\n" << "\n";
    UtilJsonCpp::streamroot(logstream, root);
    return EXIT_SUCCESS;
}

// All Json Value objects can call getMemberNames() except nullptr and object
bool UtilJsonCpp::checkJsonValueHasGetMemberNames (const Json::Value::iterator &val_iterator)
{
    return (*val_iterator).type() == Json::ValueType::nullValue || (*val_iterator).type() == Json::ValueType::objectValue;
}

std::string UtilJsonCpp::ValueJsonTypeString(const Json::Value::iterator &val_iterator)
{
    std::string tname;

    // From the JsonCpp source code:
    //
    //    enum ValueType
    //    {
    //       nullValue = 0,
    //       intValue,
    //       uintValue,
    //       realValue,
    //       stringValue,
    //       booleanValue,
    //       arrayValue,
    //       objectValue
    //    };
    //

    switch((*val_iterator).type())
    {
        case Json::ValueType::nullValue:
            tname = "nullValue";
            break;
        case Json::ValueType::intValue:
            tname = "intValue";
            break;
        case Json::ValueType::uintValue:
            tname = "uintValue";
            break;
        case Json::ValueType::realValue:
            tname = "realValue";
            break;
        case Json::ValueType::stringValue:
            tname = "stringValue";
            break;
        case Json::ValueType::booleanValue:
            tname = "booleanValue";
            break;
        case Json::ValueType::arrayValue:
            tname = "arrayValue";
            break;
        case Json::ValueType::objectValue:
            tname = "objectValue";
            break;
        default:
            tname = "unknown type";
            break;
    }

    return tname;
}
