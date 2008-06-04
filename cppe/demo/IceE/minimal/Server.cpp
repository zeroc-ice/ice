// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class HelloI : public Hello
{
public:

    virtual void
    sayHello(const Ice::Current&) const
    {
        printf("Hello World!\n");
    }
};

int
main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;
    Ice::CommunicatorPtr communicator;

    try
    {
        communicator = Ice::initialize(argc, argv);
        if(argc > 1)
        {
            fprintf(stderr, "%s: too many arguments\n", argv[0]);
            return EXIT_FAILURE;
        }
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("Hello", "tcp -p 10000");
        Demo::HelloPtr object = new HelloI;
        adapter->add(object, communicator->stringToIdentity("hello"));
        adapter->activate();
        communicator->waitForShutdown();
    }
    catch(const Ice::Exception& ex)
    {
        fprintf(stderr, "%s\n", ex.toString().c_str());
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            fprintf(stderr, "%s\n", ex.toString().c_str());
            status = EXIT_FAILURE;
        }
    }

    return status;
}
