// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

public abstract class Map extends java.util.AbstractMap 
    implements java.util.SortedMap, KeyCodec
{
    public abstract byte[] encodeValue(Object o, Ice.Communicator communicator);
    public abstract Object decodeValue(byte[] b, Ice.Communicator communicator);

    public
    Map(Connection connection, String dbName, String key, String value, 
        boolean createDb, java.util.Comparator comparator)
    {
        _connection = (ConnectionI) connection;
        _comparator = (comparator == null) ? null : new Comparator(comparator);
                
        _errorPrefix = "Freeze DB DbEnv(\"" + _connection.envName() + "\") Db(\"" + dbName + "\"): ";
        _trace = _connection.trace();
        
        init(null, dbName, key, value, createDb, null);
    }

    protected
    Map(Connection connection, String dbName, java.util.Comparator comparator)
    {
        _connection = (ConnectionI) connection;
        _comparator = (comparator == null) ? null : new Comparator(comparator);

        _errorPrefix = "Freeze DB DbEnv(\"" + _connection.envName() + "\") Db(\"" + dbName + "\"): ";
        _trace = _connection.trace();
    }

    protected void
    init(Freeze.Map.Index[] indices, String dbName, 
         String key, String value, boolean createDb, java.util.Map indexComparators)
    {
        _db = Freeze.SharedDb.get(_connection, dbName, key, 
                                  value, indices, createDb, _comparator,
                                  indexComparators);
        _token = _connection.registerMap(this);

        if(indices != null)
        {
            for(int i = 0; i < indices.length; ++i)
            {
                _indexMap.put(indices[i].name(), indices[i]);
            }
        }
    }

    public void
    close()
    {
        close(false);
    }

    //
    // SortedMap methods
    //

    public java.util.Comparator
    comparator()
    {
        if(_comparator == null)
        {
            return null;
        }
        else
        {
            //
            // Return's the user's comparator, not the DB comparator.
            //
            return _comparator.comparator();
        }
    }

    public Object firstKey()
    {
        return firstKey(null, null);
    }
    
    public Object lastKey()
    {
        return lastKey(null, null);
    }

    Object firstKey(Object fromKey, Object toKey)
    {
        byte[] fk = fromKey == null ? null :
            encodeKey(fromKey, _connection.communicator());

        byte[] k = getFirstOrLastKey(_db.db(), _db.dbName(), fk, true);
        if(k == null)
        {
            throw new NoSuchElementException();
        }
        else
        {
            Object key = decodeKey(k, _connection.communicator());
            if(toKey != null && comparator().compare(key, toKey) >= 0)
            {
                throw new NoSuchElementException();
            }
            return key;
        }
    }

    Object lastKey(Object fromKey, Object toKey)
    {
        byte[] tk = toKey == null ? null :
            encodeKey(toKey, _connection.communicator());

        byte[] k = getFirstOrLastKey(_db.db(), _db.dbName(), tk, false);
        if(k == null)
        {
            throw new NoSuchElementException();
        }
        else
        {
            Object key = decodeKey(k, _connection.communicator());
            if(fromKey != null && comparator().compare(fromKey, key) > 0)
            {
                throw new NoSuchElementException();
            }
            return key;
        }
    }

    public java.util.SortedMap headMap(Object toKey)
    {
        if(toKey == null)
        {
            throw new NullPointerException();
        }
        if(_comparator == null)
        {
            throw new UnsupportedOperationException();
        }

        return new SubMap(this, null, toKey);
    }
    
    public java.util.SortedMap tailMap(Object fromKey)
    {
        if(fromKey == null)
        {
            throw new NullPointerException();
        }
        if(_comparator == null)
        {
            throw new UnsupportedOperationException();
        }

        return new SubMap(this, fromKey, null);
    } 
   
    public java.util.SortedMap subMap(Object fromKey, Object toKey)
    {
        if(fromKey == null || toKey == null )
        {
            throw new NullPointerException();
        }
        if(_comparator == null)
        {
            throw new UnsupportedOperationException();
        }
        return new SubMap(this, fromKey, toKey);
    }

    
    //
    // Additional non-standard xxMapForIndex methods
    //
    public java.util.SortedMap headMapForIndex(String indexName, Object toKey)
    {
        if(toKey == null)
        {
            throw new NullPointerException();
        }
        
        Map.Index index = (Map.Index)_indexMap.get(indexName);
        if(index == null)
        {
            throw new IllegalArgumentException("Can't find index '" + indexName + "'");
        }
        else if(index.comparator() == null)
        {
            throw new IllegalArgumentException("Index '" + indexName + "' has no user-defined comparator");
        }
        return new SubMap(index, null, toKey);
    }
    
    public java.util.SortedMap tailMapForIndex(String indexName, Object fromKey)
    {
        if(fromKey == null)
        {
            throw new NullPointerException();
        }
        Map.Index index = (Map.Index)_indexMap.get(indexName);
        if(index == null)
        {
            throw new IllegalArgumentException("Can't find index '" + indexName + "'");
        }
        else if(index.comparator() == null)
        {
            throw new IllegalArgumentException("Index '" + indexName + "' has no user-defined comparator");
        }
        return new SubMap(index, fromKey, null);
    } 
   
    public java.util.SortedMap subMapForIndex(String indexName, Object fromKey, Object toKey)
    {
        if(fromKey == null || toKey == null )
        {
            throw new NullPointerException();
        }
        Map.Index index = (Map.Index)_indexMap.get(indexName);
        if(index == null)
        {
            throw new IllegalArgumentException("Can't find index '" + indexName + "'");
        }
        else if(index.comparator() == null)
        {
            throw new IllegalArgumentException("Index '" + indexName + "' has no user-defined comparator");
        }
        return new SubMap(index, fromKey, toKey);
    }

    public java.util.SortedMap mapForIndex(String indexName)
    {
        Map.Index index = (Map.Index)_indexMap.get(indexName);
        if(index == null)
        {
            throw new IllegalArgumentException("Can't find index '" + indexName + "'");
        }
        else if(index.comparator() == null)
        {
            throw new IllegalArgumentException("Index '" + indexName + "' has no user-defined comparator");
        }
        return new SubMap(index, null, null);
    }
    
    //
    // Plain Map methods
    //
    public int
    size()
    {
        if(_db == null)
        {
            DatabaseException ex = new DatabaseException();
            ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
            throw ex;
        }

        //
        // The number of records cannot be cached and then adjusted by
        // the member functions since the map would no longer work in
        // the presence of transactions - if a record is added (and
        // the size adjusted) and the transaction aborted then the
        // cached map size() would be incorrect.
        //
        
        //
        // TODO: DB_FAST_STAT doesn't seem to do what the
        // documentation says...
        //
        try
        {
            com.sleepycat.db.StatsConfig config = new com.sleepycat.db.StatsConfig();
            //
            // TODO: DB_FAST_STAT doesn't seem to do what the
            // documentation says...
            //      
            //config.setFast(true);
            com.sleepycat.db.BtreeStats s = (com.sleepycat.db.BtreeStats)_db.db().getStats(null, config);
            return s.getNumData();
        }
        catch(com.sleepycat.db.DatabaseException e)
        {
            DatabaseException ex = new DatabaseException();
            ex.initCause(e);
            ex.message = _errorPrefix + "Db.stat: " + e.getMessage();
            throw ex;
        }
    }

    public boolean
    containsValue(Object value)
    {
        for(;;)
        {
            EntryIterator p = null;
            try
            {
                p = (EntryIterator)entrySet().iterator();

                if(value == null)
                {
                    while(p.hasNext())
                    {
                        Entry e = (Entry)p.next();
                        if(e.getValue() == null)
                        {
                            return true;
                        }
                    }
                } 
                else 
                {
                    while(p.hasNext()) 
                    {
                        Entry e = (Entry)p.next();
                        if(value.equals(e.getValue()))
                        {
                            return true;
                        }
                    }
                }
                return false;
            }
            catch(DeadlockException e)
            {
                if(_connection.dbTxn() != null)
                {
                    throw e;
                }
                else
                {
                    if(_connection.deadlockWarning())
                    {
                        _connection.communicator().getLogger().warning("Deadlock in Freeze.Map.containsValue while " +
                                                                       "iterating over Db \"" + _db.dbName()  +
                                                                       "\"; retrying...");
                    }

                    //
                    // Try again
                    //
                }
            }
            finally
            {
                if(p != null)
                {
                    p.close();
                }
            }
        }
    }

    public boolean
    containsKey(Object key)
    {
        if(_db == null)
        {
            DatabaseException ex = new DatabaseException();
            ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
            throw ex;
        }

        byte[] k = encodeKey(key, _connection.communicator());

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        dbValue.setPartial(true);

        if(_trace >= 2)
        {
            _connection.communicator().getLogger().trace("Freeze.Map", "checking key in Db \"" + _db.dbName() + "\"");
        }

        for(;;)
        {
            try
            {
                return _db.db().get(_connection.dbTxn(), dbKey, dbValue, null)
                    == com.sleepycat.db.OperationStatus.SUCCESS;
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                if(_connection.dbTxn() != null)
                {
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
                    throw ex;
                }
                else
                {
                    if(_connection.deadlockWarning())
                    {
                        _connection.communicator().getLogger().warning(
                            "Deadlock in Freeze.Map.containsKey while " +
                            "reading Db \"" + _db.dbName() +
                            "\"; retrying...");
                    }
                    //
                    // Try again
                    //
                }
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(e);
                ex.message = _errorPrefix + "Db.get: " + e.getMessage();
                throw ex;
            }
        }
    }

    public Object
    get(Object key)
    {
        byte[] k = encodeKey(key, _connection.communicator());
        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
        byte[] v = getImpl(dbKey);
        if(v == null)
        {
            return null;
        }
        else
        {
            return decodeValue(v, _connection.communicator());
        }
    }

    public Object
    put(Object key, Object value)
    {
        byte[] k = encodeKey(key, _connection.communicator());
        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
        byte[] v = getImpl(dbKey);
        Object o = null;
        if(v != null)
        {
            o = decodeValue(v, _connection.communicator());
        }
        putImpl(dbKey, value);
        return o;
    }

    public Object
    remove(Object key)
    {
        byte[] k = encodeKey(key, _connection.communicator());
        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
        byte[] v = getImpl(dbKey);
        
        if(v != null && removeImpl(dbKey))
        {
            return decodeValue(v, _connection.communicator());
        }
        else
        {
            return null;
        }
    }

    //
    // Proprietary API calls. These are much faster than the
    // corresponding Java collections API methods since the unwanted
    // reads are avoided.
    //
    public void
    fastPut(Object key, Object value)
    {
        byte[] k = encodeKey(key, _connection.communicator());
        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
        putImpl(dbKey, value);
    }

    //
    // Returns true if the record was removed, false otherwise.
    //
    public boolean
    fastRemove(Object key)
    {
        byte[] k = encodeKey(key, _connection.communicator());
        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
        return removeImpl(dbKey);
    }

    public void
    clear()
    {
        if(_db == null)
        {
            DatabaseException ex = new DatabaseException();
            ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
            throw ex;
        }

        com.sleepycat.db.Transaction txn = _connection.dbTxn();

        for(;;)
        {
            try
            {
                _db.db().truncate(txn, false);
                break;
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                if(txn != null)
                {
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Db.truncate: " + e.getMessage();
                    throw ex;
                }
                else
                {
                    if(_connection.deadlockWarning())
                    {
                        _connection.communicator().getLogger().warning("Deadlock in Freeze.Map.clear on Db \"" +
                                                                       _db.dbName() + "\"; retrying...");
                    }
                    
                    //
                    // Try again
                    //
                }
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(e);
                ex.message = _errorPrefix + "Db.truncate: " + e.getMessage();
                throw ex;
            }
        }
    }
        
    public java.util.Set
    entrySet()
    {
        if(_entrySet == null)
        {
            _entrySet = new java.util.AbstractSet()
            {
                public java.util.Iterator
                iterator()
                {
                    return new EntryIteratorImpl(null, null, null, false, false);
                }
                
                public boolean
                contains(Object o)
                {
                    if(!(o instanceof Map.Entry))
                    {
                        return false;
                    }
                    Map.Entry entry = (Map.Entry)o;
                    Object value = entry.getValue();
                    
                    byte[] v = getImpl(entry.getDbKey());
                    return v != null && valEquals(decodeValue(v, _connection.communicator()), value);
                }
                
                public boolean
                remove(Object o)
                {
                    if(!(o instanceof Map.Entry))
                    {
                        return false;
                    }
                    Map.Entry entry = (Map.Entry)o;
                    Object value = entry.getValue();

                    byte[] v = getImpl(entry.getDbKey());
                    if(v != null && valEquals(decodeValue(v, _connection.communicator()), value))
                    {
                        return removeImpl(entry.getDbKey());
                    }
                    return false;
                }
                
                public int
                size()
                {
                    return Map.this.size();
                }
                
                public void
                clear()
                {
                    Map.this.clear();
                }
            };
        }

        return _entrySet;
    }

    public void
    closeAllIterators()
    {
        closeAllIteratorsExcept(null, false);
    }
    
    void
    closeAllIteratorsExcept(Object except, boolean finalizing)
    {
        synchronized(_iteratorList)
        {
            java.util.Iterator p = _iteratorList.iterator();

            while(p.hasNext())
            {
                Object obj = p.next();
                if(obj != except)
                {
                    ((EntryIteratorImpl)obj).close(finalizing);
                }
            }
        }
    }

    protected void
    finalize()
    {
        close(true);
    }

    //
    // The synchronization is only needed when finalizing is true
    //
    void 
    close(boolean finalizing)
    {
        synchronized(_connection)
        {
            if(_db != null)
            {
                closeAllIteratorsExcept(null, finalizing);
                try
                {
                    _db.close();
                }
                finally
                {
                    _db = null;
                    _connection.unregisterMap(_token);
                    _token = null;
                }
            }
        }
    }

    EntryIterator 
    createIterator(Index index, Object fromKey, Object toKey)
    {
        KeyCodec codec = index == null ? (KeyCodec)this : (KeyCodec)index;

        Ice.Communicator communicator = _connection.getCommunicator();

        return new EntryIteratorImpl(index, 
                                 fromKey == null ? null : codec.encodeKey(fromKey, communicator),
                                 toKey == null ? null : codec.encodeKey(toKey, communicator),
                                 false, true);
    }
        
    ConnectionI connection()
    {
        return _connection;
    }

    private static boolean
    valEquals(Object o1, Object o2)
    {
        return (o1 == null ? o2 == null : o1.equals(o2));
    }

    private byte[]
    getFirstOrLastKey(com.sleepycat.db.Database db, String dbName, byte[] key, boolean first)
    {
        if(db == null)
        {
            DatabaseException ex = new DatabaseException();
            ex.message = _errorPrefix + "\"" + dbName + "\" has been closed";
            throw ex;
        }
        
        if(_trace >= 2)
        {
            _connection.communicator().getLogger().trace("Freeze.Map", "searching Db \"" + dbName + "\"");
        }
        
        com.sleepycat.db.DatabaseEntry dbKey = key == null ?
            new com.sleepycat.db.DatabaseEntry():
            new com.sleepycat.db.DatabaseEntry(key);

        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        dbValue.setPartial(true); // not interested in value

        try
        {
            for(;;)
            {
                com.sleepycat.db.Cursor dbc = null;
                try
                {
                    dbc = db.openCursor(_connection.dbTxn(), null);
                    com.sleepycat.db.OperationStatus status;

                    if(key == null)
                    {
                        status = first ?
                            dbc.getFirst(dbKey, dbValue, null) : dbc.getLast(dbKey, dbValue, null);
                    }
                    else if(first)
                    {
                        status = dbc.getSearchKeyRange(dbKey, dbValue, null);
                    }
                    else
                    {
                        status = dbc.getSearchKeyRange(dbKey, dbValue, null);

                        if(status == com.sleepycat.db.OperationStatus.SUCCESS)
                        {
                            //
                            // goto the previous pair, which must be < key
                            //
                            status = dbc.getPrevNoDup(dbKey, dbValue, null);
                        }
                        else if(status == com.sleepycat.db.OperationStatus.NOTFOUND)
                        {
                            //
                            // All keys < desired toKey, so we pick the largest of
                            // all, the last one
                            //
                            status = dbc.getLast(dbKey, dbValue, null);
                        }
                    }
                   
                    if(status == com.sleepycat.db.OperationStatus.SUCCESS)
                    {
                        return dbKey.getData();
                    }
                    else
                    {
                        return null;
                    }
                }
                catch(com.sleepycat.db.DeadlockException dx)
                {
                    if(_connection.dbTxn() != null)
                    {
                        DeadlockException ex = new DeadlockException();
                        ex.initCause(dx);
                        ex.message = _errorPrefix + "Dbc.getXXX: " + dx.getMessage();
                        throw ex;
                    }
                    else
                    {
                        if(_connection.deadlockWarning())
                        {
                            _connection.communicator().getLogger().warning(
                                "Deadlock in Freeze.Map while searching \"" + db.getDatabaseName() +
                                "\"; retrying...");
                        }
                        
                        //
                        // Retry
                        //
                    }
                }
                finally
                {
                    if(dbc != null)
                    {
                        try
                        {
                            dbc.close();
                        }
                        catch(com.sleepycat.db.DeadlockException dx)
                        {
                            //
                            // Ignored
                            //
                        }
                    }
                }
            }
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            DatabaseException ex = new DatabaseException();
            ex.initCause(dx);
            ex.message = _errorPrefix + "Db.openCursor/Dbc.getXXX: " + dx.getMessage();
            throw ex; 
        }
    }

    private byte[]
    getImpl(com.sleepycat.db.DatabaseEntry dbKey)
    {
        if(_db == null)
        {
            DatabaseException ex = new DatabaseException();
            ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
            throw ex;
        }

        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();

        if(_trace >= 2)
        {
            _connection.communicator().getLogger().trace("Freeze.Map", "reading value from Db \"" + _db.dbName() +
                                                         "\"");
        }

        for(;;)
        {
            try
            {
                com.sleepycat.db.OperationStatus rc = _db.db().get(_connection.dbTxn(), dbKey, dbValue, null);
                if(rc == com.sleepycat.db.OperationStatus.SUCCESS)
                {
                    return dbValue.getData();
                }
                else
                {
                    return null;
                }
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                if(_connection.dbTxn() != null)
                {
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
                    throw ex;
                }
                else
                {
                    if(_connection.deadlockWarning())
                    {
                        _connection.communicator().getLogger().warning("Deadlock in Freeze.Map.getImpl while " +
                                                                       "reading Db \"" + _db.dbName() +
                                                                       "\"; retrying...");
                    }

                    //
                    // Try again
                    //
                }
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(e);
                ex.message = _errorPrefix + "Db.get: " + e.getMessage();
                throw ex;
            }
        }
    }

    private void
    putImpl(com.sleepycat.db.DatabaseEntry dbKey, Object value)
    {
        if(_db == null)
        {
            DatabaseException ex = new DatabaseException();
            ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
            throw ex;
        }

        byte[] v = encodeValue(value, _connection.communicator());      
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry(v);
        
        if(_trace >= 2)
        {
            _connection.communicator().getLogger().trace("Freeze.Map", "writing value in Db \"" + _db.dbName() + "\"");
        }

        com.sleepycat.db.Transaction txn = _connection.dbTxn();
        if(txn == null)
        {
            closeAllIterators();
        }

        for(;;)
        {
            try
            {
                _db.db().put(txn, dbKey, dbValue);
                break;
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                if(txn != null)
                {
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Db.put: " + e.getMessage();
                    throw ex;
                }
                else
                {
                    if(_connection.deadlockWarning())
                    {
                        _connection.communicator().getLogger().warning("Deadlock in Freeze.Map.putImpl while " +
                                                                       "writing into Db \"" + _db.dbName() +
                                                                       "\"; retrying...");
                    }

                    //
                    // Try again
                    //
                }
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(e);
                ex.message = _errorPrefix + "Db.put: " + e.getMessage();
                throw ex;
            }
        }
    }

    private boolean
    removeImpl(com.sleepycat.db.DatabaseEntry dbKey)
    {
        if(_db == null)
        {
            DatabaseException ex = new DatabaseException();
            ex.message = _errorPrefix + "\"" + _db.dbName() + "\" has been closed";
            throw ex;
        }

        if(_trace >= 2)
        {
            _connection.communicator().getLogger().trace("Freeze.Map", "deleting value from Db \"" + _db.dbName() +
                                                         "\"");
        }

        com.sleepycat.db.Transaction txn = _connection.dbTxn();
        if(txn == null)
        {
            closeAllIterators();
        }

        for(;;)
        {
            try
            {
                com.sleepycat.db.OperationStatus rc = _db.db().delete(txn, dbKey);
                return (rc == com.sleepycat.db.OperationStatus.SUCCESS);
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                if(txn != null)
                {
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Db.del: " + e.getMessage();
                    throw ex;
                }
                else
                {
                    if(_connection.deadlockWarning())
                    {
                        _connection.communicator().getLogger().warning("Deadlock in Freeze.Map.removeImpl while " +
                                                                       "writing into Db \"" + _db.dbName() +
                                                                       "\"; retrying...");
                    }

                    //
                    // Try again
                    //
                }
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(e);
                ex.message = _errorPrefix + "Db.del: " + e.getMessage();
                throw ex;
            }
        }
    }

    private class Comparator implements java.util.Comparator
    {
        Comparator(java.util.Comparator comparator)
        {
            _comparator = comparator;
        }
        
        public java.util.Comparator comparator()
        {
            return _comparator;
        }
        
        public int compare(Object o1, Object o2)
        {
            byte[] d1 = (byte[])o1;
            byte[] d2 = (byte[])o2;

            Ice.Communicator communicator = _connection.communicator();

            return _comparator.compare(decodeKey(d1, communicator),
                                       decodeKey(d2, communicator));
        }

        //
        // The user-supplied comparator
        //
        private final java.util.Comparator _comparator;
    }

    public abstract class Index 
        implements com.sleepycat.db.SecondaryKeyCreator, java.util.Comparator, KeyCodec
    {
        //
        // Implementation details
        //
        public boolean
        createSecondaryKey(com.sleepycat.db.SecondaryDatabase secondary,
                           com.sleepycat.db.DatabaseEntry key,
                           com.sleepycat.db.DatabaseEntry value,
                           com.sleepycat.db.DatabaseEntry result)
            throws com.sleepycat.db.DatabaseException
        {
            Ice.Communicator communicator = _connection.getCommunicator();   
            byte[] secondaryKey = marshalKey(value.getData());
            assert(secondaryKey != null);
           
            result.setData(secondaryKey);
            result.setSize(secondaryKey.length);
            return true;
        }
        
        com.sleepycat.db.SecondaryDatabase
        db()
        {
            return _db;
        }
        
        String name()
        {
            return _name;
        }

        protected Index(String name)
        {
            _name = name;
        }

        void
        associate(String dbName, com.sleepycat.db.Database db, 
                  com.sleepycat.db.Transaction txn, boolean createDb,
                  java.util.Comparator comparator)
            throws com.sleepycat.db.DatabaseException, java.io.FileNotFoundException
        {
            _dbName = dbName + "." + _name;
            _comparator = comparator;

            assert(txn != null);
            assert(_db == null);

            com.sleepycat.db.SecondaryConfig config = new com.sleepycat.db.SecondaryConfig();
            config.setAllowCreate(createDb);
            config.setAllowPopulate(true); // We always populate empty indices
            config.setSortedDuplicates(true);
            config.setType(com.sleepycat.db.DatabaseType.BTREE);
            if(_comparator != null)
            {
                config.setBtreeComparator(this);
            }
            config.setKeyCreator(this);

            _db = _connection.dbEnv().getEnv().openSecondaryDatabase(txn, _dbName, null, db, config);
        }
        
        void init(Index from)
        {
            assert(_name.equals(from._name));
            assert(_db == null);

            _dbName = from._dbName;
            _db = from._db;
            _comparator = from._comparator;
        }

        java.util.Comparator comparator()
        {
            return _comparator;
        }
        
        Map parent()
        {
            return Map.this;
        }

        Object firstKey(Object fromKey, Object toKey)
        {
            byte[] fk = fromKey == null ? null :
                encodeKey(fromKey, _connection.communicator());
            
            byte[] k = getFirstOrLastKey(_db, _dbName, fk, true);
            if(k == null)
            {
                throw new NoSuchElementException();
            }
            else
            {
                Object key = decodeKey(k, _connection.communicator());
                if(toKey != null && _comparator.compare(key, toKey) >= 0)
                {
                    throw new NoSuchElementException();
                }
                return key;
            }
        }

        Object lastKey(Object fromKey, Object toKey)
        {
            byte[] tk = toKey == null ? null :
                encodeKey(toKey, _connection.communicator());
            
            byte[] k = getFirstOrLastKey(_db, _dbName, tk, false);
            if(k == null)
            {
                throw new NoSuchElementException();
            }
            else
            {
                Object key = decodeKey(k, _connection.communicator());
                if(fromKey != null && _comparator.compare(fromKey, key) > 0)
                {
                    throw new NoSuchElementException();
                }
                return key;
            }
        }

        void close()
        {
            //
            // close() is called by SharedDb only on the "main" index
            // (the one that was associated)
            //

            if(_db != null)
            {
                try
                {
                    _db.close();
                }
                catch(com.sleepycat.db.DatabaseException dx)
                {
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(dx);
                    ex.message = _errorPrefix + "Db.close for index \"" + _dbName + "\": " + dx.getMessage();
                    throw ex;
                }
                _db = null;
            }
        }
    
        public EntryIterator 
        untypedFind(Object key, boolean onlyDups)
        {
            byte[] k = encodeKey(key, _connection.communicator());
            return new EntryIteratorImpl(this, k, null, onlyDups, false);
        }

        public int
        untypedCount(Object key)
        {
            byte[] k = encodeKey(key, _connection.communicator());

            com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
            com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
            
            //
            // When we have a custom-comparison function, Berkeley DB returns
            // the key on-disk (when it finds one). We disable this behavior:
            // (ref Oracle SR 5925672.992)
            //
            dbKey.setPartial(true);

            //
            // dlen is 0, so we should not retrieve any value 
            // 
            dbValue.setPartial(true);
            
            try
            {
                for(;;)
                {
                    com.sleepycat.db.Cursor dbc = null;
                    try
                    {
                        dbc = _db.openCursor(null, null);   
                        if(dbc.getSearchKey(dbKey, dbValue, null) == com.sleepycat.db.OperationStatus.SUCCESS)
                        {
                            return dbc.count();
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    catch(com.sleepycat.db.DeadlockException dx)
                    {
                        if(_connection.deadlockWarning())
                        {
                            _connection.communicator().getLogger().warning(
                                "Deadlock in Freeze.Map.Index.untypedCount while iterating over index \"" + _dbName +
                                "\"; retrying...");
                        }
                        
                        //
                        // Retry
                        //
                    }
                    finally
                    {
                        if(dbc != null)
                        {
                            try
                            {
                                dbc.close();
                            }
                            catch(com.sleepycat.db.DeadlockException dx)
                            {
                                //
                                // Ignored
                                //
                            }
                        }
                    }
                }
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                DatabaseException ex = new DatabaseException();
                ex.initCause(dx);
                ex.message = _errorPrefix + "Db.cursor for index \"" + _dbName + "\": " + dx.getMessage();
                throw ex; 
            }
        } 
        
        boolean containsKey(Object key)
        {
            byte[] k = encodeKey(key, _connection.communicator());

            com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
            com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
            dbValue.setPartial(true);

            if(_trace >= 2)
            {
                _connection.communicator().getLogger().trace("Freeze.Map.Index", "checking key in Db \"" + _dbName +
                                                             "\"");
            }
            
            for(;;)
            {
                try
                {
                    return _db.get(_connection.dbTxn(), dbKey, dbValue, null)
                        == com.sleepycat.db.OperationStatus.SUCCESS;
                }
                catch(com.sleepycat.db.DeadlockException e)
                {
                    if(_connection.dbTxn() != null)
                    {
                        DeadlockException ex = new DeadlockException();
                        ex.initCause(e);
                        ex.message = _errorPrefix + "Db.get: " + e.getMessage();
                        throw ex;
                    }
                    else
                    {
                        if(_connection.deadlockWarning())
                        {
                            _connection.communicator().getLogger().warning(
                                "Deadlock in Freeze.Map.Index.containsKey while " +
                                "reading Db \"" + _dbName + "\"; retrying...");
                        }
                        //
                        // Try again
                        //
                    }
                }
                catch(com.sleepycat.db.DatabaseException e)
                {
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Db.get: " + e.getMessage();
                    throw ex;
                }
            }
        }
        
        //
        // Extracts the index key from this value
        //
        public abstract Object extractKey(Object value);

        protected byte[] marshalKey(byte[] value)
        {
            Object decodedValue = decodeValue(value, _connection.communicator());
            return encodeKey(extractKey(decodedValue), _connection.communicator());
        }

        //
        // The user-supplied comparator
        //
        protected java.util.Comparator _comparator;

        private String _name;
        private String _dbName;
        private com.sleepycat.db.SecondaryDatabase _db;
    }

    /**
     *
     * The entry iterator allows clients to explicitly close the iterator 
     * and free resources allocated for the iterator as soon as possible.
     *
     **/
    public interface EntryIterator extends java.util.Iterator
    {
        void close();
        void destroy(); // an alias for close
    }

    class EntryIteratorImpl implements EntryIterator
    {
        EntryIteratorImpl(Index index, byte[] fromKey, byte[] toKey, 
                          boolean onlyFromKeyDups, boolean skipDups)
        {
            _index = index;
            _fromKey = fromKey;
            _toKey = toKey;
            _onlyFromKeyDups = onlyFromKeyDups;
            _skipDups = skipDups;

            try
            {
                com.sleepycat.db.Transaction txn = _connection.dbTxn();
                if(txn == null)
                {
                    //
                    // Start transaction
                    //
                    txn = _connection.dbEnv().getEnv().beginTransaction(null, null);
                    _txn = txn;

                    if(_connection.txTrace() >= 1)
                    {
                        String txnId = Long.toHexString((_txn.getId() & 0x7FFFFFFF) + 0x80000000L); 

                        _connection.communicator().getLogger().trace("Freeze.Map", _errorPrefix +
                                                                     "started transaction " + txnId + " for cursor");
                    }
                }
                
                //
                // Open cursor with this transaction
                //
                if(index == null)
                {
                    _cursor = _db.db().openCursor(txn, null);
                }
                else
                {
                    _cursor = index.db().openSecondaryCursor(txn, null);
                }
            }
            catch(com.sleepycat.db.DeadlockException dx)
            {
                dead();
                DeadlockException ex = new DeadlockException();
                ex.initCause(dx);
                ex.message = _errorPrefix + "EntryIterator constructor: " + dx.getMessage();
                throw ex;
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                dead();
                DatabaseException ex = new DatabaseException();
                ex.initCause(dx);
                ex.message = _errorPrefix + "EntryIterator constructor: " + dx.getMessage();
                throw ex;
            }

            synchronized(_iteratorList)
            {
                _iteratorList.addFirst(this);
                java.util.Iterator p = _iteratorList.iterator();
                p.next();
                _iteratorListToken = p;
            }
        }

        public boolean
        hasNext()
        {
            if(_current == null || _current == _lastReturned)
            {
                com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry();
                com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
                com.sleepycat.db.DatabaseEntry dbIKey = new com.sleepycat.db.DatabaseEntry();
                com.sleepycat.db.OperationStatus status = null;

                try
                {
                    if(_index != null)
                    {
                        com.sleepycat.db.SecondaryCursor c = (com.sleepycat.db.SecondaryCursor)_cursor;
                        if(_current == null)
                        {
                            //
                            // First key
                            //
                            if(_fromKey != null)
                            {
                                dbIKey.setData(_fromKey);
                                status = c.getSearchKeyRange(dbIKey, dbKey, dbValue, null);
                            }
                            else
                            {
                                status = c.getFirst(dbIKey, dbKey, dbValue, null);
                            }
                        }
                        else
                        {   
                            if(_onlyFromKeyDups)
                            {
                                status = c.getNextDup(dbIKey, dbKey, dbValue, null);
                            }
                            else if(_skipDups)
                            {
                                status = c.getNextNoDup(dbIKey, dbKey, dbValue, null);
                            }
                            else
                            {
                                status = c.getNext(dbIKey, dbKey, dbValue, null);
                            }
                        }
                    }
                    else
                    {
                        if(_current == null && _fromKey != null)
                        {
                            dbKey.setData(_fromKey);
                            status = _cursor.getSearchKey(dbKey, dbValue, null);
                        }
                        else
                        {
                            status = _cursor.getNext(dbKey, dbValue, null);
                        }
                    }
                }
                catch(com.sleepycat.db.DeadlockException dx)
                {
                    dead();
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(dx);
                    ex.message = _errorPrefix + "Dbc.get: " + dx.getMessage();
                    throw ex;
                }
                catch(com.sleepycat.db.DatabaseException dx)
                {
                    dead();
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(dx);
                    ex.message = _errorPrefix + "Dbc.get: " + dx.getMessage();
                    throw ex;
                }
                
                if(status == com.sleepycat.db.OperationStatus.SUCCESS)
                {
                    //
                    // Verify it's < _toKey
                    //
                    boolean inRange = true;
                    if(_toKey != null)
                    {
                        if(_index != null)
                        {
                            inRange = _index.compare(dbIKey.getData(), _toKey) < 0;
                        }
                        else
                        {
                            inRange = _comparator.compare(dbKey.getData(), _toKey) < 0;
                        }
                    }
                    
                    if(inRange)
                    {
                        _current = new Entry(this, Map.this, _connection.communicator(), dbKey, 
                                             dbValue.getData(), dbIKey.getData());
                        return true;
                    }
                }
                return false;
            }
            else
            {
                return true;
            }
        }
        
        public Object
        next()
        {
            if(hasNext())
            {
                _lastReturned = _current;
                return _lastReturned;
            }
            else
            {
                throw new java.util.NoSuchElementException();
            }
        }

        public void
        remove()
        {
            if(_txn != null)
            {
                closeAllIteratorsExcept(this, false);
            }

            //
            // Removes the last object returned by next()
            //
            if(_lastReturned == null)
            {
                throw new IllegalStateException();
            }
            
            if(_lastReturned == _current)
            {
                try
                {
                    if(_cursor.delete() == com.sleepycat.db.OperationStatus.KEYEMPTY)
                    {
                        throw new IllegalStateException();
                    }
                }
                catch(com.sleepycat.db.DeadlockException e)
                {
                    dead();
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Dbc.del: " + e.getMessage();
                    throw ex;
                }
                catch(com.sleepycat.db.DatabaseException e)
                { 
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Dbc.del: " + e.getMessage();
                    throw ex;
                }
            }
            else
            {
                //
                // Duplicate the cursor and move the _lastReturned 
                // element to delete it (using the duplicate)
                //
                
                //
                // This works only for non-index iterators
                //
                if(_index != null)
                {
                    throw new UnsupportedOperationException();
                }

                com.sleepycat.db.Cursor clone = null;

                try
                {
                    clone = _cursor.dup(true);

                    //
                    // Not interested in data
                    //
                    com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
                    dbValue.setPartial(true);
                    
                    com.sleepycat.db.OperationStatus rc = clone.getSearchKey(_lastReturned.getDbKey(), dbValue, null);

                    if(rc == com.sleepycat.db.OperationStatus.NOTFOUND)
                    {
                        throw new IllegalStateException();
                    }
                    if(clone.delete() == com.sleepycat.db.OperationStatus.KEYEMPTY)
                    {
                        throw new IllegalStateException();
                    }
                }
                catch(com.sleepycat.db.DeadlockException e)
                {
                    dead();
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "EntryIterator.remove: " + e.getMessage();
                    throw ex;
                }
                catch(com.sleepycat.db.DatabaseException e)
                { 
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "EntryIterator.remove: " + e.getMessage();
                    throw ex;
                }
                finally
                {
                    if(clone != null)
                    {
                        closeCursor(clone);
                    }
                }
            }
        }

        //
        // Extra operations.
        //
        public void
        close()
        {
            close(false);
        }

        //
        // The synchronized is needed because this method can be called 
        // concurrently by Connection, Map and Map.EntryIterator finalizers.
        //
        synchronized void
        close(boolean finalizing)
        {
            if(finalizing && (_cursor != null || _txn != null) && _connection.closeInFinalizeWarning())
            {
                _connection.communicator().getLogger().warning(
                    "finalize() closing a live iterator on Map \"" + _db.dbName() + "\"; the application " +
                     "should have closed it earlier by calling Map.EntryIterator.close(), " +
                     "Map.closeAllIterators(), Map.close(), Connection.close(), or (if also " +
                     "leaking a transaction) Transaction.commit() or Transaction.rollback()");
            }
         
            if(_iteratorListToken != null)
            {
                synchronized(_iteratorList)
                {
                    _iteratorListToken.remove();
                    _iteratorListToken = null;
                }
            }
   
            if(_cursor != null) 
            {
                com.sleepycat.db.Cursor cursor = _cursor;
                _cursor = null;
                closeCursor(cursor);
            }
            
            if(_txn != null)
            {
                String txnId = null;

                try
                {
                    if(_connection.txTrace() >= 1)
                    {
                        txnId = Long.toHexString((_txn.getId() & 0x7FFFFFFF) + 0x80000000L); 
                    }

                    _txn.commit();

                    if(_connection.txTrace() >= 1)
                    {
                        _connection.communicator().getLogger().trace("Freeze.Map", _errorPrefix +
                                                                     "committed transaction " + txnId);
                    }
                }
                catch(com.sleepycat.db.DeadlockException e)
                {
                    if(_connection.txTrace() >= 1)
                    {
                        _connection.communicator().getLogger().trace("Freeze.Map", _errorPrefix +
                                                                     "failed to commit transaction " + txnId + ": " +
                                                                     e.getMessage());
                    }

                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "DbTxn.commit: " + e.getMessage();
                    throw ex;
                }
                catch(com.sleepycat.db.DatabaseException e)
                {
                    if(_connection.txTrace() >= 1)
                    {
                        _connection.communicator().getLogger().trace("Freeze.Map", _errorPrefix +
                                                                     "failed to commit transaction " + txnId + ": " +
                                                                     e.getMessage());
                    }

                    DatabaseException ex = new DatabaseException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "DbTxn.commit: " + e.getMessage();
                    throw ex;
                }
                finally
                {
                    _txn = null;
                }
            }   
        }
        
        //
        // An alias for close()
        //
        public void
        destroy()
        {
            close();
        }

        protected void
        finalize()
        {
            close(true);
        }

        void
        setValue(Map.Entry entry, Object value)
        {
            if(_index != null)
            {
                throw new UnsupportedOperationException(
                    _errorPrefix + "Cannot set an iterator retrieved through an index");
            }

            if(_txn != null)
            {
                closeAllIteratorsExcept(this, false);
            }

            //
            // Are we trying to update the current value?
            //
            if(_current == entry)
            {
                //
                // Yes, update it directly
                //
                byte[] v = encodeValue(value, _connection.communicator());
                com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry(v);
                
                try
                {
                    _cursor.putCurrent(dbValue);
                }
                catch(com.sleepycat.db.DeadlockException e)
                {
                    dead();
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Dbc.put: " + e.getMessage();
                    throw ex;
                }
                catch(com.sleepycat.db.DatabaseException e)
                { 
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "Dbc.put: " + e.getMessage();
                    throw ex;
                }
            }
            else
            {
                //
                // Duplicate the cursor and move the entry
                // element to update it (using the duplicate cursor)
                //
                
                com.sleepycat.db.Cursor clone = null;

                try
                {
                    clone = _cursor.dup(true);

                    //
                    // Not interested in data
                    //
                    com.sleepycat.db.DatabaseEntry dummy = new com.sleepycat.db.DatabaseEntry();
                    dummy.setPartial(true);
                    
                    com.sleepycat.db.OperationStatus rc = clone.getSearchKey(entry.getDbKey(), dummy, null);

                    if(rc == com.sleepycat.db.OperationStatus.NOTFOUND)
                    {
                        NotFoundException ex = new NotFoundException();
                        ex.message = _errorPrefix + "Dbc.get: DB_NOTFOUND";
                        throw ex;
                    }
                   
                    byte[] v = encodeValue(value, _connection.communicator());
                    com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry(v);
                    clone.putCurrent(dbValue);
                }
                catch(com.sleepycat.db.DeadlockException e)
                {
                    dead();
                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "EntryIterator.setValue: " + e.getMessage();
                    throw ex;
                }
                catch(com.sleepycat.db.DatabaseException e)
                { 
                    DatabaseException ex = new DatabaseException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "EntryIterator.setValue: " + e.getMessage();
                    throw ex;
                }
                finally
                {
                    if(clone != null)
                    {
                        closeCursor(clone);
                    }
                }
            }
        }

        private void
        closeCursor(com.sleepycat.db.Cursor cursor)
        {
            try
            {
                cursor.close();
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                dead();
                DeadlockException ex = new DeadlockException();
                ex.initCause(e);
                ex.message = _errorPrefix + "Dbc.close: " + e.getMessage();
                throw ex;
            }
            catch(com.sleepycat.db.DatabaseException e)
            { 
                DatabaseException ex = new DatabaseException();
                ex.initCause(e);
                ex.message = _errorPrefix + "Dbc.close: " + e.getMessage();
                throw ex;
            }
        }

        private void
        dead()
        {
            if(_cursor != null) 
            {
                com.sleepycat.db.Cursor cursor = _cursor;
                _cursor = null;
                closeCursor(cursor);
            }

            if(_txn != null)
            {
                String txnId = null;

                try
                {
                    if(_connection.txTrace() >= 1)
                    {
                        txnId = Long.toHexString((_txn.getId() & 0x7FFFFFFF) + 0x80000000L); 
                    }

                    _txn.abort();

                    if(_connection.txTrace() >= 1)
                    {
                        _connection.communicator().getLogger().trace("Freeze.Map", _errorPrefix +
                                                                     "rolled back transaction " + txnId);
                    }
                }
                catch(com.sleepycat.db.DeadlockException e)
                {
                    if(_connection.txTrace() >= 1)
                    {
                        _connection.communicator().getLogger().trace("Freeze.Map", _errorPrefix +
                                                                     "failed to roll back transaction " + txnId +
                                                                     ": " + e.getMessage());
                    }

                    DeadlockException ex = new DeadlockException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "DbTxn.abort: " + e.getMessage();
                    throw ex;
                }
                catch(com.sleepycat.db.DatabaseException e)
                {
                    if(_connection.txTrace() >= 1)
                    {
                        _connection.communicator().getLogger().trace("Freeze.Map", _errorPrefix +
                                                                     "failed to roll back transaction " +
                                                                     txnId + ": " + e.getMessage());
                    }

                    DatabaseException ex = new DatabaseException();
                    ex.initCause(e);
                    ex.message = _errorPrefix + "DbTxn.abort: " + e.getMessage();
                    throw ex;
                }
                finally
                {
                    _txn = null;
                }
            }
        }

        private com.sleepycat.db.Transaction _txn;
        private com.sleepycat.db.Cursor _cursor;
        private Entry _current;
        private Entry _lastReturned;
        private java.util.Iterator _iteratorListToken;
        
        private final Index _index;
        private final byte[] _fromKey;
        private final byte[] _toKey;
        private final boolean _onlyFromKeyDups;
        private final boolean _skipDups;
    }

    static class Entry implements java.util.Map.Entry 
    {
        public
        Entry(EntryIteratorImpl iterator, Map map, Ice.Communicator communicator, 
              com.sleepycat.db.DatabaseEntry dbKey, byte[] valueBytes, byte[] indexBytes)
        {
            _iterator = iterator;
            _map = map;
            _communicator = communicator;
            _dbKey = dbKey;
            _valueBytes = valueBytes;
            _indexBytes = indexBytes;
        }

        public Object
        getKey()
        {
            if(!_haveKey)
            {
                assert(_dbKey != null);
                _key = _map.decodeKey(_dbKey.getData(), _communicator);
                _haveKey = true;
            }
            return _key;
        }

        public Object
        getValue()
        {
            if(!_haveValue)
            {
                assert(_valueBytes != null);
                _value = _map.decodeValue(_valueBytes, _communicator);
                _haveValue = true;
                //
                // Not needed anymore
                //
                _valueBytes = null;
            }
            return _value;
        }

        public byte[]
        getIndexBytes()
        {
            return _indexBytes;
        }

        public Object
        setValue(Object value)
        {
            Object old = getValue();
            _iterator.setValue(this, value);
            _value = value;
            _haveValue = true;
            return old;
        }

        public boolean
        equals(Object o)
        {
            if(!(o instanceof Map.Entry))
            {
                return false;
            }
            Map.Entry e = (Map.Entry)o;
            return eq(getKey(), e.getKey()) && eq(getValue(), e.getValue());
        }

        public int
        hashCode()
        {
            return ((getKey()   == null) ? 0 : getKey().hashCode()) ^
                   ((getValue() == null) ? 0 : getValue().hashCode());
        }

        public String
        toString()
        {
            return getKey() + "=" + getValue();
        }

        com.sleepycat.db.DatabaseEntry
        getDbKey()
        {
            return _dbKey;
        }

        private /*static*/ boolean
        eq(Object o1, Object o2)
        {
            return (o1 == null ? o2 == null : o1.equals(o2));
        }

        private EntryIteratorImpl _iterator;
        private Map _map;
        private Ice.Communicator _communicator;
        private com.sleepycat.db.DatabaseEntry _dbKey;
        private byte[] _valueBytes;
        private byte[] _indexBytes;
        private Object _key;
        private boolean _haveKey = false;
        private Object _value;
        private boolean _haveValue = false;
    }

    public static class Patcher implements IceInternal.Patcher
    {
        public
        Patcher(String type)
        {
            this.type = type;
        }

        public void
        patch(Ice.Object v)
        {
            value = v;
        }

        public String
        type()
        {
            return this.type;
        }

        public Ice.Object
        value()
        {
            return this.value;
        }

        public String type;
        public Ice.Object value;
    }
    
    protected ConnectionI _connection;
    private final Comparator _comparator;

    protected java.util.Iterator _token;
    protected SharedDb _db;
    protected String _errorPrefix;
    protected int _trace;

    private java.util.Set _entrySet;
    private LinkedList _iteratorList = new LinkedList();
    private java.util.Map _indexMap = new java.util.HashMap();
}
