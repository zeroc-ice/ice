// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

import Freeze.NavigableMap;
import java.nio.ByteBuffer;

//
// Submap of a Freeze Map or of another submap
//

class SubMap<K, V> extends java.util.AbstractMap<K, V> implements NavigableMap<K, V>
{
    SubMap(MapI<K, V> map, K fromKey, boolean fromInclusive, K toKey, boolean toInclusive, boolean ascending)
    {
        _map = map;
        if(ascending)
        {
            _view = new AscendingView(fromKey, fromInclusive, toKey, toInclusive);
        }
        else
        {
            _view = new DescendingView(fromKey, fromInclusive, toKey, toInclusive);
        }
    }

    private
    SubMap(MapI<K, V> map, View v)
    {
        _map = map;
        _view = v;
    }

    //
    // NavigableMap methods
    //

    @Override
    public boolean
    fastRemove(K key)
    {
        if(!_view.inRange(key, true))
        {
            return false;
        }

        return _map.fastRemove(key);
    }

    @Override
    public java.util.Map.Entry<K, V>
    firstEntry()
    {
        return _view.first();
    }

    @Override
    public java.util.Map.Entry<K, V>
    lastEntry()
    {
        return _view.last();
    }

    @Override
    public java.util.Map.Entry<K, V>
    ceilingEntry(K key)
    {
        return _view.ceiling(key);
    }

    @Override
    public java.util.Map.Entry<K, V>
    floorEntry(K key)
    {
        return _view.floor(key);
    }

    @Override
    public java.util.Map.Entry<K, V>
    higherEntry(K key)
    {
        return _view.higher(key);
    }

    @Override
    public java.util.Map.Entry<K, V>
    lowerEntry(K key)
    {
        return _view.lower(key);
    }

    @Override
    public K
    ceilingKey(K key)
    {
        EntryI<K, V> e = _view.ceiling(key);
        return e != null ? e.getKey() : null;
    }

    @Override
    public K
    floorKey(K key)
    {
        EntryI<K, V> e = _view.floor(key);
        return e != null ? e.getKey() : null;
    }

    @Override
    public K
    higherKey(K key)
    {
        EntryI<K, V> e = _view.higher(key);
        return e != null ? e.getKey() : null;
    }

    @Override
    public K
    lowerKey(K key)
    {
        EntryI<K, V> e = _view.lower(key);
        return e != null ? e.getKey() : null;
    }

    @Override
    public java.util.Set<K>
    descendingKeySet()
    {
        return descendingMap().keySet();
    }

    @Override
    public NavigableMap<K, V>
    descendingMap()
    {
        if(_descendingMap == null)
        {
            View v = _view.descendingView();
            _descendingMap = new SubMap<K, V>(_map, v);
        }
        return _descendingMap;
    }

    @Override
    public NavigableMap<K, V>
    headMap(K toKey, boolean inclusive)
    {
        if(toKey == null)
        {
            throw new NullPointerException();
        }
        View v = _view.subView(null, false, toKey, inclusive);
        return new SubMap<K, V>(_map, v);
    }

