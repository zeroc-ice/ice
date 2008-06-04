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
        HelloPrx hello = HelloPrx::checkedCast(communicator->stringToProxy("hello:tcp -p 10000"));
        if(!hello)
        {
            fprintf(stderr, "%s: invalid proxy\n", argv[0]);
            status = EXIT_FAILURE;
        }
        else
        {
            hello->sayHello();
        }
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
