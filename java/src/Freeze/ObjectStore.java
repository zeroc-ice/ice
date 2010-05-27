// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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

            if(catalogData != null && catalogData.evictor == false)
            {
                DatabaseException ex = new DatabaseException();
                ex.message = _evictor.errorPrefix() + evictor.filename() + " is not an evictor database";
                throw ex;
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
                    catalogData = new CatalogData();
                    catalogData.evictor = true;
                    catalog.put(evictor.filename(), catalogData);
                }

                tx.commit();
            }
            catch(java.io.FileNotFoundException dx)
            {
                NotFoundException ex = new NotFoundException();
                ex.initCause(dx);
                ex.message = _evictor.errorPrefix() + "Db.open: " + dx.getMessage();
                throw ex;
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(dx);
                ex.message = _evictor.errorPrefix() + "Db.open: " + dx.getMessage();
                throw ex;
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
            DatabaseException ex = new DatabaseException();
            ex.initCause(dx);
            ex.message = _evictor.errorPrefix() + "Db.close: " + dx.getMessage();
            throw ex;
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

        byte[] key = marshalKey(ident, _communicator);
        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(key);

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
                    DeadlockException ex = new DeadlockException(
                        _evictor.errorPrefix() + "Db.get: " + dx.getMessage(), transaction);
                    ex.initCause(dx);
                    throw ex;
                }
                //
                // Otherwise try again
                //
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                DatabaseException ex = new DatabaseException(_evictor.errorPrefix() + "Db.get: " + dx.getMessage());
                ex.initCause(dx);
                throw ex;
            }
        }
    }

    void
    save(byte[] key, byte[] value, byte status, com.sleepycat.db.Transaction tx)
        throws com.sleepycat.db.DatabaseException
    {
        assert tx != null;

        switch(status)
        {
            case BackgroundSaveEvictorI.created:
            case BackgroundSaveEvictorI.modified:
            {
                com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(key);
                com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry(value);
                com.sleepycat.db.OperationStatus err;
                if(status == BackgroundSaveEvictorI.created)
                {
                    err = _db.putNoOverwrite(tx, dbKey, dbValue);
                }
                else
                {
                    err = _db.put(tx, dbKey, dbValue);
                }
                if(err != com.sleepycat.db.OperationStatus.SUCCESS)
                {
                    throw new DatabaseException();
                }
                break;
            }
            case BackgroundSaveEvictorI.destroyed:
            {
                com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(key);
                com.sleepycat.db.OperationStatus err = _db.delete(tx, dbKey);
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

    static byte[]
    marshalKey(Ice.Identity v, Ice.Communicator communicator)
    {
        IceInternal.BasicStream os =
            new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), false, false);
        v.__write(os);
        IceInternal.Buffer buf = os.prepareWrite();
        byte[] r = new byte[buf.size()];
        buf.b.get(r);
        return r;
    }

    static Ice.Identity
    unmarshalKey(byte[] b, Ice.Communicator communicator)
    {
        IceInternal.BasicStream is =
            new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), false, false);
        is.resize(b.length, true);
        IceInternal.Buffer buf = is.getBuffer();
        buf.b.position(0);
        buf.b.put(b);
        buf.b.position(0);
        Ice.Identity key = new Ice.Identity();
        key.__read(is);
        return key;
    }

    static byte[]
    marshalValue(ObjectRecord v, Ice.Communicator communicator)
    {
        IceInternal.BasicStream os =
            new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), false, false);
        os.startWriteEncaps();
        v.__write(os);
        os.writePendingObjects();
        os.endWriteEncaps();
        IceInternal.Buffer buf = os.prepareWrite();
        byte[] r = new byte[buf.size()];
        buf.b.get(r);
        return r;
    }

    static ObjectRecord
    unmarshalValue(byte[] b, Ice.Communicator communicator)
    {
        IceInternal.BasicStream is =
            new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator), false, false);
        is.sliceObjects(false);
        is.resize(b.length, true);
        IceInternal.Buffer buf = is.getBuffer();
        buf.b.position(0);
        buf.b.put(b);
        buf.b.position(0);
        ObjectRecord rec= new ObjectRecord();
        is.startReadEncaps();
        rec.__read(is);
        is.readPendingObjects();
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
    public Object
    load(Object identObj)
    {
        Ice.Identity ident = (Ice.Identity)identObj;

        byte[] key = marshalKey(ident, _communicator);

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(key);
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
                DatabaseException ex = new DatabaseException();
                ex.initCause(dx);
                ex.message = _evictor.errorPrefix() + "Db.get: " + dx.getMessage();
                throw ex;
            }
        }

        ObjectRecord rec = unmarshalValue(dbValue.getData(), _communicator);
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

        byte[] key = marshalKey(ident, _communicator);

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(key);
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

            DeadlockException ex = new DeadlockException(
                _evictor.errorPrefix() + "Db.get: " + dx.getMessage(), transaction);
            ex.initCause(dx);
            throw ex;
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            DatabaseException ex = new DatabaseException();
            ex.initCause(dx);
            ex.message = _evictor.errorPrefix() + "Db.get: " + dx.getMessage();
            throw ex;
        }

        ObjectRecord rec = unmarshalValue(dbValue.getData(), _communicator);
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

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(marshalKey(ident, _communicator));
        com.sleepycat.db.DatabaseEntry dbValue =
            new com.sleepycat.db.DatabaseEntry(marshalValue(objectRecord, _communicator));

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

            DeadlockException ex = new DeadlockException(
                _evictor.errorPrefix() + "Db.put: " + dx.getMessage(), transaction);
            ex.initCause(dx);
            throw ex;
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            DatabaseException ex = new DatabaseException();
            ex.initCause(dx);
            ex.message = _evictor.errorPrefix() + "Db.put: " + dx.getMessage();
            throw ex;
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

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(marshalKey(ident, _communicator));
        com.sleepycat.db.DatabaseEntry dbValue =
            new com.sleepycat.db.DatabaseEntry(marshalValue(objectRecord, _communicator));

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
                    DeadlockException ex = new DeadlockException(
                        _evictor.errorPrefix() + "Db.putNoOverwrite: " + dx.getMessage(), transaction);
                    ex.initCause(dx);
                    throw ex;
                }
                //
                // Otherwise retry
                //
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(dx);
                ex.message = _evictor.errorPrefix() + "Db.putNoOverwrite: " + dx.getMessage();
                throw ex;
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

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(marshalKey(ident, _communicator));

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
                    DeadlockException ex = new DeadlockException(
                        _evictor.errorPrefix() + "Db.delete: " + dx.getMessage(), transaction);
                    ex.initCause(dx);
                    throw ex;
                }

                //
                // Otherwise retry
                //

            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(dx);
                ex.message = _evictor.errorPrefix() + "Db.delete: " + dx.getMessage();
                throw ex;
            }
        }
    }

    private final IceUtil.Cache _cache;
    private final String _facet;
    private final String _dbName;
    private final EvictorI _evictor;
    private final java.util.List<Index> _indices;
    private final Ice.Communicator _communicator;

    private com.sleepycat.db.Database _db;
    private Ice.Object _sampleServant;
}
