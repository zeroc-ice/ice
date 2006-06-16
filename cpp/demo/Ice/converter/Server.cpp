// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <EchoI.h>
#include <Ice/Application.h>

using namespace std;

class EchoServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    EchoServer app;
    return app.main(argc, argv, "config.server");
}

int
EchoServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Echo");
    adapter->add(new EchoI, communicator()->stringToIdentity("echo"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
