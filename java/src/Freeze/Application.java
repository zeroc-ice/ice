// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Freeze;

public abstract class Application extends Ice.Application
{

    public Application(String dbEnvName)
    {
	_dbEnvName = dbEnvName;
    }

    public int
    run(String[] args)
    {
	//
	// TODO: Can interrupts be handled in Java?
	//
	//ignoreInterrupt();
	
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
	    status = 1;
	}
	catch(Exception ex)
	{
	    System.err.println(appName() + ": unknown exception");
	    status = 1;
	}
	
	if (dbEnv != null)
	{
	    try
	    {
		dbEnv.close();
	    }
	    catch(DBException ex)
	    {
		System.err.println(appName() + ": " + ex + ": " + ex.message);
		status = 1;
	    }
	    catch(Exception ex)
	    {
		System.err.println(appName() + ": unknown exception");
		status = 1;
	    }
	    dbEnv = null;
	}
	
	//defaultInterrupt();
	return status;
    }

    public abstract int
    runFreeze(String[] args, DBEnvironment dbEnv);

    private String _dbEnvName;

}
