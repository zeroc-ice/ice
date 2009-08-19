// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Compression.h>

using namespace std;
using namespace Compression;

class ReceiverI : public Receiver
{
public:

    virtual void sendData(const ByteSeq&, const Ice::Current&)
    {
    }
};

class Server : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Server app;
    return app.main(argc, argv, "config.server");
}

int
Server::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Compression");
    Ice::ObjectPtr object = new ReceiverI;
    adapter->add(object, communicator()->stringToIdentity("receiver"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
