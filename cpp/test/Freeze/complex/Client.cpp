// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <ComplexDict.h>
#include <TestCommon.h>
#include <NodeI.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

//
// Don't use namespace Complex to ensure that the Complex namespace
// exists correctly.
//
// using namespace Complex;
//

static int
run(int argc, char* argv[], const DBPtr& db)
{
    //
    // Register a factory for the node types.
    //
    CommunicatorPtr communicator = db->getCommunicator();
    Ice::ObjectFactoryPtr factory = new Complex::ObjectFactoryI;
    communicator->addObjectFactory(factory, "::Complex::NumberNode");
    communicator->addObjectFactory(factory, "::Complex::AddNode");
    communicator->addObjectFactory(factory, "::Complex::MultiplyNode");

    Complex::ComplexDict m(db);

    cout << "Testing database expressions...";
    Complex::ComplexDict::const_iterator p;
    for (p = m.begin(); p != m.end(); ++p)
    {
	test(p->first.result == p->second->calc());
    }
    cout << "ok" << endl;

    return 0;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
    DBEnvironmentPtr dbEnv;
    string dbEnvDir = "db";
    DBPtr db;

    try
    {
	communicator = Ice::initialize(argc, argv);
	if (argc != 1)
	{
	    dbEnvDir = argv[1];
	    dbEnvDir += "/";
	    dbEnvDir += "db";
	}
	dbEnv = Freeze::initialize(communicator, dbEnvDir);
	db = dbEnv->openDB("test");
	status = run(argc, argv, db);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (db)
    {
	try
	{
	    db->close();
	}
	catch(const DBException& ex)
	{
	    cerr << argv[0] << ": " << ex << ": " << ex.message << endl;
	    status = EXIT_FAILURE;
	}
	catch(const Exception& ex)
	{
	    cerr << argv[0] << ": " << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(...)
	{
	    cerr << argv[0] << ": unknown exception" << endl;
	    status = EXIT_FAILURE;
	}
	db = 0;
    }

    if (dbEnv)
    {
	try
	{
	    dbEnv->close();
	}
	catch(const DBException& ex)
	{
	    cerr << argv[0] << ": " << ex << ": " << ex.message << endl;
	    status = EXIT_FAILURE;
	}
	catch(const Exception& ex)
	{
	    cerr << argv[0] << ": " << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(...)
	{
	    cerr << argv[0] << ": unknown exception" << endl;
	    status = EXIT_FAILURE;
	}
	dbEnv = 0;
    }

    try
    {
	communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    return status;
}
