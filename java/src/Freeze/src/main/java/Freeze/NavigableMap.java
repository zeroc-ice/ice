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
 * Parent interface for Freeze maps.
 **/
public interface NavigableMap<K, V> extends java.util.SortedMap<K, V>
{
    /**
     * Alternative to <code>java.util.SortedMap.remove</code>. This version
     * is more efficient because it does not decode and return the
     * old value.
     *
     * @param key The key of the entry to remove.
     * @return <code>true</code> if an entry for <code>key</code> was removed;
     * <code>false</code>, otherwise.
     **/
    boolean fastRemove(K key);

    /**
     * Returns The entry with the least key.
     *
     * @return The entry with the least key, or <code>null</code> if the map is empty.
     **/
    java.util.Map.Entry<K, V> firstEntry();

    /**
     * Returns The entry with the greatest key.
     *
     * @return The entry with the greatest key, or <code>null</code> if the map is empty.
     **/
    java.util.Map.Entry<K, V> lastEntry();

    /**
     * Returns the entry with the least key greater than or equal to the given key.
     *
     * @param key The key.
     * @return The entry with the least key greater than or equal to the given key,
     * or <code>null</code> if there is no such entry.
     **/
    java.util.Map.Entry<K, V> ceilingEntry(K key);

    /**
     * Returns the entry with the greatest key less than or equal to the given key.
     *
     * @param key The key.
     * @return The entry with the greatest key less than or equal to the given key,
     * or <code>null</code> if there is no such entry.
     **/
    java.util.Map.Entry<K, V> floorEntry(K key);

    /**
     * Returns the entry with the least key strictly greater than the given key.
     *
     * @param key The key.
     * @return The entry with the least key strictly greater than the given key,
     * or <code>null</code> if there is no such entry.
     **/
    java.util.Map.Entry<K, V> higherEntry(K key);

    /**
     * Returns the entry with the greatest key strictly less than the given key.
     *
     * @param key The key.
     * @return The entry with the greatest key strictly less than the given key,
     * or <code>null</code> if there is no such entry.
     **/
    java.util.Map.Entry<K, V> lowerEntry(K key);

    /**
     * Returns the least key greater than or equal to the given key.
     *
     * @param key The key.
     * @return The least key greater than or equal to the given key,
     * or <code>null</code> if there is no such key.
     **/
    K ceilingKey(K key);

    /**
     * Returns the greatest key less than or equal to the given key.
     *
     * @param key The key.
     * @return The greatest key less than or equal to the given key,
     * or <code>null</code> if there is no such key.
     **/
    K floorKey(K key);

    /**
     * Returns the least key strictly greater than the given key.
     *
     * @return The least key strictly greater than the given key,
     * or <code>null</code> if there is no such key.
     **/
    K higherKey(K key);

    /**
     * Returns the greatest key strictly less than the given key.
     *
     * @return The greatest key strictly less than the given key,
     * or <code>null</code> if there is no such key.
     **/
    K lowerKey(K key);

    /**
     * Returns a reverse order <code>Set</code> view of the keys contained in this map. The set's iterator
     * returns the keys in descending order. The set is backed by the map, so changes to the map are reflected
     * in the set and vice versa.
     * <p>
     * Insertions cannot be made via the returned set and must be made on the main map.
     * <p>
     * <code>remove</code> and <code>fastRemove</code> on the returned set or on an iterator for the returned
     * set are supported, but only if this map does not have an index; otherwise, attempts to remove an element
     * raise <code>UnsupportedOperationException</code>.
     * <p>
     * If the map is modified while an iteration over the set is in progress
     * (except through the iterator's own <code>remove</code> or <code>fastRemove</code> operation), the results
     * of the iteration are undefined.
     *
     * @return A reverse order set view of the keys in this map.
     **/
    java.util.Set<K> descendingKeySet();

    /**
     * Returns a reverse order view of the mappings contained in this map.
     * The descending map is backed by this map, so changes to the map are reflected in the descending map
     * and vice versa.
     *<p>
     * Insertions cannot be made via the descending map and must be made on the main map.
     * <p>
     * <code>remove</code> and <code>fastRemove</code> on the descending map or on an iterator for the descending
     * map are supported, but only if this map does not have an index; otherwise, attempts to remove an entry
     * raise <code>UnsupportedOperationException</code>.
     * <p>
     * If either map is modified while an iteration over either map is
     * in progress (except through the iterator's own <code>remove</code> or <code>fastRemove</code>operation),
     * the results of the iteration are undefined.
     *
     * @return a Reverse order view of this map.
     **/
    NavigableMap<K, V> descendingMap();

