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
#include <Factory.h>
#include <fstream>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::ValueFactoryPtr factory = new Factory;
    communicator->installValueFactory(factory, "::Persistent");
    
    ifstream in("persistent");
    if (!in)
    {
	cerr << argv[0] << ": can't open file \"persistent\" for reading: "
	     << strerror(errno) << endl;
	return EXIT_FAILURE;
    }

    Ice::PicklerPtr pickler = communicator->getPickler();
    Ice::ObjectPtr base = pickler->unpickle(in);
    PersistentPtr persistent = dynamic_cast<Persistent*>(base.get());

    cout << "aString = \"" << persistent->aString << "\"" << endl;
    cout << "aDouble = " << persistent->aDouble << endl;

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
