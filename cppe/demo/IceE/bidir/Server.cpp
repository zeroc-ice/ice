// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <CallbackI.h>

using namespace std;
using namespace Demo;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Callback.Server");
    CallbackSenderIPtr sender = new CallbackSenderI;
    adapter->add(sender, Ice::stringToIdentity("sender"));
    adapter->activate();

    sender->start();
    try
    {
	communicator->waitForShutdown();
    }
    catch(...)
    {
	sender->destroy();
	throw;
    }
    sender->destroy();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::PropertiesPtr properties = Ice::createProperties();
        properties->load("config");
        communicator = Ice::initializeWithProperties(argc, argv, properties);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        fprintf(stderr, "%s\n", ex.toString().c_str());
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            fprintf(stderr, "%s\n", ex.toString().c_str());
            status = EXIT_FAILURE;
        }
    }

    return status;
}
