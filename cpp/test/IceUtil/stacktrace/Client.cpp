// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringUtil.h>
#include <TestCommon.h>

#include <fstream>

using namespace IceUtil;
using namespace std;


namespace IceUtilInternal
{
extern bool ICE_API printStackTraces;
}

namespace
{
class Thrower : public IceUtil::Shared
{
public:

    Thrower() : _idx(0)
    {
    }

    void first()
    {
        _idx++;
        second();
    }

    void second()
    {
        _idx++;
        third();
    }

    void third()
    {
        _idx++;
        forth();
    }

    void forth()
    {
        _idx++;
        fifth();
    }

    void fifth()
    {
        _idx++;
        throw IceUtil::NullHandleException(__FILE__, __LINE__);
    }

private:

    int _idx;
};
typedef IceUtil::Handle<Thrower> ThrowerPtr;

#if defined(__APPLE__)
void
standardizeVersion(string& str)
{
    string v1(ICE_STRING_VERSION);

    vector<string> split;
    IceUtilInternal::splitString(v1, ".", split);
    string v2(split[0] + split[1]);

    size_t pos = 0;
    while((pos = str.find(v1, pos)) != string::npos)
    {
        str.replace(pos, v1.length(), v2);
        pos += v2.length();
    }
}
#else
vector<string>
splitLines(const string& str)
{
    vector<string> result;
    istringstream is(str);
    string line;
    while(std::getline(is, line))
    {
        result.push_back(line);
    };
    return result;
}
#endif
}

int main(int argc, char* argv[])
{
    if(IceUtilInternal::stackTraceImpl() == IceUtilInternal::STNone)
    {
        cout << "This Ice build cannot capture stack traces" << endl;
        return EXIT_SUCCESS;
    }

    bool optimized = false;
#ifdef NDEBUG
    optimized = true;
#endif

    cout << "checking stacktrace... ";

    IceUtilInternal::printStackTraces = true;

    string filename = "StackTrace.";

#if defined(__APPLE__)
    bool binDist = false;
    const char* s = getenv("ICE_BIN_DIST");
    if(s && *s != '\0')
    {
       binDist = (string(s).find("all") != std::string::npos) || (string(s).find("cpp") != std::string::npos);
    }

    if(binDist && !optimized)
    {
        filename += "debug-release";
    }
    else if(optimized)
#else
    if(optimized)
#endif
    {
        filename += "release";
#if defined(_MSC_VER)
#   if(_MSC_VER == 1700)
        filename += "-vc110";
#   elif(_MSC_VER == 1800)
        filename += "-vc120";
#   elif(_MSC_VER == 1900)
        filename += "-vc140";
#    elif(_MSC_VER >= 1910)
        filename += "-vc141";
#   endif
#endif
    }
    else
    {
        filename += "debug";
    }

#if defined(_WIN64)
     filename += ".Win64";
#elif defined(_WIN32)
    filename += ".Win32";
#elif defined(__APPLE__)
    filename += ".OSX";
#else
    filename += ".Linux";

    if(!optimized && IceUtilInternal::stackTraceImpl() == IceUtilInternal::STLibbacktracePlus)
    {
        // Libbacktrace with GCC 4.8 and pie return a smaller backtrace
#   if defined(__pie__) && defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ == 408)
        filename += ".libbacktrace+48pie";
#   else
        filename += ".libbacktrace+";
#   endif
    }

#endif

    while(true)
    {
#ifndef __APPLE__
        bool match = true;
#endif
        ifstream ifs(filename.c_str());

        if(!ifs)
        {
            cout << "cannot open `" << filename << "`, failed!" << endl;
            return EXIT_FAILURE;
        }

        // Show which template we use:
        cout << filename << "... ";

        stringstream sstr;
        sstr << ifs.rdbuf();
#if defined(__APPLE__)
        string expected = sstr.str();
        standardizeVersion(expected);
#else
        vector<string> expected = splitLines(sstr.str());
#endif

        ThrowerPtr thrower = new Thrower();
        try
        {
            thrower->first();
        }
        catch(const IceUtil::Exception& ex)
        {
            string stack = ex.ice_stackTrace();
            // cerr << "\n full stack trace is \n" << stack << endl;

#ifdef __APPLE__
            standardizeVersion(stack);
            if(expected.size() < stack.size())
            {
                test(stack.compare(0, expected.size(), expected) == 0);
            }
            else
            {
                test(stack == expected);
            }
            break;
#else
            vector<string> actual = splitLines(stack);
            for(size_t i = 0; i < expected.size(); ++i)
            {
                if(actual[i].find(expected[i]) == string::npos)
                {
                    match = false;
                    //
                    // With windows and Linux optimized builds retry with the alternate
                    // files.
                    //
                    if(filename.find("StackTrace.release-vc") == 0 ||
                       filename == "StackTrace.release.Linux")
                    {
                        break;
                    }


                    cerr << "could not find `" << expected[i] << "` in " << actual[i] << endl;
                    cerr << "Full stack is:\n" << stack << endl;
                    test(false);
                }
            }


            if(!match)
            {
                if(filename.find("StackTrace.release-vc") == 0)
                {
                    // Retry with alternate stack
                    filename = "StackTrace.release";
#if defined(_WIN64)
                    filename += ".Win64";
#elif defined(_WIN32)
                    filename += ".Win32";
#endif
                    continue;
                }
                else if(filename == "StackTrace.release.Linux")
                {
                    // Retry with the debug Linux stack
                    filename = "StackTrace.debug.Linux";
                    continue;
                }
            }
            test(actual.size() >= expected.size());
            break;
#endif
        }
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}
