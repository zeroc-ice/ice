// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Latency.h>
#include <IceE/IdentityUtil.h>

#include <iostream>

using namespace std;
using namespace Demo;

class LatencyI : public Latency
{
    virtual void
    ping(const Ice::Current& current)
    {
    }

    virtual void
    withData(const ByteSeq& data, const Ice::Current& current)
    {
    }

    virtual void
    shutdown(const Ice::Current& current)
    {
        current.adapter->getCommunicator()->shutdown();
    }
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Latency");
    Ice::ObjectPtr object = new LatencyI;
    adapter->add(object, Ice::stringToIdentity("latency"));
    adapter->activate();
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::PropertiesPtr properties =  Ice::createProperties();
        properties->load("config");
        communicator = Ice::initializeWithProperties(argc, argv, properties);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex.ice_name() << endl;
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
            cerr << ex.ice_name() << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
