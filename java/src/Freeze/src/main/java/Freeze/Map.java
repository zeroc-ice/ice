// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

/**
 * Interface for Freeze maps.
 *
 * @see Connection
 **/
public interface Map<K, V> extends NavigableMap<K, V>
{
    /**
     * Alternative to <code>java.util.SortedMap.put</code>. This version
     * is more efficient because it does not decode and return the
     * old value.
     **/
    void fastPut(K key, V value);

    /**
     * Closes the database associated with this map, as well as all open iterators.
     * A map must be closed when it is no longer needed, either directly, or by
     * closing the {@link Connection} associated with this map.
     **/
    void close();

    /**
     * Closes all iterators for this map.
     *
     * @return The number of iterators that were closed.
     **/
    int closeAllIterators();

    /**
     * Closes this map and destroys the underlying Berkeley DB database along with any indexes.
     **/
    void destroy();

    /**
     * An <code>EntryIterator</code> allows the application to explicitly
     * close an iterator and free resources allocated for the iterator
     * in a timely fashion.
     **/
    public interface EntryIterator<T> extends java.util.Iterator<T>
    {
        /**
         * Closes this iterator, reclaiming associated resources.
         **/
        void close();

        /**
         * Closes this iterator, reclaiming associated resources. This
         * method is an alias for {@link #close}.
         **/
        void destroy();
    }

    /**
     * Returns the connection associated with this map.
     *
     * @return The connection associated with this map.
     **/
    Connection getConnection();

    /**
     * Closes the database for this map.
     **/
    void closeDb();
}
