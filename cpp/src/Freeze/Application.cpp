// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Application.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

Freeze::Application::Application(const string& dbEnvName) :
    _dbEnvName(dbEnvName)
{
}

Freeze::Application::~Application()
{
}

int
Freeze::Application::run(int argc, char* argv[])
{
    ignoreInterrupt();
    
    int status;
    DBEnvironmentPtr dbEnv;
    
    try
    {
	dbEnv = Freeze::initialize(communicator(), _dbEnvName);
	status = runFreeze(argc, argv, dbEnv);
    }
    catch(const LocalException& ex)
    {
	cerr << appName() << ": local exception: " << ex << endl;
	status = EXIT_FAILURE;
    }
    catch(const DBExceptionPtrE& ex)
    {
	cerr << appName() << ": database exception: " << ex->message << endl;
	status = EXIT_FAILURE;
    }
    catch(...)
    {
	cerr << appName() << ": unknown exception" << endl;
	status = EXIT_FAILURE;
    }

    if (dbEnv)
    {
	try
	{
	    dbEnv->close();
	}
	catch(const LocalException& ex)
	{
	    cerr << appName() << ": local exception: " << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(const DBExceptionPtrE& ex)
	{
	    cerr << appName() << ": database exception: " << ex->message << endl;
	    status = EXIT_FAILURE;
	}
	catch(...)
	{
	    cerr << appName() << ": unknown exception" << endl;
	    status = EXIT_FAILURE;
	}
    }

    defaultInterrupt();
    return status;
}
