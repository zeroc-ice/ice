// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Application.h>
#include <TestI.h>

using namespace std;

class Server : public Ice::Application
{
public:

    virtual int run(int argc, char* argv[]);

};

int
::Server::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();
    string name = properties->getProperty("Ice.ProgramName");

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Server");
    Ice::ObjectPtr object = new TestI(adapter, properties);
    adapter->add(object, Ice::stringToIdentity(name));
    adapter->activate();
    shutdownOnInterrupt();
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
