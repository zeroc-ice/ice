// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

public class SharedDbEnv implements com.sleepycat.db.ErrorHandler, Runnable
{
    public static SharedDbEnv
    get(Ice.Communicator communicator, String envName, com.sleepycat.db.Environment dbEnv)
    {
        MapKey key = new MapKey(envName, communicator);

        SharedDbEnv result;

        synchronized(_map)
        {
            result = _map.get(key);
            if(result == null)
            {
                result = new SharedDbEnv(key, dbEnv);

                SharedDbEnv previousValue = _map.put(key, result);
                assert(previousValue == null);
            }
            else
            {
                result._refCount++;
            }
        }

        return result;
    }

    //
    // Returns a shared map Db; the caller should NOT close this Db.
    //
    public MapDb
    getSharedMapDb(String dbName, String key, String value, java.util.Comparator<?> comparator, MapIndex[] indices,
                   boolean createDb)
    {
        if(dbName.equals(_catalog.dbName()))
        {
            _catalog.checkTypes(key, value);
            return _catalog;
        }
        else if(dbName.equals(_catalogIndexList.dbName()))
        {
            _catalogIndexList.checkTypes(key, value);
            return _catalogIndexList;
        }

        synchronized(_sharedDbMap)
        {
            MapDb db = _sharedDbMap.get(dbName);
            if(db == null)
            {
                ConnectionI insertConnection = (ConnectionI)Util.createConnection(_key.communicator, _key.envName);

                try
                {
                    db = new MapDb(insertConnection, dbName, key, value, comparator, indices, createDb);
                }
                finally
                {
                    insertConnection.close();
                }

                MapDb previousValue = _sharedDbMap.put(dbName, db);
                assert(previousValue == null);
            }
            else
            {
                db.checkTypes(key, value);
                db.connectIndices(indices);
            }
            return db;
        }
    }

    //
    // Tell SharedDbEnv to close and remove this Shared Db from the map
    //
    public void
    removeSharedMapDb(String dbName)
    {
        synchronized(_sharedDbMap)
        {
            MapDb db = _sharedDbMap.remove(dbName);
            if(db != null)
            {
                db.close();
            }
        }
    }

    public String
    getEnvName()
    {
        return _key.envName;
    }

    public Ice.Communicator
    getCommunicator()
    {
        return _key.communicator;
    }

    public Ice.EncodingVersion
    getEncoding()
    {
        return _encoding;
    }

    public com.sleepycat.db.Environment
    getEnv()
    {
        return _dbEnv;
    }

    public void
    close()
    {
        synchronized(_map)
        {
            if(--_refCount == 0)
            {
                //
                // Remove from map
                //
                SharedDbEnv value = _map.remove(_key);
                assert(value == this);

                //
                // Cleanup with _map locked to prevent concurrent cleanup of the same dbEnv
                //
                cleanup();
            }
        }
    }

    @Override
    public void
    run()
    {
        for(;;)
        {
            synchronized(this)
            {
                while(!_done)
                {
                    try
                    {
                        wait(_checkpointPeriod);
                    }
                    catch(InterruptedException ex)
                    {
                        continue;
                    }
                    break;
                }
                if(_done)
                {
                    return;
                }
            }

            if(_trace >= 2)
            {
                _key.communicator.getLogger().trace("Freeze.DbEnv", "checkpointing environment \"" + _key.envName +
                                                    "\"");
            }

            try
            {
                com.sleepycat.db.CheckpointConfig config = new com.sleepycat.db.CheckpointConfig();
                config.setKBytes(_kbyte);
                _dbEnv.checkpoint(config);
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                _key.communicator.getLogger().warning("checkpoint on DbEnv \"" + _key.envName +
                                                      "\" raised DbException: " + dx.getMessage());
            }
        }
    }

    @Override
    public void
    error(com.sleepycat.db.Environment env, String errorPrefix, String message)
    {
        _key.communicator.getLogger().error("Freeze database error in DbEnv \"" + _key.envName + "\": " + message);
    }

    //
    // EvictorContext factory/manager
    //

    //
    // Create an evictor context associated with the calling thread
    //
    synchronized TransactionalEvictorContext
    createCurrent()
    {
        Thread t = Thread.currentThread();

        TransactionalEvictorContext ctx = _ctxMap.get(t);
        assert ctx == null;

        ctx = new TransactionalEvictorContext(this);
        synchronized(_map)
        {
            _refCount++; // owned by the underlying ConnectionI
        }
        _ctxMap.put(t, ctx);

        return ctx;
    }

    synchronized TransactionalEvictorContext
    getCurrent()
    {
        Thread t = Thread.currentThread();
        return _ctxMap.get(t);
    }

