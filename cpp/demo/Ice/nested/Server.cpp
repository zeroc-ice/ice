// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    return app.main(argc, argv, "config");
}

int
NestedServer::run(int argc, char* argv[])
{
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("NestedServerAdapter");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy("nestedServer"));
    adapter->add(new NestedI(self), "nestedServer");
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
