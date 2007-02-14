// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <ServantLocatorI.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;

class TestServer : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    TestServer app;
    return app.main(argc, argv);
}

int
TestServer::run(int argc, char* argv[])
{
    communicator()->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000:udp");
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
    ServantLocatorPtr locator = new ServantLocatorI;
    
    adapter->addServantLocator(locator, "");
    adapter->activate();
    adapter->waitForDeactivate();
    return EXIT_SUCCESS;
}
