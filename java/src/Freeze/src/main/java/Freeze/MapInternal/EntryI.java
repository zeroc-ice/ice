// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

import java.nio.ByteBuffer;

class EntryI<K, V> implements java.util.Map.Entry<K, V>
{
    public
    EntryI(MapI<K, V> map, K key, com.sleepycat.db.DatabaseEntry dbKey, ByteBuffer valueBytes, ByteBuffer indexBytes)
    {
        _map = map;
        _dbKey = dbKey;
        _valueBytes = valueBytes;
        _indexBytes = indexBytes;
        _key = key;
        _haveKey = key != null;
    }

    @Override
    public K
    getKey()
    {
        if(!_haveKey)
        {
            assert(_dbKey != null);
            _key = _map.decodeKey(_dbKey);
            _haveKey = true;
        }
        return _key;
    }

    @Override
    public V
    getValue()
    {
        if(!_haveValue)
        {
            assert(_valueBytes != null);
            _value = _map.decodeValue(_valueBytes);
            _haveValue = true;
            //
            // Not needed anymore
            //
            _valueBytes = null;
        }
        return _value;
    }

    public ByteBuffer
    getIndexBytes()
    {
        return _indexBytes;
    }

    @Override
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

    @Override
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

    @Override
    public int
    hashCode()
    {
        return ((getKey()   == null) ? 0 : getKey().hashCode()) ^
               ((getValue() == null) ? 0 : getValue().hashCode());
    }

    @Override
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
    private ByteBuffer _valueBytes;
    private ByteBuffer _indexBytes;

    private K _key;
    private boolean _haveKey = false;
    private V _value;
    private boolean _haveValue = false;
    private IteratorI<K, V> _iterator;
}
