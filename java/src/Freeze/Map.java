// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

public interface Map<K, V> extends NavigableMap<K, V>
{
    //
    // Faster alternative to the standard put() method because it
    // doesn't read and decode the old value.
    //
    void fastPut(K key, V value);

    void close();

    //
    // Close all iterators for this map. Returns the number of
    // iterators that were closed.
    //
    int closeAllIterators();

    //
    // Close this map and destroy the underlying Berkeley DB database.
    //
    void destroy();

    /**
     *
     * The entry iterator allows clients to explicitly close the iterator
     * and free resources allocated for the iterator as soon as possible.
     *
     **/
    public interface EntryIterator<T> extends java.util.Iterator<T>
    {
        void close();
        void destroy(); // an alias for close
    }

    Connection getConnection();
    void closeDb();
}
