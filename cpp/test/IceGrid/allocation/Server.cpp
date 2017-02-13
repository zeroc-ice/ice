// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace std;

class Server : public Ice::Application
{
public:

    virtual int run(int argc, char* argv[]);
};

int
Server::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();

    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("Test", args);
    Ice::stringSeqToArgs(args, argc, argv);

    string name = properties->getProperty("Ice.ProgramName");

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Server");
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("nonallocatable"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable1"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable2"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable3"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable4"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable11"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable21"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable31"));
    adapter->add(new TestI(properties), communicator()->stringToIdentity("allocatable41"));

    shutdownOnInterrupt();
    try
    {
        adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicator()->waitForShutdown();
    ignoreInterrupt();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    Server app;
    int rc = app.main(argc, argv);
    return rc;
}
