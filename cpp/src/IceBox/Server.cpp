// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceBox/ServiceManagerI.h>

using namespace std;
using namespace Ice;
using namespace IceBox;

static void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
main(int argc, char* argv[])
{
    CommunicatorPtr communicator;
    ServiceManagerPtr serviceManager;
    int status;

    try
    {
        communicator = initialize(argc, argv);

	/*
        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
                usage(argv[0]);
                return EXIT_SUCCESS;
            }
            else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
            {
                cout << ICE_STRING_VERSION << endl;
                return EXIT_SUCCESS;
            }
            else
            {
                cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }
        }
	*/

        PropertiesPtr properties = communicator->getProperties();
        StringSeq args = argsToStringSeq(argc, argv);
        args = properties->parseCommandLineOptions("IceBox", args);
        stringSeqToArgs(args, argc, argv);

        ServiceManagerI* serviceManagerImpl = new ServiceManagerI(communicator, argc, argv);
        serviceManager = serviceManagerImpl;
        status = serviceManagerImpl->run();
    }
    catch (const Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if (communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch (const Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
