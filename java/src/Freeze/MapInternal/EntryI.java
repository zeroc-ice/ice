// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

class EntryI<K, V> implements java.util.Map.Entry<K, V>
{
    public
    EntryI(MapI<K, V> map, K key, com.sleepycat.db.DatabaseEntry dbKey, byte[] valueBytes, byte[] indexBytes)
    {
        _map = map;
        _dbKey = dbKey;
        _valueBytes = valueBytes;
        _indexBytes = indexBytes;
        _communicator = map.connection().getCommunicator();
        _encoding = map.connection().getEncoding();
        _key = key;
        _haveKey = key != null;
    }

    public K
    getKey()
    {
        if(!_haveKey)
        {
            assert(_dbKey != null);
            _key = _map.decodeKey(_dbKey.getData(), _communicator, _encoding);
            _haveKey = true;
        }
        return _key;
    }

    public V
    getValue()
    {
        if(!_haveValue)
        {
            assert(_valueBytes != null);
            _value = _map.decodeValue(_valueBytes, _communicator, _encoding);
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

    public V
    setValue(V value)
    {
        V old = getValue();
        if(_iterator != null)
        {
            _iterator.setValue(this, value);
        }
        else
        {
            _map.putImpl(_dbKey, value);
        }
        _value = value;
        _haveValue = true;
        return old;
    }

    public boolean
    equals(Object o)
    {
        if(!(o instanceof EntryI))
        {
            return false;
        }
        @SuppressWarnings("unchecked")
        EntryI<K, V> e = (EntryI<K, V>)o;
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

    void
    iterator(IteratorI<K, V> iterator)
    {
        _iterator = iterator;
    }

    com.sleepycat.db.DatabaseEntry
    getDbKey()
    {
        return _dbKey;
    }

    private static boolean
    eq(Object o1, Object o2)
    {
        return (o1 == null ? o2 == null : o1.equals(o2));
    }

    private MapI<K, V> _map;
    private com.sleepycat.db.DatabaseEntry _dbKey;
    private byte[] _valueBytes;
    private byte[] _indexBytes;

    private Ice.Communicator _communicator;
    private Ice.EncodingVersion _encoding;
    private K _key;
    private boolean _haveKey = false;
    private V _value;
    private boolean _haveValue = false;
    private IteratorI<K, V> _iterator;
}
