// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


package Freeze;

public class Util
{

    public static Evictor
    createEvictor(Ice.ObjectAdapter adapter, String envName, String filename, 
		  ServantInitializer initializer, Index[] indices, boolean createDb)
    {
	return new EvictorI(adapter, envName, filename, initializer, indices, createDb);
    } 

    public static Evictor
    createEvictor(Ice.ObjectAdapter adapter, String envName, 
		  com.sleepycat.db.DbEnv dbEnv, String filename, 
		  ServantInitializer initializer, Index[] indices, boolean createDb)
    {
	return new EvictorI(adapter, envName, dbEnv, filename, initializer, indices, createDb);
    } 

    public static Connection
    createConnection(Ice.Communicator communicator, String envName)
    {
	return new ConnectionI(communicator, envName);
    } 

    public static Connection
    createConnection(Ice.Communicator communicator, String envName, com.sleepycat.db.DbEnv dbEnv)
    {
	return new ConnectionI(communicator, envName, dbEnv);
    } 

    public static synchronized FatalErrorCallback
    registerFatalErrorCallback(FatalErrorCallback cb)
    {
	FatalErrorCallback result = _fatalErrorCallback;
	_fatalErrorCallback = cb;
	return result;
    }
    
    static synchronized void handleFatalError(Evictor evictor, Ice.Communicator communicator, RuntimeException ex)
    {
	if(_fatalErrorCallback != null)
	{
	    _fatalErrorCallback.handleError(evictor, communicator, ex);
	}
	else
	{
	    communicator.getLogger().error("*** Halting JVM ***");
	    Runtime.getRuntime().halt(1);
	}
    }


    private static FatalErrorCallback _fatalErrorCallback = null;
}

