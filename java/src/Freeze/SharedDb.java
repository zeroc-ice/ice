// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class SharedDb
{
    public static SharedDb
    get(ConnectionI connection, String dbName, String key, String value, 
        Map.Index[] indices, boolean createDb, java.util.Comparator comparator,
        java.util.Map indexComparators)
    {
        MapKey mapKey = new MapKey(connection.envName(), connection.communicator(), dbName);

        if(dbName.equals(Util.catalogName()))
        {
            //
            // We don't want to lock the _map to retrieve the catalog
            //
            SharedDb result = connection.dbEnv().getCatalog();
            checkTypes(result, key, value);
            result._refCount++;
            return result;
        }

        synchronized(_map) 
        {
            SharedDb result = (SharedDb)_map.get(mapKey);
            if(result == null)
            {
                try
                {
                    result = new SharedDb(mapKey, key, value, connection, 
                                          indices, createDb, comparator, indexComparators);
                }
                catch(com.sleepycat.db.DatabaseException dx)
                {
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(dx);
                    ex.message = errorPrefix(mapKey) + "creation: " + dx.getMessage();
                    throw ex;
                }

                Object previousValue = _map.put(mapKey, result);
                assert(previousValue == null);
            }
            else
            {
                checkTypes(result, key, value);
                result.connectIndices(indices);
                result._refCount++;
            }
            return result;
        }
    }
        
    public static SharedDb
    openCatalog(SharedDbEnv dbEnv)
    {
        MapKey mapKey = new MapKey(dbEnv.getEnvName(), dbEnv.getCommunicator(), Util.catalogName());

        synchronized(_map) 
        {
            SharedDb result = (SharedDb)_map.get(mapKey);
            if(result != null)
            {
                DatabaseException ex = new DatabaseException();
                ex.message = errorPrefix(mapKey) + "Catalog already opened";
                throw ex;
            }

            try
            {
                result = new SharedDb(mapKey, dbEnv.getEnv());
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(dx);
                ex.message = errorPrefix(mapKey) + "creation: " + dx.getMessage();
                throw ex;
            }
            Object previousValue = _map.put(mapKey, result);
            assert(previousValue == null);
            return result;
        }
    }

    public com.sleepycat.db.Database
    db()
    {
        return _db;
    }

    public String 
    dbName()
    {
        return _mapKey.dbName;
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
                Object value = _map.remove(_mapKey);
                assert(value == this);

                if(_trace >= 1)
                {
                    _mapKey.communicator.getLogger().trace("Freeze.Map", "closing Db \"" + _mapKey.dbName + "\"");
                }

                //
                // Keep lock to prevent somebody else from re-opening this Db
                // before it's closed.
                //
                try
                {
                    cleanupIndices();
                   
                    _db.close();
                }
                catch(com.sleepycat.db.DatabaseException dx)
                {
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(dx);
                    ex.message = errorPrefix(_mapKey) + "close: " + dx.getMessage();
                    throw ex;
                }
            }
        }
    }

    protected void 
    finalize()
    {
        assert(_refCount == 0);
    }

    private SharedDb(MapKey mapKey, String key, String value, ConnectionI connection, Map.Index[] indices,
                     boolean createDb, java.util.Comparator comparator, java.util.Map indexComparators)
        throws com.sleepycat.db.DatabaseException
    {   
        _mapKey = mapKey;
        _indices = indices;
        _trace = connection.trace();

        Connection catalogConnection = Util.createConnection(_mapKey.communicator, connection.dbEnv().getEnvName());

        try
        {
            Catalog catalog = new Catalog(catalogConnection, Util.catalogName(), true);
            CatalogData catalogData = (CatalogData)catalog.get(_mapKey.dbName);
            if(catalogData != null)
            {
                if(catalogData.evictor)
                {
                    DatabaseException ex = new DatabaseException();
                    ex.message = errorPrefix(_mapKey) + "is not an evictor";
                    throw ex;
                }
                _key = catalogData.key;
                _value = catalogData.value;
                checkTypes(this, key, value);
            }
            else
            {
                _key = key;
                _value = value;
            }
            
            try
            {
                Transaction tx = catalogConnection.beginTransaction();
                com.sleepycat.db.Transaction txn = Util.getTxn(tx);

                com.sleepycat.db.DatabaseConfig config = new com.sleepycat.db.DatabaseConfig();
                config.setAllowCreate(createDb);
                config.setType(com.sleepycat.db.DatabaseType.BTREE);
                if(comparator != null)
                {
                    config.setBtreeComparator(comparator);
                }

                if(_trace >= 1)
                {
                    _mapKey.communicator.getLogger().trace("Freeze.Map", "opening Db \"" + _mapKey.dbName + "\"");
                }

                _db = connection.dbEnv().getEnv().openDatabase(txn, mapKey.dbName, null, config);

                if(_indices != null)
                {
                    for(int i = 0; i < _indices.length; ++i)
                    {
                        java.util.Comparator indexComparator = null;
                        if(indexComparators != null)
                        {
                            indexComparator = (java.util.Comparator)indexComparators.get(_indices[i].name());
                        }

                        _indices[i].associate(mapKey.dbName, _db, txn, createDb, indexComparator);
                    }
                }

                if(catalogData == null)
                {
                    catalogData = new CatalogData();
                    catalogData.evictor = false;
                    catalogData.key = key;
                    catalogData.value = value;
                    catalog.put(_mapKey.dbName, catalogData);
                }
                
                tx.commit();
                
                //
                // TODO: FREEZE_DB_MODE
                //
            }
            catch(java.io.FileNotFoundException dx)
            {
                cleanupIndices();
                NotFoundException ex = new NotFoundException();
                ex.initCause(dx);
                ex.message = errorPrefix(_mapKey) + "Db.open: " + dx.getMessage();
                throw ex;
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                cleanupIndices();
                DatabaseException ex = new DatabaseException();
                ex.initCause(dx);
                ex.message = errorPrefix(_mapKey) + "Db.open: " + dx.getMessage();
                throw ex;
            }
            finally
            {
                Transaction tx = catalogConnection.currentTransaction();
                if(tx != null)
                {
                    try
                    {
                        tx.rollback();
                    }
                    catch(DatabaseException de)
                    {
                    }
                }
            }
        }
        finally
        {
            catalogConnection.close();
        }

        _refCount = 1;
    }

    private SharedDb(MapKey mapKey, com.sleepycat.db.Environment dbEnv)
        throws com.sleepycat.db.DatabaseException
    {   
        _mapKey = mapKey;
        _key = "string";
        _value = "::Freeze::CatalogData";
        _trace = _mapKey.communicator.getProperties().getPropertyAsInt("Freeze.Trace.Map");
        
        if(_trace >= 1)
        {
            _mapKey.communicator.getLogger().trace("Freeze.Map", "opening Db \"" + _mapKey.dbName + "\"");
        }

        com.sleepycat.db.DatabaseConfig config = new com.sleepycat.db.DatabaseConfig();
        config.setAllowCreate(true);
        config.setType(com.sleepycat.db.DatabaseType.BTREE);
        config.setTransactional(true);

        try
        {
            _db = dbEnv.openDatabase(null, mapKey.dbName, null, config);
        }
        catch(java.io.FileNotFoundException dx)
        {
            //
            // This should never happen
            //
            NotFoundException ex = new NotFoundException();
            ex.initCause(dx);
            ex.message = errorPrefix(_mapKey) + "Db.open: " + dx.getMessage();
            throw ex;
        }       
        _refCount = 1;
    }

    private void
    connectIndices(Map.Index[] indices)
    {
        if(indices != null)
        {
            assert(_indices != null && indices.length == _indices.length);

            for(int i = 0; i < indices.length; ++i)
            {
                indices[i].init(_indices[i]);
            }
        }
    }

    private void
    cleanupIndices()
    {
        if(_indices != null)
        {
            for(int i = 0; i < _indices.length; ++i)
            {
                _indices[i].close();
            }
            _indices = null;
        }
    }

    private static void
    checkTypes(SharedDb sharedDb, String key, String value)
    {
        if(!key.equals(sharedDb._key))
        {
            DatabaseException ex = new DatabaseException();
            ex.message = errorPrefix(sharedDb._mapKey) + sharedDb.dbName() + "'s key type is " + sharedDb._key +
                ", not " + key;
            throw ex;
        }
        if(!value.equals(sharedDb._value))
        {
            DatabaseException ex = new DatabaseException();
            ex.message = errorPrefix(sharedDb._mapKey) + sharedDb.dbName() + "'s value type is " + sharedDb._value +
                ", not " + value;
            throw ex;
        }
    }

    private static String
    errorPrefix(MapKey k)
    {
        return "Freeze DB DbEnv(\"" + k.envName + "\") Db(\"" + k.dbName + "\"): ";
    }

    private static class MapKey
    {
        final String envName;
        final Ice.Communicator communicator;
        final String dbName;
        
        MapKey(String envName, Ice.Communicator communicator, String dbName)
        {
            assert(envName != null);
            assert(communicator != null);
            assert(dbName != null);

            this.envName = envName;
            this.communicator = communicator;
            this.dbName = dbName;
        }

        public boolean
        equals(Object o)
        {   
            try
            {
                MapKey k = (MapKey)o;
                return (dbName.equals(k.dbName)) && (communicator == k.communicator) && envName.equals(k.envName);
            }
            catch(ClassCastException ex)
            {
                communicator.getLogger().trace("Freeze.Map", "equals cast failed");
                return false;
            }
        }
        
        public int hashCode()
        {
            return dbName.hashCode() ^ envName.hashCode() ^ communicator.hashCode();
        }
    }

    private com.sleepycat.db.Database _db;
    private MapKey _mapKey;
    private String _key;
    private String _value;
    private int _refCount = 0;
    private int _trace;
    private Map.Index[] _indices;

    //
    // Hash map of (MapKey, SharedDb)
    //
    private static java.util.Map _map = new java.util.HashMap();
}
