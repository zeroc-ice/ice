// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    string endpts = "default -p 12345 -t 2000";
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("TestAdapter", endpts);
    Ice::ObjectPtr d = new DI;
    adapter->add(d, Ice::stringToIdentity("d"));
    d->ice_addFacet(d, "facetABCD");
    Ice::ObjectPtr f = new FI;
    d->ice_addFacet(f, "facetEF");
    Ice::ObjectPtr h = new HI(communicator);
    f->ice_addFacet(h, "facetGH");

    adapter->activate();
    communicator->waitForShutdown();

    d->ice_removeAllFacets(); // Break cyclic dependencies
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
