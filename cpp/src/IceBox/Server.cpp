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

int
main(int argc, char* argv[])
{
    CommunicatorPtr communicator;
    ServiceManagerPtr serviceManager;
    int status;

    try
    {
        communicator = initialize(argc, argv);

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
