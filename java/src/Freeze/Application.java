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

        ShutdownHook hook = null;

	try
	{
	    dbEnv = Freeze.Util.initialize(communicator(), _dbEnvName);
            hook = new ShutdownHook(dbEnv);
            Runtime.getRuntime().addShutdownHook(hook);
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
            Runtime.getRuntime().removeShutdownHook(hook);
	}
	
	return status;
    }

    public abstract int
    runFreeze(String[] args, DBEnvironment dbEnv);

    private String _dbEnvName;
}
