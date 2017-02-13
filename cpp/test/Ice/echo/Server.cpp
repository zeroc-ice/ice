// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <BlobjectI.h>
#include <Test.h>

DEFINE_TEST("server")

using namespace std;

class EchoI : public Test::Echo
{
public:

    EchoI(const BlobjectIPtr& blob) :
        _blob(blob)
    {
    }

    virtual void startBatch(const Ice::Current&)
    {
        _blob->startBatch();
    }

    virtual void flushBatch(const Ice::Current&)
    {
        _blob->flushBatch();
    }

    virtual void shutdown(const Ice::Current& current)
    {
        current.adapter->getCommunicator()->shutdown();
    }

private:

    BlobjectIPtr _blob;
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    BlobjectIPtr blob = new BlobjectI;
    adapter->addDefaultServant(blob, "");
    adapter->add(new EchoI(blob), communicator->stringToIdentity("__echo"));
    adapter->activate();

    TEST_READY

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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
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
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
