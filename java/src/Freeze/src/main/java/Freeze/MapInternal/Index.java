// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

import Freeze.DatabaseException;
import Freeze.DeadlockException;
import Freeze.MapIndex;
import Freeze.NavigableMap;
import java.nio.ByteBuffer;

public abstract class Index<K, V, I>
    implements MapIndex, KeyCodec<I>, com.sleepycat.db.SecondaryKeyCreator, java.util.Comparator<ByteBuffer>
{
    protected
    Index(MapI<K, V> map, String name, java.util.Comparator<I> comparator)
    {
        _map = map;
        _name = name;
        _comparator = comparator;
    }

    //
    // Subclasses define this so that we can extract the index key from a value.
    //
    protected abstract I extractKey(V value);

    //
    // MapIndex methods
    //

    @Override
    public String
    name()
    {
        return _name;
    }

    @Override
    public void
    associate(String dbName, com.sleepycat.db.Database db, com.sleepycat.db.Transaction txn, boolean createDb)
        throws com.sleepycat.db.DatabaseException, java.io.FileNotFoundException
    {
        _dbName = dbName + "." + _name;
        _trace = new TraceLevels(_map.connection(), _dbName);

        assert(txn != null);
        assert(_db == null);

        com.sleepycat.db.SecondaryConfig config = new com.sleepycat.db.SecondaryConfig();
        config.setAllowCreate(createDb);
        config.setAllowPopulate(true); // We always populate empty indices
        config.setSortedDuplicates(true);
        config.setType(com.sleepycat.db.DatabaseType.BTREE);
        if(_comparator != null)
        {
            java.util.Comparator<byte[]> c = new java.util.Comparator<byte[]>()
            {
                @Override
                public int compare(byte[] a1, byte[] a2)
                {
                    return Index.this.compare(ByteBuffer.wrap(a1), ByteBuffer.wrap(a2));
                }
            };
            config.setBtreeComparator(c);
        }
        config.setKeyCreator(this);

        Ice.Properties properties = _map.connection().getCommunicator().getProperties();
        String propPrefix = "Freeze.Map." + _dbName + ".";

        int btreeMinKey = properties.getPropertyAsInt(propPrefix + "BtreeMinKey");
        if(btreeMinKey > 2)
        {
            if(_trace.level >= 1)
            {
                _trace.logger.trace("Freeze.Map", "Setting \"" + _dbName + "\"'s btree minkey to " + btreeMinKey);
            }
            config.setBtreeMinKey(btreeMinKey);
        }

        boolean checksum = properties.getPropertyAsInt(propPrefix + "Checksum") > 0;
        if(checksum)
        {
            if(_trace.level >= 1)
            {
               _trace.logger.trace("Freeze.Map", "Turning checksum on for \"" + _dbName + "\"");
            }
            config.setChecksum(true);
        }

        int pageSize = properties.getPropertyAsInt(propPrefix + "PageSize");
        if(pageSize > 0)
        {
            if(_trace.level >= 1)
            {
                _trace.logger.trace("Freeze.Map", "Setting \"" + _dbName + "\"'s pagesize to " + pageSize);
            }
            config.setPageSize(pageSize);
        }

        _db = _map.connection().dbEnv().getEnv().openSecondaryDatabase(txn, _dbName, null, db, config);
    }

    @Override
    public void
    init(MapIndex f)
    {
        @SuppressWarnings("unchecked")
        Index<K, V, I> from = (Index<K, V, I>)f;

        assert(_name.equals(from._name));
        assert(_db == null);

        _dbName = from._dbName;
        _db = from._db;
        _comparator = from._comparator;
        _trace = _map.traceLevels();
    }

    @Override
    public void
    close()
    {
        //
        // close() is called by MapDb only on the "main" index
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
                throw new DatabaseException(
                    _trace.errorPrefix + "Db.close for index \"" + _dbName + "\": " + dx.getMessage(), dx);
            }
            _db = null;
        }
    }

    //
    // SecondaryKeyCreator methods
    //

    @Override
    public boolean
    createSecondaryKey(com.sleepycat.db.SecondaryDatabase secondary,
                       com.sleepycat.db.DatabaseEntry key,
                       com.sleepycat.db.DatabaseEntry value,
                       com.sleepycat.db.DatabaseEntry result)
        throws com.sleepycat.db.DatabaseException
    {
        ByteBuffer secondaryKey = marshalKey(UtilI.getBuffer(value));
        assert(secondaryKey != null);

        result.setDataNIO(secondaryKey);
        return true;
    }

    //
    // java.util.Comparator<ByteBuffer>.compare()
    //
    @Override
    public int compare(ByteBuffer b1, ByteBuffer b2)
    {
        assert(_comparator != null);
        return _comparator.compare(decodeKey(b1), decodeKey(b2));
    }

    private class FindModel implements IteratorModel<K, V>
    {
        FindModel(I key, boolean onlyDups)
        {
            _fromKey = key;
            _onlyDups = onlyDups;
        }

        @Override
        public String
        dbName()
        {
            return Index.this.dbName();
        }

        @Override
        public TraceLevels
        traceLevels()
        {
            return _trace;
        }

        @Override
        public com.sleepycat.db.Cursor
        openCursor()
            throws com.sleepycat.db.DatabaseException
        {
            return _db.openSecondaryCursor(_map.connection().dbTxn(), null);
        }

        @Override
        public EntryI<K, V>
        firstEntry(com.sleepycat.db.Cursor cursor)
            throws com.sleepycat.db.DatabaseException
        {
            return Index.this.findFirstEntry(cursor, _fromKey);
        }

        @Override
        public EntryI<K, V>
        nextEntry(com.sleepycat.db.Cursor cursor)
            throws com.sleepycat.db.DatabaseException
        {
            return Index.this.findNextEntry(cursor, _onlyDups);
        }

        private final I _fromKey;
        private final boolean _onlyDups;
    }

    public IteratorI<K, V>
    find(I key, boolean onlyDups)
    {
        return new IteratorI<K, V>(_map, new FindModel(key, onlyDups));
    }

    public IteratorI<K, V>
    find(I key)
    {
        return find(key, true);
    }

    public int
    count(I key)
    {
        ByteBuffer k = encodeKey(key);

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();

        //
        // When we have a custom-comparison function, Berkeley DB returns
        // the key on-disk (when it finds one). We disable this behavior:
        // (ref Oracle SR 5925672.992)
        //
        // In DB > 5.1.x we can not set DB_DBT_PARTIAL in the key Dbt when calling
        // getSearchKey.
        //
        if(com.sleepycat.db.Environment.getVersionMajor() < 5 ||
           (com.sleepycat.db.Environment.getVersionMajor() == 5 &&
            com.sleepycat.db.Environment.getVersionMinor() <= 1))
        {
            dbKey.setPartial(true);
        }

        //
        // dlen is 0, so we should not retrieve any value
        //
        dbValue.setPartial(true);

        try
        {
            for(;;)
            {
                try
                {
                    com.sleepycat.db.Cursor dbc = null;
                    try
                    {
                        dbc = _db.openCursor(_map.connection().dbTxn(), null);
                        if(dbc.getSearchKey(dbKey, dbValue, null) == com.sleepycat.db.OperationStatus.SUCCESS)
                        {
                            return dbc.count();
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    finally
                    {
                        if(dbc != null)
                        {
                            dbc.close();
                        }
                    }
                }
                catch(com.sleepycat.db.DeadlockException dx)
                {
                    if(_map.connection().dbTxn() != null)
                    {
                        throw new DeadlockException(_trace.errorPrefix + "Dbc.count: " + dx.getMessage(),
                                                    _map.connection().currentTransaction(), dx);
                    }
                    else
                    {

                        if(_trace.deadlockWarning)
                        {
                            _trace.logger.warning(
                                "Deadlock in Freeze.MapInternal.Index.count while iterating over index \"" + _dbName +
                                "\"; retrying...");
                        }

                        //
                        // Retry
                        //
                    }
                }
            }
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            throw new DatabaseException(
                _trace.errorPrefix + "Db.cursor for index \"" + _dbName + "\": " + dx.getMessage(), dx);
        }
    }

    //
    // Used by subclasses to implement headMapForXXX.
    //
    public NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    createHeadMap(I toKey, boolean inclusive)
    {
        if(toKey == null)
        {
            throw new NullPointerException();
        }

        if(_comparator == null)
        {
            throw new UnsupportedOperationException("Index '" + _name + "' has no user-defined comparator");
        }

        return new IndexedSubMap<K, V, I>(this, null, false, toKey, inclusive);
    }

    //
    // Used by subclasses to implement tailMapForXXX.
    //
    public NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    createTailMap(I fromKey, boolean inclusive)
    {
        if(fromKey == null)
        {
            throw new NullPointerException();
        }

        if(_comparator == null)
        {
            throw new UnsupportedOperationException("Index '" + _name + "' has no user-defined comparator");
        }

        return new IndexedSubMap<K, V, I>(this, fromKey, inclusive, null, false);
    }

    //
    // Used by subclasses to implement subMapForXXX.
    //
    public NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    createSubMap(I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
    {
        if(fromKey == null || toKey == null )
        {
            throw new NullPointerException();
        }

        if(_comparator == null)
        {
            throw new UnsupportedOperationException("Index '" + _name + "' has no user-defined comparator");
        }

        return new IndexedSubMap<K, V, I>(this, fromKey, fromInclusive, toKey, toInclusive);
    }

    //
    // Used by subclasses to implement mapForXXX.
    //
    public NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    createMap()
    {
        if(_comparator == null)
        {
            throw new UnsupportedOperationException("Index '" + _name + "' has no user-defined comparator");
        }

        return new IndexedSubMap<K, V, I>(this, null, false, null, false);
    }

    ByteBuffer encodeKey(I k)
    {
        IceInternal.BasicStream str = _map.createWriteStream();
        encodeKey(k, str);
        return str.prepareWrite().b;
    }

    I decodeKey(ByteBuffer buf)
    {
        return decodeKey(_map.createReadStream(buf));
    }

    I decodeKey(com.sleepycat.db.DatabaseEntry entry)
    {
        ByteBuffer b = entry.getDataNIO();
        if(b != null)
        {
            return decodeKey(_map.createReadStream(b));
        }
        else
        {
            byte[] arr = entry.getData();
            assert(arr != null && entry.getOffset() == 0 && entry.getSize() == arr.length);
            return decodeKey(_map.createReadStream(arr));
        }
    }

    com.sleepycat.db.SecondaryDatabase
    db()
    {
        return _db;
    }

    String
    dbName()
    {
        return _dbName;
    }

    TraceLevels
    traceLevels()
    {
        return _trace;
    }

    java.util.Comparator<I>
    comparator()
    {
        return _comparator;
    }

    MapI<K, V>
    parent()
    {
        return _map;
    }

    boolean
    containsKey(Object o)
    {
        @SuppressWarnings("unchecked")
        I key = (I)o;

        ByteBuffer k = encodeKey(key);

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(k);
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        dbValue.setPartial(true);

        if(_trace.level >= 2)
        {
            _trace.logger.trace("Freeze.MapInternal.Index", "checking key in Db \"" + _dbName + "\"");
        }

        for(;;)
        {
            try
            {
                return _db.get(_map.connection().dbTxn(), dbKey, dbValue, null) ==
                    com.sleepycat.db.OperationStatus.SUCCESS;
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                if(_map.connection().dbTxn() != null)
                {
                    throw new DeadlockException(_trace.errorPrefix + "Db.get: " + e.getMessage(),
                                                _map.connection().currentTransaction(), e);
                }
                else
                {
                    if(_trace.deadlockWarning)
                    {
                        _trace.logger.warning(
                            "Deadlock in Freeze.MapInternal.Index.containsKey while " + "reading Db \"" + _dbName +
                            "\"; retrying...");
                    }
                    //
                    // Try again
                    //
                }
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                throw new DatabaseException(_trace.errorPrefix + "Db.get: " + e.getMessage(), e);
            }
        }
    }

    //
    // Used by the iterator created by the findByXXX methods.
    //
    EntryI<K, V>
    findFirstEntry(com.sleepycat.db.Cursor cursor, I fromKey)
        throws com.sleepycat.db.DatabaseException
    {
        com.sleepycat.db.SecondaryCursor c = (com.sleepycat.db.SecondaryCursor)cursor;

        assert(fromKey != null);
        ByteBuffer k = encodeKey(fromKey);

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbIKey = new com.sleepycat.db.DatabaseEntry(k);

        if(c.getSearchKey(dbIKey, dbKey, dbValue, null) == com.sleepycat.db.OperationStatus.SUCCESS)
        {
            return new EntryI<K, V>(_map, null, dbKey, UtilI.getBuffer(dbValue), UtilI.getBuffer(dbIKey));
        }

        return null;
    }

    //
    // Used by the iterator created by the findByXXX methods.
    //
    EntryI<K, V>
    findNextEntry(com.sleepycat.db.Cursor cursor, boolean onlyDups)
        throws com.sleepycat.db.DatabaseException
    {
        com.sleepycat.db.SecondaryCursor c = (com.sleepycat.db.SecondaryCursor)cursor;

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbIKey = new com.sleepycat.db.DatabaseEntry();

        com.sleepycat.db.OperationStatus status;

        if(onlyDups)
        {
            status = c.getNextDup(dbIKey, dbKey, dbValue, null);
        }
        else
        {
            status = c.getNext(dbIKey, dbKey, dbValue, null);
        }

        if(status == com.sleepycat.db.OperationStatus.SUCCESS)
        {
            return new EntryI<K, V>(_map, null, dbKey, UtilI.getBuffer(dbValue), UtilI.getBuffer(dbIKey));
        }

        return null;
    }

    //
    // Used by IndexedSubMap for ascending maps.
    //
    EntryI<K, V>
    firstEntry(com.sleepycat.db.Cursor cursor, I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
        throws com.sleepycat.db.DatabaseException
    {
        com.sleepycat.db.SecondaryCursor c = (com.sleepycat.db.SecondaryCursor)cursor;

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbIKey = new com.sleepycat.db.DatabaseEntry();

        com.sleepycat.db.OperationStatus status;

        if(fromKey != null)
        {
            ByteBuffer k = encodeKey(fromKey);
            dbIKey.setDataNIO(k);
            dbIKey.setReuseBuffer(false);

            status = c.getSearchKeyRange(dbIKey, dbKey, dbValue, null);

            if(status == com.sleepycat.db.OperationStatus.SUCCESS && !fromInclusive)
            {
                int cmp = compare(UtilI.getBuffer(dbIKey), k);
                assert(cmp >= 0);
                if(cmp == 0)
                {
                    status = c.getNextNoDup(dbIKey, dbKey, dbValue, null);
                }
            }
        }
        else
        {
            status = c.getFirst(dbIKey, dbKey, dbValue, null);
        }

        if(status == com.sleepycat.db.OperationStatus.SUCCESS)
        {
            return newEntry(dbIKey, dbKey, dbValue, fromKey, fromInclusive, toKey, toInclusive);
        }

        return null;
    }

    //
    // Used by IndexedSubMap for ascending maps.
    //
    EntryI<K, V>
    nextEntry(com.sleepycat.db.Cursor cursor, I toKey, boolean toInclusive)
        throws com.sleepycat.db.DatabaseException
    {
        com.sleepycat.db.SecondaryCursor c = (com.sleepycat.db.SecondaryCursor)cursor;

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbIKey = new com.sleepycat.db.DatabaseEntry();

        if(c.getNextNoDup(dbIKey, dbKey, dbValue, null) == com.sleepycat.db.OperationStatus.SUCCESS)
        {
            return newEntry(dbIKey, dbKey, dbValue, null, false, toKey, toInclusive);
        }

        return null;
    }

    //
    // Used by IndexedSubMap for descending maps.
    //
    EntryI<K, V>
    lastEntry(com.sleepycat.db.Cursor cursor, I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
        throws com.sleepycat.db.DatabaseException
    {
        com.sleepycat.db.SecondaryCursor c = (com.sleepycat.db.SecondaryCursor)cursor;

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbIKey = new com.sleepycat.db.DatabaseEntry();

        com.sleepycat.db.OperationStatus status;

        if(fromKey != null)
        {
            ByteBuffer k = encodeKey(fromKey);
            dbIKey.setDataNIO(k);
            dbIKey.setReuseBuffer(false);

            status = c.getSearchKeyRange(dbIKey, dbKey, dbValue, null);

            if(status == com.sleepycat.db.OperationStatus.SUCCESS && !fromInclusive)
            {
                int cmp = compare(UtilI.getBuffer(dbIKey), k);
                assert(cmp >= 0);
                if(cmp == 0)
                {
                    status = c.getPrevNoDup(dbIKey, dbKey, dbValue, null);
                }
            }
        }
        else
        {
            status = c.getLast(dbIKey, dbKey, dbValue, null);
        }

        if(status == com.sleepycat.db.OperationStatus.SUCCESS)
        {
            return newEntry(dbIKey, dbKey, dbValue, toKey, toInclusive, fromKey, fromInclusive);
        }

        return null;
    }

    //
    // Used by IndexedSubMap for descending maps.
    //
    EntryI<K, V>
    previousEntry(com.sleepycat.db.Cursor cursor, I toKey, boolean toInclusive)
        throws com.sleepycat.db.DatabaseException
    {
        com.sleepycat.db.SecondaryCursor c = (com.sleepycat.db.SecondaryCursor)cursor;

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
        com.sleepycat.db.DatabaseEntry dbIKey = new com.sleepycat.db.DatabaseEntry();

        if(c.getPrevNoDup(dbIKey, dbKey, dbValue, null) == com.sleepycat.db.OperationStatus.SUCCESS)
        {
            return newEntry(dbIKey, dbKey, dbValue, toKey, toInclusive, null, false);
        }

        return null;
    }

    //
    // marshalKey may be overridden by subclasses as an optimization.
    //
    protected ByteBuffer
    marshalKey(ByteBuffer value)
    {
        V decodedValue = _map.decodeValue(value);
        return encodeKey(extractKey(decodedValue));
    }

    private EntryI<K, V>
    newEntry(com.sleepycat.db.DatabaseEntry dbIKey, com.sleepycat.db.DatabaseEntry dbKey,
             com.sleepycat.db.DatabaseEntry dbValue, I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
    {
        I key = null;
        if(fromKey != null || toKey != null)
        {
            key = decodeKey(dbIKey);
            if(!checkRange(key, fromKey, fromInclusive, toKey, toInclusive))
            {
                return null;
            }
        }

        return new EntryI<K, V>(_map, null, dbKey, UtilI.getBuffer(dbValue), UtilI.getBuffer(dbIKey));
    }

    private boolean
    checkRange(I key, I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
    {
        if(fromKey != null)
        {
            int cmp = _comparator.compare(key, fromKey);
            if((fromInclusive && cmp < 0) || (!fromInclusive && cmp <= 0))
            {
                return false;
            }
        }
        if(toKey != null)
        {
            int cmp = _comparator.compare(key, toKey);
            if((toInclusive && cmp > 0) || (!toInclusive && cmp >= 0))
            {
                return false;
            }
        }
        return true;
    }

    private MapI<K, V> _map;
    private String _name;
    private java.util.Comparator<I> _comparator;
    private TraceLevels _trace;
    private String _dbName;
    private com.sleepycat.db.SecondaryDatabase _db;
}