    synchronized void
    setCurrentTransaction(Transaction tx)
    {
        TransactionI txi = (TransactionI)tx;

        if(txi != null)
        {
            if(txi.getConnectionI() == null || txi.getConnectionI().dbEnv() == null)
            {
                throw new DatabaseException(errorPrefix(_key.envName) + "invalid transaction");
            }

            //
            // Check this tx refers to this DbEnv
            //
            if(txi.getConnectionI().dbEnv() != this)
            {
                throw new DatabaseException(errorPrefix(_key.envName) +
                                            "the given transaction is bound to environment '" +
                                            txi.getConnectionI().dbEnv()._key.envName + "'");
            }
        }

        Thread t = Thread.currentThread();

        if(txi != null)
        {
            TransactionalEvictorContext ctx = _ctxMap.get(t);
            if(ctx == null || !tx.equals(ctx.transaction()))
            {
                ctx = new TransactionalEvictorContext(txi, getCommunicator());
                _ctxMap.put(t, ctx);
            }
        }
        else
        {
            _ctxMap.put(t, null);
        }
    }

    private
    SharedDbEnv(MapKey key, com.sleepycat.db.Environment dbEnv)
    {
        _key = key;
        _dbEnv = dbEnv;
        _ownDbEnv = (dbEnv == null);

        Ice.Properties properties = key.communicator.getProperties();
        _trace = properties.getPropertyAsInt("Freeze.Trace.DbEnv");

        String propertyPrefix = "Freeze.DbEnv." + _key.envName;
        String dbHome = properties.getPropertyWithDefault(propertyPrefix + ".DbHome", _key.envName);

        String encoding = properties.getPropertyWithDefault(
            propertyPrefix + ".EncodingVersion", Ice.Util.encodingVersionToString(Ice.Util.currentEncoding()));
        _encoding = Ice.Util.stringToEncodingVersion(encoding);
        IceInternal.Protocol.checkSupportedEncoding(_encoding);

        java.io.File dir = new java.io.File(dbHome);
        if(!dir.exists())
        {
            throw new DatabaseException("DbHome directory `" + dbHome + "' does not exists");
        }

        //
        // Normally the file lock is necessary, but for read-only situations (such as when
        // using the FreezeScript utilities) this property allows the file lock to be
        // disabled.
        //
        if(properties.getPropertyAsIntWithDefault(propertyPrefix + ".LockFile", 1) > 0)
        {
            //
            // Use a file lock to prevent multiple processes from opening the same db env. We
            // create the lock file in a sub-directory to ensure db_hotbackup won't try to copy
            // the file when backing up the environment (this would fail on Windows where copying
            // a locked file isn't possible).
            //
            dir = new java.io.File(dbHome + "/__Freeze");
            if(!dir.exists())
            {
                if(!dir.mkdir())
                {
                    throw new DatabaseException("Failed to create directory `" + dbHome + "/__Freeze'");
                }
            }
            _fileLock = new IceUtilInternal.FileLock(dbHome + "/__Freeze/lock");
        }

        try
        {
            if(_ownDbEnv)
            {
                com.sleepycat.db.EnvironmentConfig config = new com.sleepycat.db.EnvironmentConfig();

                config.setErrorHandler(this);
                config.setInitializeLocking(true);
                config.setInitializeLogging(true);
                config.setInitializeCache(true);
                config.setAllowCreate(true);
                config.setTransactional(true);

                //
                // Deadlock detection
                //
                config.setLockDetectMode(com.sleepycat.db.LockDetectMode.YOUNGEST);

                if(properties.getPropertyAsInt(propertyPrefix + ".DbRecoverFatal") > 0)
                {
                    config.setRunFatalRecovery(true);
                }
                else
                {
                    config.setRunRecovery(true);
                }

                if(properties.getPropertyAsIntWithDefault(propertyPrefix + ".DbPrivate", 1) > 0)
                {
                    config.setPrivate(true);
                }

                if(properties.getPropertyAsIntWithDefault(propertyPrefix + ".OldLogsAutoDelete", 1) > 0)
                {
                    config.setLogAutoRemove(true);
                }

                if(_trace >= 1)
                {
                    _key.communicator.getLogger().trace("Freeze.DbEnv", "opening database environment \"" +
                                                        _key.envName + "\"");
                }

                try
                {
                    java.io.File home = new java.io.File(dbHome);
                    _dbEnv = new com.sleepycat.db.Environment(home, config);
                }
                catch(java.io.FileNotFoundException dx)
                {
                    throw new NotFoundException(errorPrefix(_key.envName) + "open: " + dx.getMessage(), dx);
                }

                //
                // Default checkpoint period is every 120 seconds
                //
                _checkpointPeriod =
                    properties.getPropertyAsIntWithDefault(propertyPrefix + ".CheckpointPeriod", 120) * 1000;

                _kbyte = properties.getPropertyAsIntWithDefault(propertyPrefix + ".PeriodicCheckpointMinSize", 0);

                String threadName;
                String programName = properties.getProperty("Ice.ProgramName");
                if(programName.length() > 0)
                {
                    threadName = programName + "-";
                }
                else
                {
                    threadName = "";
                }
                threadName += "FreezeCheckpointThread(" + _key.envName + ")";

                if(_checkpointPeriod > 0)
                {
                    _thread = new Thread(this, threadName);
                    _thread.start();
                }
            }

            _catalog = new MapDb(_key.communicator, _key.envName, Util.catalogName(), "string",
                                 "::Freeze::CatalogData", _dbEnv);
            _catalogIndexList = new MapDb(_key.communicator, _key.envName, Util.catalogIndexListName(),
                                          "string", "::Ice::StringSeq", _dbEnv);
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            cleanup();
            throw new DatabaseException(errorPrefix(_key.envName) + "creation: " + dx.getMessage(), dx);
        }
        catch(java.lang.RuntimeException ex)
        {
            cleanup();
            throw ex;
        }
        catch(java.lang.Error ex)
        {
            cleanup();
            throw ex;
        }

        _refCount = 1;
    }

