// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

//
// Sub-map of a Freeze Map or of another submap
//
//
// When it's based of an index, the key is the index key and the value
// is a Set of Map.Entry.
//

class SubMap extends java.util.AbstractMap implements java.util.SortedMap
{
    private class IndexValue extends java.util.AbstractSet
    {
        public java.util.Iterator
        iterator()
        {
            return _index.untypedFind(_myKey, true);
        }
               
        public int
        size()
        {
            return _index.untypedCount(_myKey);
        }

        public boolean equals(Object o)
        {
            if(o instanceof IndexValue)
            {
                IndexValue indexValue = (IndexValue)o;
                return indexValue._myKey.equals(_myKey);
            }
            else
            {
                return false;
            }
        }

        public int hashCode()
        {
            return _myKey.hashCode();
        }
        
        private IndexValue(Object key)
        {
            _myKey = key;
        }

        private Object
        getKey()
        {
            return _myKey;
        }

        private Object _myKey;
    }

    private class IndexEntry implements java.util.Map.Entry
    {
        public Object getKey()
        {
            return _value.getKey();
        }

        public Object getValue()
        {
            return _value;
        }
     
        public Object setValue(Object value)
        {
            throw new UnsupportedOperationException();
        } 

        public boolean equals(Object o)
        {
            if(o instanceof IndexEntry)
            {
                IndexEntry indexEntry = (IndexEntry)o;
                return indexEntry._value.equals(_value);
            }
            else
            {
                return false;
            }
        }

        public int hashCode()
        {
            return _value.hashCode();
        }
        
        SubMap parent()
        {
            return SubMap.this;
        }

        private IndexEntry(Object key)
        {
            _value = new IndexValue(key);
        }

        private IndexValue _value;
    }

    private class IndexIterator implements Map.EntryIterator
    {
        public boolean hasNext()
        {
            return _iterator.hasNext();
        }
       
        public Object next()
        {
            Map.Entry entry = (Map.Entry)_iterator.next();
            return new IndexEntry(_index.decodeKey(entry.getIndexBytes(),
                                                   _map.connection().communicator()));
        }
        
        public void remove()
        {
            _iterator.remove();
        }
        
        public void close()
        {
            _iterator.close();
        }

        public void destroy()
        {
            close();
        }
        
        private IndexIterator()
        {
            assert _index != null;
            _iterator = _map.createIterator(_index, _fromKey, _toKey);
        }

        Map.EntryIterator _iterator;
    }

    SubMap(Map map, Object fromKey, Object toKey)
    {
        _fromKey = fromKey;
        _toKey = toKey;
        _map = map;
        _index = null;

        if(fromKey != null && toKey != null)
        {
            if(map.comparator().compare(fromKey, toKey) >= 0)
            {
                throw new IllegalArgumentException();
            }
        }
    }
    
    SubMap(Map.Index index, Object fromKey, Object toKey)
    {
        _fromKey = fromKey;
        _toKey = toKey;
        _map = index.parent();
        _index = index;

        if(fromKey != null && toKey != null)
        {
            if(index.comparator().compare(fromKey, toKey) >= 0)
            {
                throw new IllegalArgumentException();
            }
        }
    }
    
    private SubMap(SubMap subMap, Object fromKey, Object toKey)
    {
        _fromKey = fromKey;
        _toKey = toKey;
        _map = subMap._map;
        _index = subMap._index;

        if(fromKey != null && toKey != null)
        {
            if(comparator().compare(fromKey, toKey) >= 0)
            {
                throw new IllegalArgumentException();
            }
        }
    }

    //
    // SortedMap methods
    //
    public java.util.Comparator comparator()
    {
        if(_index != null)
        {
            return _index.comparator();
        }
        else
        {
            return _map.comparator();
        }
    }

    public Object firstKey()
    {
        return _index != null ?
            _index.firstKey(_fromKey, _toKey) :
            _map.firstKey(_fromKey, _toKey);
    }