    /**
     * Returns a view of the portion of this map whose keys are strictly less than <code>toKey</code>, or less than
     * or equal to <code>toKey</code> if <code>inclusive</code> is true.
     * The returned map is backed by this map, so changes to this map are reflected in the returned map
     * and vice versa.
     *<p>
     * Insertions cannot be made via the returned map and must be made on the main map.
     * <p>
     * <code>remove</code> and <code>fastRemove</code> on the returned map or on an iterator for the returned
     * map are supported, but only if this map does not have an index; otherwise, attempts to remove an entry
     * raise <code>UnsupportedOperationException</code>.
     * <p>
     * The returned map throws an <code>IllegalArgumentException</code> on an attempt to insert a key
     * outside its range.
     *
     * @param toKey High endpoint of the keys in the returned map.
     * @param inclusive If <code>true</code>, the endpoint is included in the returned map; otherwise, the endpoint
     * is excluded.
     * @return A view of the portion of this map whose keys are strictly less than <code>toKey</code>, or
     * less than or equal to <code>toKey</code> if <code>inclusive</code> is true.
     **/
    NavigableMap<K, V> headMap(K toKey, boolean inclusive);

    /**
     * Returns a view of the portion of this map whose keys are strictly greater than <code>fromKey</code>, or greater
     * than or equal to <code>fromKey</code> if <code>inclusive</code> is true.
     *<p>
     * Insertions cannot be made via the returned map and must be made on the main map.
     * <p>
     * <code>remove</code> and <code>fastRemove</code> on the returned map or on an iterator for the returned
     * map are supported, but only if this map does not have an index; otherwise, attempts to remove an entry
     * raise <code>UnsupportedOperationException</code>.
     * <p>
     * The returned map throws an <code>IllegalArgumentException</code> on an attempt to insert a key
     * outside its range.
     *
     * @param fromKey Low endpoint of the keys in the returned map.
     * @param inclusive If <code>true</code>, the endpoint is included in the returned map; otherwise, the endpoint
     * is excluded.
     * @return A view of the portion of this map whose keys are strictly greater than <code>fromKey</code>, or
     * greater than or equal to <code>fromKey</code> if <code>inclusive</code> is true.
     **/
    NavigableMap<K, V> tailMap(K fromKey, boolean inclusive);

    /**
     * Returns a view of the portion of this map whose keys range from <code>fromKey</code> to <code>toKey</code>.
     * If <code>fromKey</code> and <code>toKey</code> are equal, the returned map is empty unless
     * <code>fromInclusive</code> and <code>toInclusive</code> are both <code>true</code>.
     *<p>
     * Insertions cannot be made via the returned map and must be made on the main map.
     * <p>
     * <code>remove</code> and <code>fastRemove</code> on the returned map or on an iterator for the returned
     * map are supported, but only if this map does not have an index; otherwise, attempts to remove an entry
     * raise <code>UnsupportedOperationException</code>.
     * <p>
     * The returned map throws an <code>IllegalArgumentException</code> on an attempt to insert a key
     * outside its range.
     *
     * @param fromKey Low endpoint of the keys in the returned map.
     * @param fromInclusive <code>true</code> if the low endpoint is to be included in the returned view;
     * <code>false</code>, otherwise.
     * @param toKey High endpoint of the keys in the returned map.
     * @param toInclusive <code>true</code> if the high endpoint is to be included in the returned view;
     * <code>false</code>, otherwise.
     * @return A view of the portion of this map whose keys range from <code>fromKey</code> to <code>toKey</code>.
     **/
    NavigableMap<K, V> subMap(K fromKey, boolean fromInclusive, K toKey, boolean toInclusive);

    /**
     * Removes and returns a key-value mapping associated with the least key in this map,
     * or <code>null</code> if the map is empty.
     *
     * @return The removed first entry, or <code>null</code> if this map is empty.
     **/
    java.util.Map.Entry<K, V> pollFirstEntry();

    /**
     * Removes and returns a key-value mapping associated with the greatest key in this map,
     * or <code>null</code> if the map is empty.
     *
     * @return The removed last entry, or <code>null</code> if this map is empty.
     **/
    java.util.Map.Entry<K, V> pollLastEntry();
}
