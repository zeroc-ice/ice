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

#include <Ice/Application.h>
#include <NestedI.h>

using namespace std;
using namespace Ice;

class NestedServer : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    NestedServer app;
    return app.main(argc, argv, "config.server");
}

int
NestedServer::run(int argc, char* argv[])
{
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Nested.Server");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy(Ice::stringToIdentity("nestedServer")));
    adapter->add(new NestedI(self), Ice::stringToIdentity("nestedServer"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
