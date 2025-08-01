// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "PythonUtil.h"

using namespace std;
using namespace IceInternal;

#ifdef _WIN32
int
wmain(int argc, wchar_t* argv[])
#else
int
main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    const string programName = args[0];
    try
    {
        return Slice::Python::compile(args);
    }
    catch (const std::exception& ex)
    {
        consoleErr << programName << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        consoleErr << programName << ": error:unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
