// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringUtil.h>
#include <TestHelper.h>

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

vector<string>
splitLines(const string& str)
{
    vector<string> result;
    istringstream is(str);
    string line;
    while(std::getline(is, line))
    {
#ifdef __APPLE__
        //
        // Remove patch number from stack trace
        //
        string v1 = ICE_STRING_VERSION;
        size_t pos = line.find(v1);
        if(pos != string::npos)
        {
            vector<string> split;
            IceUtilInternal::splitString(v1, ".", split);
            string v2(split[0] + split[1]);
            line.replace(pos, v1.length(), v2);
        }
#endif

        result.push_back(line);
    }
    return result;
}

}

class Client : public Test::TestHelper
{
public:

    virtual void run(int argc, char* argv[]);
};

void
Client::run(int argc, char* argv[])
{
    if(IceUtilInternal::stackTraceImpl() == IceUtilInternal::STNone)
    {
        cout << "This Ice build cannot capture stack traces" << endl;
        return;
    }

    bool optimized = false;
#ifdef NDEBUG
    optimized = true;
#endif

    cout << "checking stacktrace... ";

    IceUtilInternal::printStackTraces = true;

    string filename = "StackTrace.";

    if(optimized)
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
#elif defined(__apple_build_version__)
#   if(__apple_build_version__ >= 10001145)
        filename += "-xcode10";
    #else
        filename += "-xcode9";
    #endif
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
    filename += ".macOS";
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
        bool match = true;
        ifstream ifs(filename.c_str());

        if(!ifs)
        {
            ostringstream os;
            os << "cannot open `" << filename << "`, failed!";
            cout << os.str() << endl;
            throw invalid_argument(os.str());
        }

        // Show which template we use:
        cout << filename << "... ";

        stringstream sstr;
        sstr << ifs.rdbuf();
        vector<string> expected = splitLines(sstr.str());

        ThrowerPtr thrower = new Thrower();
        try
        {
            thrower->first();
        }
        catch(const IceUtil::Exception& ex)
        {
            string stack = ex.ice_stackTrace();

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
                    if(filename.find("StackTrace.release-vc") == 0 || filename == "StackTrace.release.Linux")
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
        }
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client);
