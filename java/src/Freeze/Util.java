// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

public class Util
{
    public static BackgroundSaveEvictor
    createBackgroundSaveEvictor(Ice.ObjectAdapter adapter, String envName, String filename, ServantInitializer initializer,
                                Index[] indices, boolean createDb)
    {
        return new BackgroundSaveEvictorI(adapter, envName, filename, initializer, indices, createDb);
    } 

    public static BackgroundSaveEvictor
    createBackgroundSaveEvictor(Ice.ObjectAdapter adapter, String envName, com.sleepycat.db.Environment dbEnv, String filename, 
                                ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        return new BackgroundSaveEvictorI(adapter, envName, dbEnv, filename, initializer, indices, createDb);
    } 


    public static TransactionalEvictor
    createTransactionalEvictor(Ice.ObjectAdapter adapter, String envName, String filename, java.util.Map facetTypes,
                               ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        return new TransactionalEvictorI(adapter, envName, filename, facetTypes, initializer, indices, createDb);
    } 

    public static TransactionalEvictor
    createTransactionalEvictor(Ice.ObjectAdapter adapter, String envName, com.sleepycat.db.Environment dbEnv, String filename, 
                               java.util.Map facetTypes, ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        return new TransactionalEvictorI(adapter, envName, dbEnv, filename, facetTypes, initializer, indices, createDb);
    }

    
    public static Connection
    createConnection(Ice.Communicator communicator, String envName)
    {
        return new ConnectionI(communicator, envName, null);
    } 

    public static Connection
    createConnection(Ice.Communicator communicator, String envName, com.sleepycat.db.Environment dbEnv)
    {
        return new ConnectionI(communicator, envName, dbEnv);
    }

    public static String catalogName()
    {
        return _catalogName;
    }

    public static com.sleepycat.db.Transaction
    getTxn(Transaction tx)
    {
        try
        {
            return ((TransactionI)tx).dbTxn();
        }
        catch(ClassCastException e)
        {
            return null;
        }
    }

    public static synchronized FatalErrorCallback
    registerFatalErrorCallback(FatalErrorCallback cb)
    {
        FatalErrorCallback result = _fatalErrorCallback;
        _fatalErrorCallback = cb;
        return result;
    }
    
    static synchronized void handleFatalError(BackgroundSaveEvictor evictor, Ice.Communicator communicator, RuntimeException ex)
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

    private static String _catalogName = "__catalog";
    private static FatalErrorCallback _fatalErrorCallback = null;
}
