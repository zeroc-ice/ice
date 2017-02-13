// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

public class ConnectionI implements Connection
{
    @Override
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

    @Override
    public Transaction
    currentTransaction()
    {
        return _transaction;
    }

    @Override
    public void
    removeMapIndex(String mapName, String indexName)
    {
        if(_dbEnv == null)
        {
            throw new DatabaseException("Closed connection");
        }

        try
        {
            _dbEnv.getEnv().removeDatabase(dbTxn(), mapName + "." + indexName, null);
        }
        catch(com.sleepycat.db.DeadlockException dx)
        {
            throw new DeadlockException(errorPrefix() + dx.getMessage(), _transaction, dx);
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            throw new DatabaseException(errorPrefix() + dx.getMessage(), dx);
        }
        catch(java.io.FileNotFoundException fne)
        {
            throw new IndexNotFoundException(mapName, indexName);
        }
    }

    @Override
    public void
    close()
    {
        if(_transaction != null)
        {
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

        java.util.Iterator<Map<?,?> > p = _mapList.iterator();
        while(p.hasNext())
        {
            p.next().close();
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

    @Override
    public Ice.Communicator
    getCommunicator()
    {
        return _communicator;
    }

    @Override
    public Ice.EncodingVersion
    getEncoding()
    {
        return _encoding;
    }

    @Override
    public String
    getName()
    {
        return _envName;
    }

    @Override
    protected void
    finalize()
        throws Throwable
    {
        try
        {
            if(_dbEnv != null)
            {
                _logger.warning("leaked Connection for DbEnv \"" + _envName + "\"");
            }
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    ConnectionI(SharedDbEnv dbEnv)
    {
        _dbEnv = dbEnv;
        _communicator = dbEnv.getCommunicator();
        _encoding = dbEnv.getEncoding();
        _logger = _communicator.getLogger();
        _envName = dbEnv.getEnvName();
        _trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Map");
        _txTrace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Transaction");

        Ice.Properties properties = _communicator.getProperties();
        _deadlockWarning = properties.getPropertyAsInt("Freeze.Warn.Deadlocks") > 0;
    }

    ConnectionI(Ice.Communicator communicator, String envName, com.sleepycat.db.Environment dbEnv)
    {
        this(SharedDbEnv.get(communicator, envName, dbEnv));
    }

    public void
    closeAllIterators()
    {
        java.util.Iterator<Map<?,?>> p = _mapList.iterator();
        while(p.hasNext())
        {
            p.next().closeAllIterators();
        }
    }

    public java.util.Iterator<?>
    registerMap(Map<?,?> map)
    {
        _mapList.addFirst(map);
        java.util.Iterator<Map<?,?>> p = _mapList.iterator();
        p.next();
        return p;
    }

    public void
    unregisterMap(java.util.Iterator<?> p)
    {
        p.remove();
    }

    void
    clearTransaction()
    {
        _transaction = null;
    }

    public com.sleepycat.db.Transaction
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

    public SharedDbEnv
    dbEnv()
    {
        return _dbEnv;
    }

    public String
    envName()
    {
        return _envName;
    }

    public Ice.Communicator
    communicator()
    {
        return _communicator;
    }

    public final int
    trace()
    {
        return _trace;
    }

    public final int
    txTrace()
    {
        return _txTrace;
    }

    public final boolean
    deadlockWarning()
    {
        return _deadlockWarning;
    }

    private String
    errorPrefix()
    {
        return "DbEnv(\"" + _envName + "\"): ";
    }

    private Ice.Communicator _communicator;
    private Ice.EncodingVersion _encoding;
    private Ice.Logger _logger;
    private SharedDbEnv _dbEnv;
    private String _envName;
    private TransactionI _transaction;
    private LinkedList<Map<?,?> > _mapList = new LinkedList<Map<?,?>>();
    private int _trace;
    private int _txTrace;
    private boolean _deadlockWarning;
}
