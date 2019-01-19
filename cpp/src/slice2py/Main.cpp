//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Slice/PythonUtil.h>
#include <Slice/Util.h>
#include <IceUtil/ConsoleUtil.h>

using namespace std;
using namespace Slice;
using namespace Slice::Python;
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
        return Slice::Python::compile(args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        consoleErr << args[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
