// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/RubyUtil.h>
#include <Slice/Util.h>
#include <IceUtil/ConsoleUtil.h>

using namespace std;
using namespace Slice;
using namespace Slice::Ruby;
using namespace IceUtilInternal;

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return Slice::Ruby::compile(args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << argv[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(const std::string& msg)
    {
        consoleErr << argv[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(const char* msg)
    {
        consoleErr << argv[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        consoleErr << argv[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
