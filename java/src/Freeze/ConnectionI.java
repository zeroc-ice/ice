// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class ConnectionI implements Connection
{

    public Transaction
    beginTransaction()
    {
        if(_transaction != null)
        {
            throw new Freeze.TransactionAlreadyInProgressException();
        }
        closeAllIterators();
        _transaction = new TransactionI(this);
        return _transaction;
    }

    public Transaction
    currentTransaction()
    {
        return _transaction;
    }

    public void
    close()
    {
        close(false);
    }

    public Ice.Communicator
    getCommunicator()
    {
        return _communicator;
    }

    public String
    getName()
    {
        return _envName;
    }

    protected void
    finalize()
    {
        close(true);
    }

    void
    close(boolean finalizing)
    {
        if(_transaction != null)
        {
            if(finalizing)
            {
                _communicator.getLogger().warning
                    ("Finalizing Connection on DbEnv \"" +  _envName + "\" with active transaction");
            }
            
            try
            {
                _transaction.rollback();
            }
            catch(Freeze.DatabaseException dx)
            {
                //
                // Ignored
                //
            }
        }

        
        synchronized(this)
        {
            java.util.Iterator p = _mapList.iterator();
            while(p.hasNext())
            {
                ((Map) p.next()).close(finalizing);
            }
        }
        

        if(_dbEnv != null)
        {
            try
            {
                _dbEnv.close();
            }
            finally
            {
                _dbEnv = null;
            }
        }
    }

    ConnectionI(SharedDbEnv dbEnv)
    {
        _dbEnv = dbEnv;
        _communicator = dbEnv.getCommunicator();
        _envName = dbEnv.getEnvName();
        _trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Map");
        _txTrace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Transaction");
        
        Ice.Properties properties = _communicator.getProperties();
        _deadlockWarning = properties.getPropertyAsInt("Freeze.Warn.Deadlocks") > 0;
        _closeInFinalizeWarning = properties.getPropertyAsIntWithDefault("Freeze.Warn.CloseInFinalize", 1) > 0; 
    }

    ConnectionI(Ice.Communicator communicator, String envName, com.sleepycat.db.Environment dbEnv)
    {
        this(SharedDbEnv.get(communicator, envName, dbEnv));
    }


    //
    // The synchronization is only needed only during finalization
    //

    synchronized void
    closeAllIterators()
    {
        java.util.Iterator p = _mapList.iterator();
        while(p.hasNext())
        {
            ((Map) p.next()).closeAllIterators();
        }
    }

    synchronized java.util.Iterator
    registerMap(Map map)
    {
        _mapList.addFirst(map);
        java.util.Iterator p = _mapList.iterator();
        p.next();
        return p;
    }

    synchronized void
    unregisterMap(java.util.Iterator p)
    {
        p.remove();
    }

    void
    clearTransaction()
    {
        _transaction = null;
    }

    com.sleepycat.db.Transaction
    dbTxn()
    {
        if(_transaction == null)
        {
            return null;
        }
        else
        {
            return _transaction.dbTxn();
        }
    }

    SharedDbEnv
    dbEnv()
    {
        return _dbEnv;
    }

    String
    envName()
    {
        return _envName;
    }

    Ice.Communicator
    communicator()
    {
        return _communicator;
    }

    final int
    trace()
    {
        return _trace;
    }

    final int
    txTrace()
    {
        return _txTrace;
    }

    final boolean
    deadlockWarning()
    {
        return _deadlockWarning;
    }

    final boolean
    closeInFinalizeWarning()
    {
        return _closeInFinalizeWarning;
    }

    private Ice.Communicator _communicator;
    private SharedDbEnv _dbEnv;
    private String _envName;
    private TransactionI _transaction;
    private LinkedList _mapList = new Freeze.LinkedList();
    private int _trace;
    private int _txTrace;
    private boolean _deadlockWarning;
    private boolean _closeInFinalizeWarning;
}
