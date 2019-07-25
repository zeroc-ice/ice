// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

#if defined(__GNUC__) && !defined(__sun) && !defined(__FreeBSD__) && !defined(__MINGW32__) && \
    !defined(ICE_STATIC_LIBS)
#  define HAS_STACK_TRACES
#endif

#if defined(_WIN32) && !defined(ICE_OS_WINRT) && !defined(__MINGW32__)
#  define HAS_STACK_TRACES
#endif

namespace IceUtilInternal
{
extern bool ICE_UTIL_API printStackTraces;
}

#ifdef HAS_STACK_TRACES

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

#ifdef _WIN32
string
getIceHome()
{
    vector<wchar_t> buf(256);
    DWORD ret = GetEnvironmentVariableW(L"ICE_HOME", &buf[0], static_cast<DWORD>(buf.size()));
    string iceHome = (ret > 0 && ret < buf.size()) ? IceUtil::wstringToString(&buf[0]) : string("");
    if(!iceHome.empty())
    {
        return iceHome;
    }
    else
    {
        HKEY hKey;

        string key = string("SOFTWARE\\ZeroC\\Ice ") + ICE_STRING_VERSION;
        const wstring keyName = IceUtil::stringToWstring(key);

        if(RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyName.c_str(), 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        {
            return "";
        }

        WCHAR buf[512];
        DWORD bufSize = sizeof(buf);
        if(RegQueryValueExW(hKey, L"InstallDir", 0, NULL, (LPBYTE)buf, &bufSize) != ERROR_SUCCESS)
        {
            return "";
        }
        return IceUtil::wstringToString(wstring(buf));
    }
}
#endif

#if defined(__APPLE__)
void
standardizeVersion(string& str)
{
    string v1 = ICE_STRING_VERSION;
    string v2("36");
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

#endif

int main(int argc, char* argv[])
{
#ifdef HAS_STACK_TRACES

    bool optimized = false;
#ifdef NDEBUG
    optimized = true;
#endif

#if defined(_WIN32)
    bool binDist = false;
    vector<wchar_t> buf(256);
    DWORD ret = GetEnvironmentVariableW(L"USE_BIN_DIST", &buf[0], static_cast<DWORD>(buf.size()));
    string valstr = (ret > 0 && ret < buf.size()) ? IceUtil::wstringToString(&buf[0]) : string("");
    binDist = valstr == "yes";

    if(binDist)
    {
        //
        // For Windows we only run the test against bindist if PDBs were installed
        //
        string pdb = getIceHome() + "\\bin\\icebox.pdb";
        if(!ifstream(pdb))
        {
            cout << "Test requires PDBs to be installed" << endl;
            return EXIT_SUCCESS;
        }
    }
    else if(optimized)
    {
        //
        // Only support debug srcdist Windows builds
        //
        return EXIT_SUCCESS;
    }
#endif

    cout << "checking stacktrace... ";

    IceUtilInternal::printStackTraces = true;

    string filename = "StackTrace.";

#if defined(__APPLE__)
    bool binDist = false;
    const char* s = getenv("USE_BIN_DIST");
    if(s && *s != '\0')
    {
       binDist = string(s) == "yes";
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

#if defined(_WIN32)
    filename += ".Win32";
#elif defined(__APPLE__)
    filename += ".macOS";
#else
    filename += ".Linux";
#endif

    while(true)
    {
#if defined(_WIN32) && defined(NDEBUG)
        bool match = true;
#endif
        ifstream ifs(filename.c_str());
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
            test(expected.size() <= actual.size());
            for(size_t i = 0; i < expected.size(); ++i)
            {
                if(actual[i].find(expected[i]) == string::npos)
                {
#if defined(_WIN32) && defined(NDEBUG)
                    match = false;
                    //
                    // With windows optimized builds retry with the alternate
                    // expect file.
                    //
                    if(filename != "StackTrace.release.Win32")
                    {
                        break;
                    }
                    else
                    {
                        test(false);
                    }
#else
                    test(false);
#endif
                }
            }

#if defined(_WIN32) && defined(NDEBUG)
            if(!match && filename != "StackTrace.release.Win32")
            {
                filename = "StackTrace.release.Win32";
                continue;
            }
#endif
            break;
#endif
        }
    }
    cout << "ok" << endl;
#else
    cout << "Test not supported on this platform" << endl;
#endif

    return EXIT_SUCCESS;
}
