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
#include <Pickle.h>
#include <fstream>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    PersistentPtr persistent = new Persistent;
    persistent->aString = "this is the persistent string";
    persistent->aDouble = 3.14;

    ofstream out("persistent");
    if (!out)
    {
	cerr << argv[0] << ": can't open file \"persistent\" for writing: "
	     << strerror(errno) << endl;
	return EXIT_FAILURE;
    }

    Ice::PicklerPtr pickler = communicator->getPickler();
    pickler->pickle(persistent.get(), out);

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
    catch(const Ice::LocalException& ex)
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
	catch(const Ice::LocalException& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
