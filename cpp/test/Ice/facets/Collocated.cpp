// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
    Ice::PropertiesPtr properties = communicator->getProperties();

    string protocol = properties->getProperty("Ice.Protocol");

    if (protocol.empty())
    {
        protocol = "tcp";
    }

    string endpts = protocol + " -p 12345 -t 2000";
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("TestAdapter", endpts);
    Ice::ObjectPtr d = new DI;
    adapter->add(d, "d");
    d->_ice_addFacet(d, "facetABCD");
    d->_ice_addFacet(new FI, "facetEF");
    d->_ice_addFacet(new GI(communicator), "facetG");

    GPrx allTests(const Ice::CommunicatorPtr&);
    allTests(communicator);

    d->_ice_removeAllFacets(); // Break cyclic dependencies
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

    if (communicator)
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
