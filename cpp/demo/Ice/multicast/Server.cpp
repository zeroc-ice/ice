// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <Discovery.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class HelloI : public Hello
{
public:

    virtual void
    sayHello(const Ice::Current&)
    {
        cout << "Hello World!" << endl;
    }
};

class DiscoverI : public Discover
{
public:

    DiscoverI(const Ice::ObjectPrx& obj) :
        _obj(obj)
    {
    }

    virtual void
    lookup(const DiscoverReplyPrx& reply, const Ice::Current&)
    {
        try
        {
            reply->reply(_obj);
        }
        catch(const Ice::LocalException&)
        {
            // Ignore
        }
    }

private:

    const Ice::ObjectPrx _obj;
};

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
    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = communicator()->getProperties()->parseCommandLineOptions("Discover", args);

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Hello");
    Ice::ObjectAdapterPtr discoverAdapter = communicator()->createObjectAdapter("Discover");

    Ice::ObjectPrx hello = adapter->addWithUUID(new HelloI);
    DiscoverPtr d = new DiscoverI(hello);
    discoverAdapter->add(d, communicator()->stringToIdentity("discover"));

    discoverAdapter->activate();
    adapter->activate();

    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
