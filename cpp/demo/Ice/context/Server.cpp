// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ContextI.h>

using namespace std;

class ContextServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    ContextServer app;
    return app.main(argc, argv, "config.server");
}

int
ContextServer::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Context");
    Demo::ContextPtr hello = new ContextI;
    adapter->add(hello, communicator()->stringToIdentity("context"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
