// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
