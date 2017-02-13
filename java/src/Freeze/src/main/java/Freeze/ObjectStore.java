// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class ObjectStore implements IceUtil.Store
{
    ObjectStore(String facet, String facetType, boolean createDb, EvictorI evictor, java.util.List<Index> indices,
                boolean populateEmptyIndices)
    {
        _cache = new IceUtil.Cache(this);

        _facet = facet;

        _evictor = evictor;
        _indices = indices;
        _communicator = evictor.communicator();
        _encoding = evictor.encoding();
        _keepStats = false;

        if(facet.equals(""))
        {
            _dbName = EvictorI.defaultDb;
        }
        else
        {
            _dbName = facet;
        }

        if(facetType != null)
        {
            //
            // Create a sample servant with this type
            //
            Ice.ObjectFactory factory = _communicator.findObjectFactory(facetType);
            if(factory == null)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "No object factory registered for type-id '" +
                                            facetType + "'");
            }

            _sampleServant = factory.create(facetType);
        }

        Connection connection = Util.createConnection(_communicator, evictor.dbEnv().getEnvName());

        try
        {
            Catalog catalog = new Catalog(connection, Util.catalogName(), true);
            CatalogData catalogData = catalog.get(evictor.filename());

            if(catalogData != null)
            {
                if(catalogData.evictor)
                {
                    _keepStats = catalogData.value.isEmpty();
                }
                else
                {
                    DatabaseException ex = new DatabaseException();
                    ex.message = _evictor.errorPrefix() + evictor.filename() + " is not an evictor database";
                    throw ex;
                }
            }

            com.sleepycat.db.Environment dbEnv = evictor.dbEnv().getEnv();

            //
            // TODO: FREEZE_DB_MODE
            //
            com.sleepycat.db.DatabaseConfig config = new com.sleepycat.db.DatabaseConfig();
            config.setType(com.sleepycat.db.DatabaseType.BTREE);
            config.setAllowCreate(createDb);

            Ice.Properties properties = _evictor.communicator().getProperties();
            String propPrefix = "Freeze.Evictor." + _evictor.filename() + ".";

            int btreeMinKey = properties.getPropertyAsInt(propPrefix + _dbName + ".BtreeMinKey");
            if(btreeMinKey > 2)
            {
                if(_evictor.trace() >= 1)
                {
                    _evictor.communicator().getLogger().trace(
                        "Freeze.Evictor", "Setting \"" + _evictor.filename() + "." + _dbName +
                        "\"'s btree minkey to " + btreeMinKey);
                }
                config.setBtreeMinKey(btreeMinKey);
            }

            boolean checksum = properties.getPropertyAsInt(propPrefix + "Checksum") > 0;
            if(checksum)
            {
                if(_evictor.trace() >= 1)
                {
                   _evictor.communicator().getLogger().trace(
                        "Freeze.Evictor", "Turning checksum on for \"" + _evictor.filename()  + "\"");
                }

                config.setChecksum(true);
            }

            int pageSize = properties.getPropertyAsInt(propPrefix + "PageSize");
            if(pageSize > 0)
            {
                if(_evictor.trace() >= 1)
                {
                    _evictor.communicator().getLogger().trace(
                        "Freeze.Evictor", "Setting \"" + _evictor.filename() + "\"'s pagesize to " + pageSize);
                }
                config.setPageSize(pageSize);
            }

            try
            {
                Transaction tx = connection.beginTransaction();
                com.sleepycat.db.Transaction txn = Util.getTxn(tx);

                _db = dbEnv.openDatabase(txn, evictor.filename(), _dbName, config);

                for(Index index : _indices)
                {
                    index.associate(this, txn, createDb, populateEmptyIndices);
                }

                if(catalogData == null)
                {
                    catalogData = new CatalogData(true, "::Ice::Identity", "Object");
                    catalog.put(evictor.filename(), catalogData);
                }

                tx.commit();
            }
            catch(java.io.FileNotFoundException dx)
            {
                throw new NotFoundException(_evictor.errorPrefix() + "Db.open: " + dx.getMessage(), dx);
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "Db.open: " + dx.getMessage(), dx);
            }
            finally
            {
                Transaction tx = connection.currentTransaction();
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
            connection.close();
        }
    }

    void
    close()
    {
        try
        {
            _db.close();

            for(Index index : _indices)
            {
                index.close();
            }
            _indices.clear();
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            throw new DatabaseException(_evictor.errorPrefix() + "Db.close: " + dx.getMessage(), dx);
        }
        _db = null;
    }

    boolean
    dbHasObject(Ice.Identity ident, TransactionI transaction)
    {
        com.sleepycat.db.Transaction tx = null;

        if(transaction != null)
        {
            tx = transaction.dbTxn();
            if(tx == null)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "inactive transaction");
            }
        }

        com.sleepycat.db.DatabaseEntry dbKey = marshalKey(ident, _communicator, _encoding);

        //
        // Keep 0 length since we're not interested in the data
        //
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        dbValue.setPartial(true);

        for(;;)
        {
            try
            {
                com.sleepycat.db.OperationStatus err = _db.get(tx, dbKey, dbValue, null);

                if(err == com.sleepycat.db.OperationStatus.SUCCESS)
                {
                    return true;
                }
                else if(err == com.sleepycat.db.OperationStatus.NOTFOUND)
                {
                    return false;
                }
                else
                {
                    throw new DatabaseException();
                }
            }
            catch(com.sleepycat.db.DeadlockException dx)
            {
                if(_evictor.deadlockWarning())
                {
                    _communicator.getLogger().warning("Deadlock in Freeze.ObjectStore.dhHasObject while reading " +
                                                      "Db \"" + _evictor.filename() + "/" + _dbName + "\"");
                }

                if(tx != null)
                {
                    throw new DeadlockException(_evictor.errorPrefix() + "Db.get: " + dx.getMessage(), transaction, dx);
                }
                //
                // Otherwise try again
                //
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "Db.get: " + dx.getMessage(), dx);
            }
        }
    }

    void
    save(com.sleepycat.db.DatabaseEntry key, com.sleepycat.db.DatabaseEntry value, byte status,
         com.sleepycat.db.Transaction tx)
        throws com.sleepycat.db.DatabaseException
    {
        assert tx != null;

        switch(status)
        {
            case BackgroundSaveEvictorI.created:
            case BackgroundSaveEvictorI.modified:
            {
                com.sleepycat.db.OperationStatus err;
                if(status == BackgroundSaveEvictorI.created)
                {
                    err = _db.putNoOverwrite(tx, key, value);
                }
                else
                {
                    err = _db.put(tx, key, value);
                }
                if(err != com.sleepycat.db.OperationStatus.SUCCESS)
                {
                    throw new DatabaseException();
                }
                break;
            }
            case BackgroundSaveEvictorI.destroyed:
            {
                com.sleepycat.db.OperationStatus err = _db.delete(tx, key);
                if(err != com.sleepycat.db.OperationStatus.SUCCESS)
                {
                    throw new DatabaseException();
                }
                break;
            }
            default:
            {
                assert false;
            }
        }
    }

    static com.sleepycat.db.DatabaseEntry
    marshalKey(Ice.Identity v, Ice.Communicator communicator, Ice.EncodingVersion encoding)
    {
        IceInternal.BasicStream os = 
            new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), encoding, false);
        v.__write(os);
        return new com.sleepycat.db.DatabaseEntry(os.prepareWrite().b);
    }

    static Ice.Identity
    unmarshalKey(com.sleepycat.db.DatabaseEntry e, Ice.Communicator communicator, Ice.EncodingVersion encoding)
    {
        IceInternal.BasicStream is;
        if(e.getDataNIO() != null)
        {
            is = new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), encoding, e.getDataNIO());
        }
        else
        {
            is = new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), encoding, e.getData());
        }
        Ice.Identity key = new Ice.Identity();
        key.__read(is);
        return key;
    }

    static com.sleepycat.db.DatabaseEntry
    marshalValue(ObjectRecord v, Ice.Communicator communicator, Ice.EncodingVersion encoding, boolean keepStats)
    {
        IceInternal.BasicStream os =
            new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), encoding, false);
        os.startWriteEncaps();

        if(keepStats)
        {
            v.__write(os);
        }
        else
        {
            os.writeObject(v.servant);
        }
        os.writePendingObjects();
        os.endWriteEncaps();
        return new com.sleepycat.db.DatabaseEntry(os.prepareWrite().b);
    }

    static ObjectRecord
    unmarshalValue(com.sleepycat.db.DatabaseEntry e, Ice.Communicator communicator, Ice.EncodingVersion encoding,
                   boolean keepStats)
    {
        IceInternal.BasicStream is;
        if(e.getDataNIO() != null)
        {
            is = new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), encoding, e.getDataNIO());
        }
        else
        {
            is = new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), encoding, e.getData());
        }
        is.sliceObjects(false);
        ObjectRecord rec = new ObjectRecord();
        is.startReadEncaps();
        if(keepStats)
        {
            rec.__read(is);
            is.readPendingObjects();
        }
        else
        {
            Ice.ObjectHolder holder = new Ice.ObjectHolder();
            is.readObject(holder);
            is.readPendingObjects();
            rec.servant = holder.value;
        }
        is.endReadEncaps();
        return rec;
    }

    final IceUtil.Cache
    cache()
    {
        return _cache;
    }

    final com.sleepycat.db.Database
    db()
    {
        return _db;
    }

    final Ice.Communicator
    communicator()
    {
        return _communicator;
    }

    final Ice.EncodingVersion
    encoding()
    {
        return _encoding;
    }

    final boolean
    keepStats()
    {
        return _keepStats;
    }

    final EvictorI
    evictor()
    {
        return _evictor;
    }

    final String
    facet()
    {
        return _facet;
    }

    final String
    dbName()
    {
        return _dbName;
    }

    final Ice.Object
    sampleServant()
    {
        return _sampleServant;
    }

    //
    // Load a servant from the database; will end up in the cache associated with
    // this ObjectStore. This load is not transactional.
    //
    @Override
    public Object
    load(Object identObj)
    {
        Ice.Identity ident = (Ice.Identity)identObj;

        com.sleepycat.db.DatabaseEntry dbKey = marshalKey(ident, _communicator, _encoding);
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();

        for(;;)
        {
            try
            {
                com.sleepycat.db.OperationStatus rs = _db.get(null, dbKey, dbValue, null);

                if(rs == com.sleepycat.db.OperationStatus.NOTFOUND)
                {
                    return null;
                }
                else if (rs != com.sleepycat.db.OperationStatus.SUCCESS)
                {
                    assert false;
                    throw new DatabaseException();
                }
                break;
            }
            catch(com.sleepycat.db.DeadlockException dx)
            {
                if(_evictor.deadlockWarning())
                {
                    _communicator.getLogger().warning("Deadlock in Freeze.ObjectStore.load while reading Db \"" +
                                                      _evictor.filename() + "/" + _dbName + "\"; retrying...");
                }

                //
                // Ignored, try again
                //
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "Db.get: " + dx.getMessage(), dx);
            }
        }

        ObjectRecord rec = unmarshalValue(dbValue, _communicator, _encoding, _keepStats);
        _evictor.initialize(ident, _facet, rec.servant);

        Object result = _evictor.createEvictorElement(ident, rec, this);
        return result;
    }

    //
    // Load a servant from the database using the given transaction; this servant
    // is NOT cached in the ObjectStore associated cache
    //
    ObjectRecord
    load(Ice.Identity ident, TransactionI transaction)
    {
        if(transaction == null)
        {
            throw new DatabaseException(_evictor.errorPrefix() + "no active transaction");
        }
        com.sleepycat.db.Transaction tx = transaction.dbTxn();
        if(tx == null)
        {
            throw new DatabaseException(_evictor.errorPrefix() + "inactive transaction");
        }

        com.sleepycat.db.DatabaseEntry dbKey = marshalKey(ident, _communicator, _encoding);
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();

        try
        {
            com.sleepycat.db.OperationStatus rs = _db.get(tx, dbKey, dbValue, null);

            if(rs == com.sleepycat.db.OperationStatus.NOTFOUND)
            {
                return null;
            }
            else if(rs != com.sleepycat.db.OperationStatus.SUCCESS)
            {
                assert false;
                throw new DatabaseException();
            }
        }
        catch(com.sleepycat.db.DeadlockException dx)
        {
            if(_evictor.deadlockWarning())
            {
                _communicator.getLogger().warning("Deadlock in Freeze.ObjectStore.load while reading Db \"" +
                                                  _evictor.filename() + "/" + _dbName + "\"");
            }

            throw new DeadlockException(_evictor.errorPrefix() + "Db.get: " + dx.getMessage(), transaction, dx);
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            throw new DatabaseException(_evictor.errorPrefix() + "Db.get: " + dx.getMessage(), dx);
        }

        ObjectRecord rec = unmarshalValue(dbValue, _communicator, _encoding, _keepStats);
        _evictor.initialize(ident, _facet, rec.servant);
        return rec;
    }

    void
    update(Ice.Identity ident, ObjectRecord objectRecord, TransactionI transaction)
    {
        if(transaction == null)
        {
            throw new DatabaseException(_evictor.errorPrefix() + "no active transaction");
        }
        com.sleepycat.db.Transaction tx = transaction.dbTxn();
        if(tx == null)
        {
            throw new DatabaseException(_evictor.errorPrefix() + "inactive transaction");
        }

        if(_sampleServant != null && !objectRecord.servant.ice_id().equals(_sampleServant.ice_id()))
        {
            String msg = _evictor.errorPrefix() + "Attempting to save a '" + objectRecord.servant.ice_id()
                + "' servant in a database of '" + _sampleServant.ice_id() + "' servants";

            throw new DatabaseException(msg);
        }

        com.sleepycat.db.DatabaseEntry dbKey = marshalKey(ident, _communicator, _encoding);
        com.sleepycat.db.DatabaseEntry dbValue = marshalValue(objectRecord, _communicator, _encoding, _keepStats);

        try
        {
            com.sleepycat.db.OperationStatus err = _db.put(tx, dbKey, dbValue);
            if(err != com.sleepycat.db.OperationStatus.SUCCESS)
            {
                throw new DatabaseException();
            }
        }
        catch(com.sleepycat.db.DeadlockException dx)
        {
            if(_evictor.deadlockWarning())
            {
                _communicator.getLogger().warning("Deadlock in Freeze.ObjectStore.update while updating Db \"" +
                                                  _evictor.filename() + "/" + _dbName + "\"");
            }

            throw new DeadlockException(_evictor.errorPrefix() + "Db.put: " + dx.getMessage(), transaction, dx);
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            throw new DatabaseException(_evictor.errorPrefix() + "Db.put: " + dx.getMessage(), dx);
        }
    }

    boolean
    insert(Ice.Identity ident, ObjectRecord objectRecord, TransactionI transaction)
    {
        com.sleepycat.db.Transaction tx = null;

        if(transaction != null)
        {
            tx = transaction.dbTxn();
            if(tx == null)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "invalid transaction");
            }
        }

        com.sleepycat.db.DatabaseEntry dbKey = marshalKey(ident, _communicator, _encoding);
        com.sleepycat.db.DatabaseEntry dbValue = marshalValue(objectRecord, _communicator, _encoding, _keepStats);

        if(_sampleServant != null && !objectRecord.servant.ice_id().equals(_sampleServant.ice_id()))
        {
            String msg = _evictor.errorPrefix() + "Attempting to save a '" + objectRecord.servant.ice_id() +
                "' servant in a database of '" + _sampleServant.ice_id() + "' servants";

            throw new DatabaseException(msg);
        }

        for(;;)
        {
            try
            {
                return _db.putNoOverwrite(tx, dbKey, dbValue) == com.sleepycat.db.OperationStatus.SUCCESS;
            }
            catch(com.sleepycat.db.DeadlockException dx)
            {
                if(_evictor.deadlockWarning())
                {
                    _communicator.getLogger().warning("Deadlock in Freeze.ObjectStore.update while updating Db \"" +
                                                      _evictor.filename() + "/" + _dbName + "\"");
                }

                if(tx != null)
                {
                    throw new DeadlockException(_evictor.errorPrefix() + "Db.putNoOverwrite: " + dx.getMessage(),
                                                transaction, dx);
                }
                //
                // Otherwise retry
                //
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "Db.putNoOverwrite: " + dx.getMessage(), dx);
            }
        }
    }

    boolean
    remove(Ice.Identity ident, TransactionI transaction)
    {
        com.sleepycat.db.Transaction tx = null;

        if(transaction != null)
        {
            tx = transaction.dbTxn();
            if(tx == null)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "invalid transaction");
            }
        }

        com.sleepycat.db.DatabaseEntry dbKey = marshalKey(ident, _communicator, _encoding);

        for(;;)
        {
            try
            {
                return _db.delete(tx, dbKey) == com.sleepycat.db.OperationStatus.SUCCESS;
            }
            catch(com.sleepycat.db.DeadlockException dx)
            {
                if(_evictor.deadlockWarning())
                {
                    _communicator.getLogger().warning("Deadlock in Freeze.ObjectStore.remove while updating Db \"" +
                                                      _evictor.filename() + "/" + _dbName + "\"");
                }

                if(tx != null)
                {
                    throw new DeadlockException(_evictor.errorPrefix() + "Db.delete: " + dx.getMessage(), transaction,
                                                dx);
                }

                //
                // Otherwise retry
                //

            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                throw new DatabaseException(_evictor.errorPrefix() + "Db.delete: " + dx.getMessage(), dx);
            }
        }
    }

    private final IceUtil.Cache _cache;
    private final String _facet;
    private final String _dbName;
    private final EvictorI _evictor;
    private final java.util.List<Index> _indices;
    private final Ice.Communicator _communicator;
    private final Ice.EncodingVersion _encoding;
    private boolean _keepStats;

    private com.sleepycat.db.Database _db;
    private Ice.Object _sampleServant;
}
