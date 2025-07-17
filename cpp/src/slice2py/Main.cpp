// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Slice/Util.h"
#include "PythonUtil.h"

using namespace std;
using namespace Slice;
using namespace Slice::Python;
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
    try
    {
        return Slice::Python::staticCompile(args);
    }
    catch (const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        consoleErr << args[0] << ": error:unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
