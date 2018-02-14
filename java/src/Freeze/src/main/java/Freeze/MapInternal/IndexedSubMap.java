// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

import Freeze.DatabaseException;
import Freeze.Map;
import Freeze.NavigableMap;
import java.nio.ByteBuffer;

//
// Indexed submap of a Freeze Map or of another submap
//
class IndexedSubMap<K, V, I>
    extends java.util.AbstractMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    implements NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
{
    private class Value extends java.util.AbstractSet<java.util.Map.Entry<K, V>>
    {
        @Override
        public java.util.Iterator<java.util.Map.Entry<K, V>>
        iterator()
        {
            return _index.find(_myKey, true);
        }

        @Override
        public int
        size()
        {
            return _index.count(_myKey);
        }

        @Override
        public boolean
        equals(Object o)
        {
            if(o instanceof IndexedSubMap.Value)
            {
                IndexedSubMap<?,?,?>.Value v = (IndexedSubMap<?,?,?>.Value)o;
                return v._myKey.equals(_myKey);
            }
            else
            {
                return false;
            }
        }

        @Override
        public int
        hashCode()
        {
            return _myKey.hashCode();
        }

        private
        Value(I key)
        {
            _myKey = key;
        }

        private I
        getKey()
        {
            return _myKey;
        }

        private I _myKey;
    }

    private class Entry implements java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    {
        @Override
        public I
        getKey()
        {
            return _value.getKey();
        }

        @Override
        public java.util.Set<java.util.Map.Entry<K, V>>
        getValue()
        {
            return _value;
        }

        @Override
        public java.util.Set<java.util.Map.Entry<K, V>>
        setValue(java.util.Set<java.util.Map.Entry<K, V>> value)
        {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean
        equals(Object o)
        {
            if(o instanceof IndexedSubMap.Entry)
            {
                IndexedSubMap<?,?,?>.Entry e = (IndexedSubMap<?,?,?>.Entry)o;
                return e._value.equals(_value);
            }
            else
            {
                return false;
            }
        }

        @Override
        public int
        hashCode()
        {
            return _value.hashCode();
        }

        IndexedSubMap<K, V, I>
        parent()
        {
            return IndexedSubMap.this;
        }

        private
        Entry(I key)
        {
            _value = new Value(key);
        }

        private Value _value;
    }

    private class Iterator
        implements Map.EntryIterator<java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>>
    {
        @Override
        public boolean
        hasNext()
        {
            return _iterator.hasNext();
        }

        @Override
        public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
        next()
        {
            EntryI<K, V> entry = (EntryI<K, V>)_iterator.next();
            return new Entry(_index.decodeKey(entry.getIndexBytes()));
        }

        @Override
        public void
        remove()
        {
            _iterator.remove();
        }

        @Override
        public void
        close()
        {
            _iterator.close();
        }

        @Override
        public void
        destroy()
        {
            close();
        }

        private
        Iterator()
        {
            assert(_index != null);
            _iterator = new IteratorI<K, V>(_map, _view);
        }

        Map.EntryIterator<java.util.Map.Entry<K, V>> _iterator;
    }

    IndexedSubMap(Index<K, V, I> index, I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
    {
        _map = index.parent();
        _index = index;
        _view = new AscendingView(fromKey, fromInclusive, toKey, toInclusive);
    }

    private
    IndexedSubMap(Index<K, V, I> index, View v)
    {
        _map = index.parent();
        _index = index;
        _view = v;
    }

    //
    // NavigableMap methods
    //

    @Override
    public boolean
    fastRemove(I key)
    {
        if(!_view.inRange(key, true))
        {
            return false;
        }

        //
        // Not yet implemented
        //
        throw new UnsupportedOperationException();
    }

    @Override
    public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    firstEntry()
    {
        return _view.first();
    }

    @Override
    public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    lastEntry()
    {
        return _view.last();
    }

    @Override
    public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    ceilingEntry(I key)
    {
        return _view.ceiling(key);
    }

    @Override
    public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    floorEntry(I key)
    {
        return _view.floor(key);
    }

    @Override
    public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    higherEntry(I key)
    {
        return _view.higher(key);
    }

    @Override
    public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    lowerEntry(I key)
    {
        return _view.lower(key);
    }

    @Override
    public I
    ceilingKey(I key)
    {
        Entry e = _view.ceiling(key);
        return e != null ? e.getKey() : null;
    }

    @Override
    public I
    floorKey(I key)
    {
        Entry e = _view.floor(key);
        return e != null ? e.getKey() : null;
    }

    @Override
    public I
    higherKey(I key)
    {
        Entry e = _view.higher(key);
        return e != null ? e.getKey() : null;
    }

    @Override
    public I
    lowerKey(I key)
    {
        Entry e = _view.lower(key);
        return e != null ? e.getKey() : null;
    }

    @Override
    public java.util.Set<I>
    descendingKeySet()
    {
        return descendingMap().keySet();
    }

    @Override
    public NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    descendingMap()
    {
        if(_descendingMap == null)
        {
            View v = _view.descendingView();
            _descendingMap = new IndexedSubMap<K, V, I>(_index, v);
        }
        return _descendingMap;
    }

    @Override
    public NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    headMap(I toKey, boolean inclusive)
    {
        if(toKey == null)
        {
            throw new NullPointerException();
        }
        View v = _view.subView(null, false, toKey, inclusive);
        return new IndexedSubMap<K, V, I>(_index, v);
    }

    @Override
    public NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    subMap(I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
    {
        if(fromKey == null || toKey == null)
        {
            throw new NullPointerException();
        }
        View v = _view.subView(fromKey, fromInclusive, toKey, toInclusive);
        return new IndexedSubMap<K, V, I>(_index, v);
    }

    @Override
    public NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    tailMap(I fromKey, boolean inclusive)
    {
        if(fromKey == null)
        {
            throw new NullPointerException();
        }
        View v = _view.subView(fromKey, inclusive, null, false);
        return new IndexedSubMap<K, V, I>(_index, v);
    }

    @Override
    public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    pollFirstEntry()
    {
        //
        // Not yet implemented
        //
        throw new UnsupportedOperationException();
    }

    @Override
    public java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>
    pollLastEntry()
    {
        //
        // Not yet implemented
        //
        throw new UnsupportedOperationException();
    }

    //
    // SortedMap methods
    //

    @Override
    public java.util.Comparator<? super I>
    comparator()
    {
        return _view.comparator();
    }

    @Override
    public I
    firstKey()
    {
        Entry e = _view.first();
        if(e == null)
        {
            throw new java.util.NoSuchElementException();
        }
        return e.getKey();
    }

    @Override
    public I
    lastKey()
    {
        Entry e = _view.last();
        if(e == null)
        {
            throw new java.util.NoSuchElementException();
        }
        return e.getKey();
    }

    @Override
    public java.util.SortedMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    headMap(I toKey)
    {
        return headMap(toKey, false);
    }

    @Override
    public java.util.SortedMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    tailMap(I fromKey)
    {
        return tailMap(fromKey, true);
    }

    @Override
    public java.util.SortedMap<I, java.util.Set<java.util.Map.Entry<K, V>>>
    subMap(I fromKey, I toKey)
    {
        return subMap(fromKey, true, toKey, false);
    }

    //
    // Map methods
    //

    @Override
    public java.util.Set<java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>>
    entrySet()
    {
        if(_entrySet == null)
        {
            _entrySet = new java.util.AbstractSet<java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>>()
            {
                @Override
                public java.util.Iterator<java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>>
                iterator()
                {
                    return new Iterator();
                }

                @Override
                public boolean
                contains(Object o)
                {
                    if(o instanceof IndexedSubMap.Entry)
                    {
                        IndexedSubMap<?,?,?>.Entry e = (IndexedSubMap<?,?,?>.Entry)o;
                        return e.parent() == IndexedSubMap.this && _index.containsKey(e.getKey());
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
                    //
                    // Not yet implemented, should remove all objects that
                    // match this index-key
                    //
                    throw new UnsupportedOperationException();
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
        I k = (I)key;
        if(!_view.inRange(k, true))
        {
            return false;
        }

        return _index.containsKey(key);
    }

    @Override
    public java.util.Set<java.util.Map.Entry<K, V>>
    get(Object key)
    {
        @SuppressWarnings("unchecked")
        I k = (I)key;
        if(!_view.inRange(k, true))
        {
            return null;
        }

        if(_index.containsKey(k))
        {
            return new Value(k);
        }
        else
        {
            return null;
        }
    }

    @Override
    public java.util.Set<java.util.Map.Entry<K, V>>
    remove(Object key)
    {
        @SuppressWarnings("unchecked")
        I k = (I)key;
        if(!_view.inRange(k, true))
        {
            return null;
        }

        //
        // Not yet implemented
        //
        throw new UnsupportedOperationException();
    }

    private Entry
    entrySearch(Search.Type type, ByteBuffer key)
    {
        if(type != Search.Type.FIRST && type != Search.Type.LAST && key == null)
        {
            throw new NullPointerException();
        }

        if(_index.db() == null)
        {
            DatabaseException ex = new DatabaseException();
            ex.message = _index.traceLevels().errorPrefix + "\"" + _index.dbName() + "\" has been closed";
            throw ex;
        }

        com.sleepycat.db.DatabaseEntry dbKey = new com.sleepycat.db.DatabaseEntry(key);

        if(Search.search(type, _map.connection(), _index.dbName(), _index.db(), dbKey, null, _index, _view,
                         _index.traceLevels()))
        {
            I k = _index.decodeKey(dbKey);
            return new Entry(k);
        }

        return null;
    }

    private abstract class View implements IteratorModel<K, V>, Search.KeyValidator
    {
        protected
        View(java.util.Comparator<? super I> comparator, I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
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
        View(View v, java.util.Comparator<? super I> comparator, I fromKey, boolean fromInclusive, I toKey,
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
        abstract View copy(I fromKey, boolean fromInclusive, I toKey, boolean toInclusive);
        abstract View descendingView();

        final Entry
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
            return entrySearch(type, key);
        }

        final Entry
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
            return entrySearch(type, key);
        }

        final Entry
        ceiling(I key)
        {
            ByteBuffer k = _index.encodeKey(key);
            return entrySearch(mapSearchType(Search.Type.CEILING), k);
        }

        final Entry
        floor(I key)
        {
            ByteBuffer k = _index.encodeKey(key);
            return entrySearch(mapSearchType(Search.Type.FLOOR), k);
        }

        final Entry
        higher(I key)
        {
            ByteBuffer k = _index.encodeKey(key);
            return entrySearch(mapSearchType(Search.Type.HIGHER), k);
        }

        final Entry
        lower(I key)
        {
            ByteBuffer k = _index.encodeKey(key);
            return entrySearch(mapSearchType(Search.Type.LOWER), k);
        }

        final View
        subView(I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
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
            return _index.dbName();
        }

        @Override
        final public TraceLevels
        traceLevels()
        {
            return _index.traceLevels();
        }

        @Override
        final public com.sleepycat.db.Cursor
        openCursor()
            throws com.sleepycat.db.DatabaseException
        {
            return _index.db().openSecondaryCursor(_map.connection().dbTxn(), null);
        }

        //
        // Search.KeyValidator methods
        //

        @Override
        final public boolean
        keyInRange(ByteBuffer key)
        {
            I k = _index.decodeKey(key);
            return inRange(k, true);
        }

        final boolean
        inRange(I key, boolean inclusive)
        {
            return !tooLow(key, inclusive, _fromKey, _fromInclusive) &&
                   !tooHigh(key, inclusive, _toKey, _toInclusive);
        }

        final java.util.Comparator<? super I>
        comparator()
        {
            return _comparator;
        }

        final protected ByteBuffer
        fromKeyBytes()
        {
            if(_fromKey != null && _fromKeyBytes == null)
            {
                _fromKeyBytes = _index.encodeKey(_fromKey);
            }
            return _fromKeyBytes;
        }

        final protected ByteBuffer
        toKeyBytes()
        {
            if(_toKey != null && _toKeyBytes == null)
            {
                _toKeyBytes = _index.encodeKey(_toKey);
            }
            return _toKeyBytes;
        }

        final protected boolean
        tooLow(I key, boolean inclusive, I targetKey, boolean targetInclusive)
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
        tooHigh(I key, boolean inclusive, I targetKey, boolean targetInclusive)
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

        final java.util.Comparator<? super I> _comparator;
        final I _fromKey;
        final boolean _fromInclusive;
        final I _toKey;
        final boolean _toInclusive;
        private ByteBuffer _fromKeyBytes;
        private ByteBuffer _toKeyBytes;
    }

    private class AscendingView extends View
    {
        AscendingView(I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
        {
            super(_index.comparator(), fromKey, fromInclusive, toKey, toInclusive);
        }

        AscendingView(View v, I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
        {
            super(v, _index.comparator(), fromKey, fromInclusive, toKey, toInclusive);
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
        copy(I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
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
            return _index.firstEntry(cursor, _fromKey, _fromInclusive, _toKey, _toInclusive);
        }

        @Override
        public EntryI<K, V>
        nextEntry(com.sleepycat.db.Cursor cursor)
            throws com.sleepycat.db.DatabaseException
        {
            return _index.nextEntry(cursor, _toKey, _toInclusive);
        }
    }

    private class DescendingView extends View
    {
        DescendingView(I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
        {
            super(java.util.Collections.reverseOrder(_index.comparator()), fromKey, fromInclusive, toKey, toInclusive);
        }

        DescendingView(View v, I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
        {
            super(v, java.util.Collections.reverseOrder(_index.comparator()), fromKey, fromInclusive, toKey,
                  toInclusive);
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
        copy(I fromKey, boolean fromInclusive, I toKey, boolean toInclusive)
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
            return _index.lastEntry(cursor, _fromKey, _fromInclusive, _toKey, _toInclusive);
        }

        @Override
        public EntryI<K, V>
        nextEntry(com.sleepycat.db.Cursor cursor)
            throws com.sleepycat.db.DatabaseException
        {
            return _index.previousEntry(cursor, _toKey, _toInclusive);
        }
    }

    private final MapI<K, V> _map;
    private final Index<K, V, I> _index;
    private final View _view;
    private java.util.Set<java.util.Map.Entry<I, java.util.Set<java.util.Map.Entry<K, V>>>> _entrySet;
    private NavigableMap<I, java.util.Set<java.util.Map.Entry<K, V>>> _descendingMap;
}
