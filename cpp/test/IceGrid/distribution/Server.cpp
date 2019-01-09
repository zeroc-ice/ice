// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
    Ice::ObjectPtr object = new TestI(properties);
    adapter->add(object, Ice::stringToIdentity(name));

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
