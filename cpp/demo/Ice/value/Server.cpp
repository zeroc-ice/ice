// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ValueI.h>

using namespace std;

class ValueServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    ValueServer app;
    return app.main(argc, argv, "config.server");
}

int
ValueServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Value");
    adapter->add(new InitialI(adapter), communicator()->stringToIdentity("initial"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
