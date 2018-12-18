// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Ice::PropertiesPtr properties = communicator->getProperties();

    string name = properties->getProperty("Ice.ProgramName");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Server");
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("nonallocatable"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable1"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable2"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable3"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable4"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable11"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable21"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable31"));
    adapter->add(new TestI(properties), Ice::stringToIdentity("allocatable41"));

    try
    {
        adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
