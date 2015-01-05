// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

class HelloServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    HelloServer app;
    return app.main(argc, argv, "config.server");
}

int
HelloServer::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Hello");
    Demo::HelloPtr hello = new HelloI;
    adapter->add(hello, communicator()->stringToIdentity("hello"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
