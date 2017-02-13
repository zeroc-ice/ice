// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

//
// A MapDb represents the Db object underneath Freeze Maps. Several Freeze Maps often
// share the very same MapDb object; SharedDbEnv manages these shared MapDb objects.
//

public class MapDb
{
    public
    MapDb(ConnectionI connection, String dbName, String key, String value, java.util.Comparator<?> comparator,
          MapIndex[] indices, boolean createDb)
    {
        _communicator = connection.communicator();
        _dbName = dbName;
        _errorPrefix = "Freeze DB DbEnv(\"" + connection.dbEnv().getEnvName() + "\") Db(\"" + dbName + "\"): ";
        _indices = indices;
        _trace = connection.trace();

        Catalog catalog = new Catalog(connection, Util.catalogName(), true);
        CatalogData catalogData = catalog.get(_dbName);
        if(catalogData != null)
        {
            if(catalogData.evictor)
            {
                throw new DatabaseException(_errorPrefix + "is not an evictor");
            }
            _key = catalogData.key;
            _value = catalogData.value;
            checkTypes(key, value);
        }
        else
        {
            _key = key;
            _value = value;
        }

        com.sleepycat.db.DatabaseConfig config = new com.sleepycat.db.DatabaseConfig();

        config.setAllowCreate(createDb);
        config.setType(com.sleepycat.db.DatabaseType.BTREE);

        if(comparator != null)
        {
            config.setBtreeComparator(comparator);
        }
        Ice.Properties properties = _communicator.getProperties();
        String propPrefix = "Freeze.Map." + _dbName + ".";

        int btreeMinKey = properties.getPropertyAsInt(propPrefix + "BtreeMinKey");
        if(btreeMinKey > 2)
        {
            if(_trace >= 1)
            {
                _communicator.getLogger().trace(
                    "Freeze.Map", "Setting \"" + _dbName + "\"'s btree minkey to " + btreeMinKey);
            }
            config.setBtreeMinKey(btreeMinKey);
        }

        boolean checksum = properties.getPropertyAsInt(propPrefix + "Checksum") > 0;
        if(checksum)
        {
            if(_trace >= 1)
            {
                _communicator.getLogger().trace("Freeze.Map", "Turning checksum on for \"" + _dbName + "\"");
            }

            config.setChecksum(true);
        }

        int pageSize = properties.getPropertyAsInt(propPrefix + "PageSize");
        if(pageSize > 0)
        {
            if(_trace >= 1)
            {
                _communicator.getLogger().trace("Freeze.Map", "Setting \"" + _dbName + "\"'s pagesize to " + pageSize);
            }
            config.setPageSize(pageSize);
        }

        if(_trace >= 1)
        {
            _communicator.getLogger().trace("Freeze.Map", "opening Db \"" + _dbName + "\"");
        }

        Transaction tx = connection.currentTransaction();
        boolean ownTx = (tx == null);

        for(;;)
        {
            try
            {
                if(ownTx)
                {
                    tx = null;
                    tx = connection.beginTransaction();
                }

                com.sleepycat.db.Transaction txn = Util.getTxn(tx);

                _db = connection.dbEnv().getEnv().openDatabase(txn, _dbName, null, config);

                String[] oldIndices = null;
                java.util.List<String> newIndices = new java.util.LinkedList<String>();

                CatalogIndexList catalogIndexList = new CatalogIndexList(connection, Util.catalogIndexListName(), true);

                if(createDb)
                {
                    oldIndices = catalogIndexList.get(_dbName);
                }

                if(_indices != null)
                {
                    for(MapIndex i : _indices)
                    {
                        String indexName = i.name();

                        i.associate(_dbName, _db, txn, createDb);

                        if(createDb)
                        {
                            if(oldIndices != null)
                            {
                                int j = java.util.Arrays.asList(oldIndices).indexOf(indexName);
                                if(j != -1)
                                {
                                    oldIndices[j] = null;
                                }
                            }
                            newIndices.add(indexName);
                        }
                    }
                }

                if(catalogData == null)
                {
                    catalogData = new CatalogData();
                    catalogData.evictor = false;
                    catalogData.key = key;
                    catalogData.value = value;
                    catalog.put(_dbName, catalogData);
                }

                if(createDb)
                {
                    boolean indexRemoved = false;

                    if(oldIndices != null)
                    {
                        //
                        // Remove old indices and write the new ones
                        //
                        for(String index : oldIndices)
                        {
                            if(index != null)
                            {
                                if(_trace >= 1)
                                {
                                    _communicator.getLogger().trace("Freeze.Map", "removing old index \"" + index +
                                                                    "\" on Db \"" +  _dbName + "\"");
                                }

                                indexRemoved = true;

                                try
                                {
                                    connection.removeMapIndex(_dbName, index);
                                }
                                catch(IndexNotFoundException ife)
                                {
                                    // Ignored

                                    if(_trace >= 1)
                                    {
                                        _communicator.getLogger().trace("Freeze.Map", "index \"" + index +
                                                                        "\" on Db \"" + _dbName + "\" does not exist");
                                    }
                                }
                            }
                        }
                    }

                    int oldSize = oldIndices == null ? 0 : oldIndices.length;

                    if(indexRemoved || newIndices.size() != oldSize)
                    {
                        if(newIndices.size() == 0)
                        {
                            catalogIndexList.remove(_dbName);
                            if(_trace >= 1)
                            {
                                _communicator.getLogger().trace(
                                    "Freeze.Map", "Removed catalogIndexList entry for Db \"" + _dbName + "\"");
                            }
                        }
                        else
                        {
                            catalogIndexList.put(_dbName, newIndices.toArray(new String[0]));
                            if(_trace >= 1)
                            {
                                _communicator.getLogger().trace(
                                    "Freeze.Map", "Updated catalogIndexList entry for Db \"" + _dbName + "\"");
                            }
                        }
                    }
                }

                if(ownTx)
                {
                    try
                    {
                        tx.commit();
                    }
                    finally
                    {
                        tx = null;
                    }
                }
                break; // for(;;)
            }
            catch(java.io.FileNotFoundException dx)
            {
                clearIndices();
                throw new NotFoundException(_errorPrefix + "Db.open: " + dx.getMessage(), dx);
            }
            catch(com.sleepycat.db.DeadlockException dx)
            {
                if(ownTx)
                {
                    if(connection.deadlockWarning())
                    {
                        connection.communicator().getLogger().warning("Deadlock in Freeze.Shared.Shared on Db \"" +
                                                                      _dbName + "\"; retrying ...");
                    }
                    tx = null;
                }
                else
                {
                    clearIndices();
                    throw new DeadlockException(_errorPrefix + "Db.open: " + dx.getMessage(), tx, dx);
                }
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                clearIndices();
                throw new DatabaseException(_errorPrefix + "Db.open: " + dx.getMessage(), dx);
            }
            finally
            {
                if(ownTx && tx != null)
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
    }

    //
    // The constructor for catalogs
    //

    MapDb(Ice.Communicator communicator, String envName, String dbName, String key,
          String value, com.sleepycat.db.Environment dbEnv)
        throws com.sleepycat.db.DatabaseException
    {
        _communicator = communicator;
        _dbName = dbName;
        _errorPrefix = "Freeze DB DbEnv(\"" + envName + "\") Db(\"" + dbName + "\"): ";
        _key = key;
        _value = value;
        _trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Map");

        if(_trace >= 1)
        {
            _communicator.getLogger().trace("Freeze.Map", "opening Db \"" + _dbName + "\"");
        }

        com.sleepycat.db.DatabaseConfig config = new com.sleepycat.db.DatabaseConfig();
        config.setAllowCreate(true);
        config.setType(com.sleepycat.db.DatabaseType.BTREE);
        config.setTransactional(true);

        try
        {
            _db = dbEnv.openDatabase(null, _dbName, null, config);
        }
        catch(java.io.FileNotFoundException dx)
        {
            //
            // This should never happen
            //
            throw new NotFoundException(_errorPrefix + "Db.open: " + dx.getMessage(), dx);
        }
    }

    public void
    close()
    {
        if(_trace >= 1)
        {
            _communicator.getLogger().trace("Freeze.Map", "closing Db \"" + _dbName + "\"");
        }

        clearIndices();

        if(_db != null)
        {
            try
            {
                _db.close();
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                throw new DatabaseException(_errorPrefix + "close: " + dx.getMessage(), dx);
            }
            finally
            {
                _db = null;
            }
        }
    }

    void
    connectIndices(MapIndex[] indices)
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

    void
    clearIndices()
    {
        if(_indices != null)
        {
            for(MapIndex i : _indices)
            {
                i.close();
            }
            _indices = null;
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
        return _dbName;
    }

    void
    checkTypes(String key, String value)
    {
        if(!key.equals(_key))
        {
            throw new DatabaseException(_errorPrefix + _dbName + "'s key type is " + _key + ", not " + key);
        }

        if(!value.equals(_value))
        {
            throw new DatabaseException(_errorPrefix + _dbName + "'s value type is " + _value + ", not " + value);
        }
    }

    private com.sleepycat.db.Database _db;
    private final Ice.Communicator _communicator;
    private final String _dbName;
    private final String _errorPrefix;
    private String _key;
    private String _value;
    private final int _trace;
    private MapIndex[] _indices;
}
