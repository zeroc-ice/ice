// **********************************************************************
//
// Copyright (c) 2002
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

package Freeze;

public abstract class Application extends Ice.Application
{
    public
    Application(String dbEnvName)
    {
	_dbEnvName = dbEnvName;
    }

    public int
    run(String[] args)
    {
	int status;
	DBEnvironment dbEnv = null;

	try
	{
	    dbEnv = Freeze.Util.initialize(communicator(), _dbEnvName);
	    status = runFreeze(args, dbEnv);
	}
	catch(DBException ex)
	{
	    System.err.println(appName() + ": " + ex + ": " + ex.message);
	    status = 1;
	}
	catch(Ice.LocalException ex)
	{
	    System.err.println(appName() + ": " + ex);
	    ex.printStackTrace();
	    status = 1;
	}
	catch(Exception ex)
	{
	    System.err.println(appName() + ": unknown exception");
	    ex.printStackTrace();
	    status = 1;
	}
	
	if(dbEnv != null)
	{
	    try
	    {
		dbEnv.close();
	    }
	    catch(DBException ex)
	    {
		System.err.println(appName() + ": " + ex + ": " + ex.message);
		ex.printStackTrace();
		status = 1;
	    }
	    catch(Exception ex)
	    {
		System.err.println(appName() + ": unknown exception");
		ex.printStackTrace();
		status = 1;
	    }
	    dbEnv = null;
	}
	
	return status;
    }

    public abstract int
    runFreeze(String[] args, DBEnvironment dbEnv);

    private String _dbEnvName;
}
