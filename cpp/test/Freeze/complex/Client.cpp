// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <ComplexDict.h>
#include <TestCommon.h>
#include <NodeI.h>
#include <Parser.h>

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
validate(const DBPtr& db)
{
    Complex::ComplexDict m(db);

    cout << "testing database expressions... ";
    Complex::ComplexDict::const_iterator p;
    Parser parser;
    for(p = m.begin(); p != m.end(); ++p)
    {
	//
	// Verify the stored record is correct.
	//
	test(p->first.result == p->second->calc());

	//
	// Verify that the expression & result again.
	//
	Complex::NodePtr root = parser.parse(p->first.expression);
	test(root->calc() == p->first.result);
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

static int
populate(const DBPtr& db)
{
    static const char* expressions[] = 
    {
	"2",
	"10",
	"2+(5*3)",
	"5*(2+3)",
	"10+(10+(20+(8*(2*(3*2+4+5+6)))))"
    };
    static int nexpressions = sizeof(expressions)/sizeof(expressions[0]);

    Complex::ComplexDict m(db);

    cout << "populating the database... ";
    Parser parser;
    for(int i = 0 ; i < nexpressions; ++i)
    {
	Complex::NodePtr root = parser.parse(expressions[i]);
	assert(root);
	Complex::Key k;
	k.expression = expressions[i];
	k.result = root->calc();
	m.insert(make_pair(k, root));
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

static void
usage(const char* name)
{
    cerr << "Usage: " << name << " [options] validate|populate\n";
    cerr <<	
	"Options:\n"
        "--dbdir           Location of the database directory.\n";
}

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

    if(argc > 1 && strcmp(argv[1], "populate") == 0)
    {
	return populate(db);
    }
    if(argc > 1 && strcmp(argv[1], "validate") == 0)
    {
	return validate(db);
    }
    usage(argv[0]);

    return EXIT_FAILURE;
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
	//
	// Scan for --dbdir command line argument.
	//
	int i = 1;
	while(i < argc)
	{
	    if(strcmp(argv[i], "--dbdir") == 0)
	    {
		if(i +1 >= argc)
		{
		    usage(argv[0]);
		    return EXIT_FAILURE;
		}

		dbEnvDir = argv[i+1];
		dbEnvDir += "/";
		dbEnvDir += "db";

		//
		// Consume arguments
		//
		while(i < argc - 2)
		{
		    argv[i] = argv[i+2];
		    ++i;
		}
		argc -= 2;
	    }
	    else
	    {
		++i;
	    }
	}

	communicator = Ice::initialize(argc, argv);
	dbEnv = Freeze::initialize(communicator, dbEnvDir);
	db = dbEnv->openDB("test", true);
	status = run(argc, argv, db);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(db)
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

    if(dbEnv)
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
