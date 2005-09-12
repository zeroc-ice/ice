// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <HelloI.h>
#include <Ice/Application.h>

using namespace std;

class Server : public Ice::Application
{
public:

    virtual int run(int argc, char* argv[]);

};

int
Server::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Hello");

    string id = communicator()->getProperties()->getProperty("Identity");

    adapter->add(new HelloFactoryI, Ice::stringToIdentity(id));
    adapter->activate();

    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    Server app;
    return app.main(argc, argv);
}
