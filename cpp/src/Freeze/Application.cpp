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
    catch(const DBException& ex)
    {
	cerr << appName() << ": " << ex << ": " << ex.message << endl;
	status = EXIT_FAILURE;
    }
    // TODO: How does this compile?
    catch(const Exception& ex)
    {
	cerr << appName() << ": " << ex << endl;
	status = EXIT_FAILURE;
    }
    catch(...)
    {
	cerr << appName() << ": unknown exception" << endl;
	status = EXIT_FAILURE;
    }

    if(dbEnv)
    {
	try
	{
	    dbEnv->close();
	}
	catch(const DBException& ex)
	{
	    cerr << appName() << ": " << ex << ": " << ex.message << endl;
	    status = EXIT_FAILURE;
	}
	catch(const Exception& ex)
	{
	    cerr << appName() << ": " << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(...)
	{
	    cerr << appName() << ": unknown exception" << endl;
	    status = EXIT_FAILURE;
	}
	dbEnv = 0;
    }

    defaultInterrupt();
    return status;
}