    public Object lastKey()
    {
        return _index != null ?
            _index.lastKey(_fromKey, _toKey) :
            _map.lastKey(_fromKey, _toKey);
    }

    public java.util.SortedMap headMap(Object toKey)
    {
        if(toKey == null)
        {
            throw new NullPointerException();
        }
        return new SubMap(this, _fromKey, toKey);
        
    }
    
    public java.util.SortedMap tailMap(Object fromKey)
    {
        if(fromKey == null)
        {
            throw new NullPointerException();
        }
        return new SubMap(this, fromKey, _toKey);
    } 
   
    public java.util.SortedMap subMap(Object fromKey, Object toKey)
    {
        if(fromKey == null || toKey == null )
        {
            throw new NullPointerException();
        }
        return new SubMap(this, fromKey, toKey);
    }

    //
    // java.util.Map methods
    //
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
                    if(_index == null)
                    {
                        return _map.createIterator(_index, _fromKey, _toKey);
                    }
                    else
                    {
                        return new IndexIterator();
                    }
                }
                
                public boolean
                contains(Object o)
                {
                    if(_index == null)
                    {
                        //
                        // If the main map contains this object, check it's within [fromKey, toKey[
                        //
                        if(_map.entrySet().contains(o))
                        {
                            Map.Entry entry = (Map.Entry)o;
                            return inRange(entry.getKey());
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if(o instanceof IndexEntry)
                        {
                            IndexEntry indexEntry = (IndexEntry)o;
                            return indexEntry.parent() == SubMap.this && 
                                _index.containsKey(indexEntry.getKey());
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
                        
                public boolean
                remove(Object o)
                {
                    if(_index == null)
                    {
                        if(o instanceof Map.Entry)
                        {
                            Map.Entry entry = (Map.Entry)o;
                            return inRange(entry.getKey()) && _map.entrySet().remove(o);
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        //
                        // Not yet implemented, should remove all objects that
                        // match this index-key
                        //
                        throw new UnsupportedOperationException();
                    }
                }
                
                public int
                size()
                {
                    throw new UnsupportedOperationException(); 
                }
                
                public boolean
                isEmpty()
                {
                    try
                    {
                        firstKey();
                        return false;
                    }
                    catch(NoSuchElementException e)
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


    public boolean constainsKey(Object key)
    {
        if(!inRange(key))
        {
            return false;
        }

        //
        // Then check if it's in the map
        //
        if(_index == null)
        {
            return _map.containsKey(key);
        }
        else
        {
            return _index.containsKey(key);
        }
    }
    
    
    public Object
    get(Object key)
    {
        if(!inRange(key))
        {
            return null;
        }
        
        if(_index == null)
        {
            return _map.get(key);
        }
        else
        {
            if(_index.containsKey(key))
            {
                return new IndexValue(key);
            }
            else
            {
                return null;
            }
        }
    }
    
    public Object
    remove(Object key)
    {
        if(!inRange(key))
        {
            return null;
        }
        
        if(_index == null)
        {
            return _map.remove(key);
        }
        else
        {
            //
            // Not yet implemented
            //
            throw new UnsupportedOperationException();
        }
    }

    public boolean
    fastRemove(Object key)
    {
        if(!inRange(key))
        {
            return false;
        }
        
        if(_index == null)
        {
            return _map.fastRemove(key);
        }
        else
        {
            //
            // Not yet implemented
            //
            throw new UnsupportedOperationException();
        }
    }

    
    private boolean inRange(Object key)
    {
        if(_fromKey != null && comparator().compare(_fromKey, key) > 0)
        {
            return false;
        }
        if(_toKey != null && comparator().compare(key, _toKey) >= 0)
        {
            return false;
        }
        return true;
    }

    private final Object _fromKey;
    private final Object _toKey;
    private final Map _map;
    private final Map.Index _index;
    private java.util.Set _entrySet;
}
