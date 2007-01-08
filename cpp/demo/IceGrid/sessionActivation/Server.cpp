// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

class Server : public Ice::Application
{
public:

    virtual int run(int argc, char* argv[]);

};

int
main(int argc, char* argv[])
{
    Server app;
    int status = app.main(argc, argv);
    return status;
}

int
Server::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Hello");
    Ice::Identity id = communicator()->stringToIdentity(properties->getProperty("Identity"));
    adapter->add(new HelloI(properties->getProperty("Ice.ServerId")), id);
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