    @Override
    public NavigableMap<K, V>
    subMap(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
    {
        if(fromKey == null || toKey == null)
        {
            throw new NullPointerException();
        }
        View v = _view.subView(fromKey, fromInclusive, toKey, toInclusive);
        return new SubMap<K, V>(_map, v);
    }

    @Override
    public NavigableMap<K, V>
    tailMap(K fromKey, boolean inclusive)
    {
        if(fromKey == null)
        {
            throw new NullPointerException();
        }
        View v = _view.subView(fromKey, inclusive, null, false);
        return new SubMap<K, V>(_map, v);
    }

    @Override
    public java.util.Map.Entry<K, V>
    pollFirstEntry()
    {
        EntryI<K, V> e = _view.first();
        if(e != null)
        {
            _map.removeImpl(e.getDbKey());
        }
        return e;
    }

    @Override
    public java.util.Map.Entry<K, V>
    pollLastEntry()
    {
        EntryI<K, V> e = _view.last();
        if(e != null)
        {
            _map.removeImpl(e.getDbKey());
        }
        return e;
    }

    //
    // SortedMap methods
    //

    @Override
    public java.util.Comparator<? super K>
    comparator()
    {
        return _view.comparator();
    }

    @Override
    public K
    firstKey()
    {
        EntryI<K, V> e = _view.first();
        if(e == null)
        {
            throw new java.util.NoSuchElementException();
        }
        return e.getKey();
    }

    @Override
    public K
    lastKey()
    {
        EntryI<K, V> e = _view.last();
        if(e == null)
        {
            throw new java.util.NoSuchElementException();
        }
        return e.getKey();
    }

    @Override
    public java.util.SortedMap<K, V>
    headMap(K toKey)
    {
        return headMap(toKey, false);
    }

    @Override
    public java.util.SortedMap<K, V>
    tailMap(K fromKey)
    {
        return tailMap(fromKey, true);
    }

    @Override
    public java.util.SortedMap<K, V>
    subMap(K fromKey, K toKey)
    {
        return subMap(fromKey, true, toKey, false);
    }

    //
    // Map methods
    //

    @Override
    public java.util.Set<java.util.Map.Entry<K, V>>
    entrySet()
    {
        if(_entrySet == null)
        {
            _entrySet = new java.util.AbstractSet<java.util.Map.Entry<K, V>>()
            {
                @Override
                public java.util.Iterator<java.util.Map.Entry<K, V>>
                iterator()
                {
                    return new IteratorI<K, V>(_map, _view);
                }

                @Override
                public boolean
                contains(Object o)
                {
                    //
                    // If the main map contains this object, verify it's within the range of this submap.
                    //
                    if(_map.entrySet().contains(o))
                    {
                        @SuppressWarnings("unchecked")
                        EntryI<K, V> entry = (EntryI<K, V>)o;
                        return _view.inRange(entry.getKey(), true);
                    }
                    else
                    {
                        return false;
                    }
                }

                @Override
                public boolean
                remove(Object o)
                {
                    if(o instanceof EntryI)
                    {
                        @SuppressWarnings("unchecked")
                        EntryI<K, V> entry = (EntryI<K, V>)o;
                        return _view.inRange(entry.getKey(), true) && _map.entrySet().remove(o);
                    }
                    else
                    {
                        return false;
                    }
                }

                @Override
                public int
                size()
                {
                    throw new UnsupportedOperationException();
                }

                @Override
                public boolean
                isEmpty()
                {
                    try
                    {
                        firstKey();
                        return false;
                    }
                    catch(java.util.NoSuchElementException e)
                    {
                        return true;
                    }
                }
            };
        }
        return _entrySet;
    }

    //
    // Put is not implemented (you have to put in the main map view)
    //

    public boolean
    constainsKey(Object key)
    {
        @SuppressWarnings("unchecked")
        K k = (K)key;
        if(!_view.inRange(k, true))
        {
            return false;
        }

        return _map.containsKey(k);
    }

    @Override
    public V
    get(Object key)
    {
        @SuppressWarnings("unchecked")
        K k = (K)key;
        if(!_view.inRange(k, true))
        {
            return null;
        }

        return _map.get(k);
    }

    @Override
    public V
    remove(Object key)
    {
        @SuppressWarnings("unchecked")
        K k = (K)key;
        if(!_view.inRange(k, true))
        {
            return null;
        }

        return _map.remove(k);
    }

    private abstract class View implements IteratorModel<K, V>, Search.KeyValidator
    {
        protected
        View(java.util.Comparator<? super K> comparator, K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
        {
            _comparator = comparator;
            _fromKey = fromKey;
            _fromInclusive = fromInclusive;
            _toKey = toKey;
            _toInclusive = toInclusive;

            //
            // Validate the key range.
            //
            if(_fromKey != null && _toKey != null)
            {
                int cmp = comparator.compare(_fromKey, _toKey);
                if(cmp > 0 || (cmp == 0 && !(_fromInclusive && _toInclusive)))
                {
                    throw new IllegalArgumentException();
                }
            }
        }

        protected
        View(View v, java.util.Comparator<? super K> comparator, K fromKey, boolean fromInclusive, K toKey,
             boolean toInclusive)
        {
            this(comparator, fromKey, fromInclusive, toKey, toInclusive);

            //
            // Verify that the key range is correct with respect to the original view.
            //
            if(!v.inRange(_fromKey, _fromInclusive) || !v.inRange(_toKey, _toInclusive))
            {
                throw new IllegalArgumentException();
            }
        }

        abstract Search.Type mapSearchType(Search.Type type);
        abstract View copy(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive);
        abstract View descendingView();

        final EntryI<K, V>
        first()
        {
            Search.Type type;
            ByteBuffer key = null;
            if(_fromKey != null)
            {
                type = _fromInclusive ? mapSearchType(Search.Type.CEILING) : mapSearchType(Search.Type.HIGHER);
                key = fromKeyBytes();
            }
            else
            {
                type = mapSearchType(Search.Type.FIRST);
            }
            return _map.entrySearch(type, key, true, this);
        }

        final EntryI<K, V>
        last()
        {
            Search.Type type;
            ByteBuffer key = null;
            if(_toKey != null)
            {
                type = _toInclusive ? mapSearchType(Search.Type.FLOOR) : mapSearchType(Search.Type.LOWER);
                key = toKeyBytes();
            }
            else
            {
                type = mapSearchType(Search.Type.LAST);
            }
            return _map.entrySearch(type, key, true, this);
        }

        final EntryI<K, V>
        ceiling(K key)
        {
            ByteBuffer k = _map.encodeKey(key);
            return _map.entrySearch(mapSearchType(Search.Type.CEILING), k, true, this);
        }

        final EntryI<K, V>
        floor(K key)
        {
            ByteBuffer k = _map.encodeKey(key);
            return _map.entrySearch(mapSearchType(Search.Type.FLOOR), k, true, this);
        }

        final EntryI<K, V>
        higher(K key)
        {
            ByteBuffer k = _map.encodeKey(key);
            return _map.entrySearch(mapSearchType(Search.Type.HIGHER), k, true, this);
        }

        final EntryI<K, V>
        lower(K key)
        {
            ByteBuffer k = _map.encodeKey(key);
            return _map.entrySearch(mapSearchType(Search.Type.LOWER), k, true, this);
        }

        final View
        subView(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
        {
            if(fromKey == null)
            {
                fromKey = _fromKey;
                fromInclusive = _fromInclusive;
            }
            if(toKey == null)
            {
                toKey = _toKey;
                toInclusive = _toInclusive;
            }
            return copy(fromKey, fromInclusive, toKey, toInclusive);
        }

        //
        // IteratorModel methods (partial)
        //

        @Override
        final public String
        dbName()
        {
            return _map.dbName();
        }

        @Override
        final public TraceLevels
        traceLevels()
        {
            return _map.traceLevels();
        }

        @Override
        final public com.sleepycat.db.Cursor
        openCursor()
            throws com.sleepycat.db.DatabaseException
        {
            return _map.openCursor();
        }

        //
        // Search.KeyValidator methods
        //

        @Override
        final public boolean
        keyInRange(ByteBuffer key)
        {
            K k = _map.decodeKey(key);
            return inRange(k, true);
        }

        final boolean
        inRange(K key, boolean inclusive)
        {
            return !tooLow(key, inclusive, _fromKey, _fromInclusive) &&
                   !tooHigh(key, inclusive, _toKey, _toInclusive);
        }

        final java.util.Comparator<? super K>
        comparator()
        {
            return _comparator;
        }

        final protected ByteBuffer
        fromKeyBytes()
        {
            if(_fromKey != null && _fromKeyBytes == null)
            {
                _fromKeyBytes = _map.encodeKey(_fromKey);
            }
            return _fromKeyBytes;
        }

        final protected ByteBuffer
        toKeyBytes()
        {
            if(_toKey != null && _toKeyBytes == null)
            {
                _toKeyBytes = _map.encodeKey(_toKey);
            }
            return _toKeyBytes;
        }

        final protected boolean
        tooLow(K key, boolean inclusive, K targetKey, boolean targetInclusive)
        {
            if(key != null && targetKey != null)
            {
                int cmp = comparator().compare(key, targetKey);
                if(cmp < 0 || (cmp == 0 && inclusive && !targetInclusive))
                {
                    return true;
                }
            }
            return false;
        }

        final protected boolean
        tooHigh(K key, boolean inclusive, K targetKey, boolean targetInclusive)
        {
            if(key != null && targetKey != null)
            {
                int cmp = comparator().compare(key, targetKey);
                if(cmp > 0 || (cmp == 0 && inclusive && !targetInclusive))
                {
                    return true;
                }
            }
            return false;
        }

        final java.util.Comparator<? super K> _comparator;
        final K _fromKey;
        final boolean _fromInclusive;
        final K _toKey;
        final boolean _toInclusive;
        private ByteBuffer _fromKeyBytes;
        private ByteBuffer _toKeyBytes;
    }

    private class AscendingView extends View
    {
        AscendingView(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
        {
            super(_map.comparator(), fromKey, fromInclusive, toKey, toInclusive);
        }

        AscendingView(View v, K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
        {
            super(v, _map.comparator(), fromKey, fromInclusive, toKey, toInclusive);
        }

        //
        // View methods
        //

        @Override
        Search.Type
        mapSearchType(Search.Type type)
        {
            return type;
        }

        @Override
        View
        copy(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
        {
            return new AscendingView(this, fromKey, fromInclusive, toKey, toInclusive);
        }

        @Override
        View
        descendingView()
        {
            return new DescendingView(this, _toKey, _toInclusive, _fromKey, _fromInclusive);
        }

        //
        // IteratorModel methods
        //

        @Override
        public EntryI<K, V>
        firstEntry(com.sleepycat.db.Cursor cursor)
            throws com.sleepycat.db.DatabaseException
        {
            return _map.firstEntry(cursor, _fromKey, _fromInclusive, _toKey, _toInclusive);
        }

        @Override
        public EntryI<K, V>
        nextEntry(com.sleepycat.db.Cursor cursor)
            throws com.sleepycat.db.DatabaseException
        {
            return _map.nextEntry(cursor, _toKey, _toInclusive);
        }
    }

    private class DescendingView extends View
    {
        DescendingView(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
        {
            super(java.util.Collections.reverseOrder(_map.comparator()), fromKey, fromInclusive, toKey, toInclusive);
        }

        DescendingView(View v, K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
        {
            super(v, java.util.Collections.reverseOrder(_map.comparator()), fromKey, fromInclusive, toKey, toInclusive);
        }

        //
        // View methods
        //

        @Override
        Search.Type
        mapSearchType(Search.Type type)
        {
            return type.descending();
        }

        @Override
        View
        copy(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive)
        {
            return new DescendingView(this, fromKey, fromInclusive, toKey, toInclusive);
        }

        @Override
        View
        descendingView()
        {
            return new AscendingView(this, _toKey, _toInclusive, _fromKey, _fromInclusive);
        }

        //
        // IteratorModel methods
        //

        @Override
        public EntryI<K, V>
        firstEntry(com.sleepycat.db.Cursor cursor)
            throws com.sleepycat.db.DatabaseException
        {
            return _map.lastEntry(cursor, _fromKey, _fromInclusive, _toKey, _toInclusive);
        }

        @Override
        public EntryI<K, V>
        nextEntry(com.sleepycat.db.Cursor cursor)
            throws com.sleepycat.db.DatabaseException
        {
            return _map.previousEntry(cursor, _toKey, _toInclusive);
        }
    }

    private final MapI<K, V> _map;
    private final View _view;
    private java.util.Set<java.util.Map.Entry<K, V>> _entrySet;
    private NavigableMap<K, V> _descendingMap;
}
