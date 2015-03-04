// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

int
main(int argc, char* argv[])
{
    Ice::CommunicatorPtr communicator;

    try
    {
        communicator = Ice::initialize(argc, argv);
        HelloPrx hello = HelloPrx::checkedCast(communicator->stringToProxy("hello:tcp -h localhost -p 10000"));
        hello->sayHello();
        communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        try
        {
            if(communicator)
            {
                communicator->destroy();
            }
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
        }
        exit(1);
    }
    return 0;
}