    private void
    cleanup()
    {
        //
        // Join thread
        //
        synchronized(this)
        {
            _done = true;
            notify();
        }

        while(_thread != null)
        {
            try
            {
                _thread.join();
                _thread = null;
                break;
            }
            catch(InterruptedException ex)
            {
            }
        }

        //
        // Release catalogs
        //
        if(_catalog != null)
        {
            try
            {
                _catalog.close();
            }
            finally
            {
                _catalog = null;
            }
        }

        if(_catalogIndexList != null)
        {
            try
            {
                _catalogIndexList.close();
            }
            finally
            {
                _catalogIndexList = null;
            }
        }

        //
        // Close Dbs
        //
        for(MapDb db: _sharedDbMap.values())
        {
            db.close();
        }

        if(_trace >= 1)
        {
            _key.communicator.getLogger().trace("Freeze.DbEnv", "closing database environment \"" +
                                                _key.envName + "\"");
        }

        if(_ownDbEnv && _dbEnv != null)
        {
            try
            {
                _dbEnv.close();
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                throw new DatabaseException(errorPrefix(_key.envName) + "close: " + dx.getMessage(), dx);
            }
            finally
            {
                _dbEnv = null;
            }
        }

        //
        // Release the file lock
        //
        if(_fileLock != null)
        {
            _fileLock.release();
        }
    }

    private static String
    errorPrefix(String envName)
    {
        return "DbEnv(\"" + envName + "\"): ";
    }

    private static class MapKey
    {
        final String envName;
        final Ice.Communicator communicator;

        MapKey(String envName, Ice.Communicator communicator)
        {
            this.envName = envName;
            this.communicator = communicator;
        }

        @Override
        public boolean
        equals(Object o)
        {
            try
            {
                MapKey k = (MapKey)o;
                return (communicator == k.communicator) && envName.equals(k.envName);
            }
            catch(ClassCastException ex)
            {
                communicator.getLogger().trace("Freeze.DbEnv", "equals cast failed");
                return false;
            }
        }

        @Override
        public int
        hashCode()
        {
            int h = 5381;
            h = IceInternal.HashUtil.hashAdd(h, envName);
            return IceInternal.HashUtil.hashAdd(h, communicator);
        }
    }

    private MapKey _key;
    private com.sleepycat.db.Environment _dbEnv;
    private boolean _ownDbEnv;
    private MapDb _catalog;
    private MapDb _catalogIndexList;
    private int _refCount = 0; // protected by _map!
    private boolean _done = false;
    private int _trace = 0;
    private long _checkpointPeriod = 0;
    private int _kbyte = 0;
    private Thread _thread;
    private Ice.EncodingVersion _encoding;

    private java.util.Map<Thread, TransactionalEvictorContext> _ctxMap =
        new java.util.HashMap<Thread, TransactionalEvictorContext>();

    private java.util.Map<String, MapDb> _sharedDbMap = new java.util.HashMap<String, MapDb>();

    private static java.util.Map<MapKey, SharedDbEnv> _map = new java.util.HashMap<MapKey, SharedDbEnv>();

    private IceUtilInternal.FileLock _fileLock;
}
