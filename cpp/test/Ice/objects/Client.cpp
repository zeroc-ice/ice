// **********************************************************************
//
// Copyright (c) 2003
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

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

class MyObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const string& type)
    {
	if(type == "::B")
	{
	    return new B;
	}
	else if(type == "::C")
	{
	    return new C;
	}
	else if(type == "::D")
	{
	    return new D;
	}
	assert(false); // Should never be reached
	return 0;
    }

    virtual void destroy()
    {
	// Nothing to do
    }
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectFactoryPtr factory = new MyObjectFactory;
    communicator->addObjectFactory(factory, "::B");
    communicator->addObjectFactory(factory, "::C");
    communicator->addObjectFactory(factory, "::D");

    InitialPrx allTests(const Ice::CommunicatorPtr&, bool);
    InitialPrx initial = allTests(communicator, false);
    initial->shutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	communicator = Ice::initialize(argc, argv);
	communicator->getProperties()->setProperty("Ice.Trace.Network", "3");
	communicator->getProperties()->setProperty("Ice.Trace.Protocol", "3");
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
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
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
