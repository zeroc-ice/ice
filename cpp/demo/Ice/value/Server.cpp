// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ValueI.h>
#include <ObjectFactory.h>

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
ValueServer::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectFactoryPtr factory = new ObjectFactory;
    communicator()->addObjectFactory(factory, Demo::Printer::ice_staticId());

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Value");
    Demo::InitialPtr initial = new InitialI(adapter);
    adapter->add(initial, communicator()->stringToIdentity("initial"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
