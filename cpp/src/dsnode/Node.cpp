// Copyright (c) ZeroC, Inc.

#include "DataStorm/DataStorm.h"

#include <iostream>

using namespace std;

static void
usage(const string& n)
{
    cerr << "Usage: " << n << " [options]\n";
    cerr << "Options:\n"
            "-h, --help               Show this message.\n"
            "-v, --version            Display the DataStorm version.\n";
}

int
main(int argc, char* argv[])
{
    try
    {
        // Parse arguments.
        for (int i = 0; i < argc; ++i)
        {
            string arg = argv[i];
            if (arg == "-v" || arg == "--version")
            {
                cout << ICE_STRING_VERSION << endl;
                return 0;
            }
            else if (arg == "-h" || arg == "--help")
            {
                usage(argv[0]);
                return 0;
            }
        }

        // CtrlCHandler must be created before the node is created or any other threads are started.
        Ice::CtrlCHandler ctrlCHandler;

        DataStorm::Node node{argc, argv};

        if (argc > 1)
        {
            cerr << "unrecognized arguments" << endl;
            usage(argv[0]);
            return 1;
        }

        // Shutdown the node on Ctrl-C.
        ctrlCHandler.setCallback([&node](int) { node.shutdown(); });

        // Exit once the user hits Ctrl-C to shutdown the node.
        node.waitForShutdown();
    }
    catch (const std::exception& ex)
    {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}
