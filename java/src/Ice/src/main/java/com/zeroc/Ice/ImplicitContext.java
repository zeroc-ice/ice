//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * An interface to associate implict contexts with communicators.
 *
 * When you make a remote invocation without an explicit context parameter,
 * Ice uses the per-proxy context (if any) combined with the <code>ImplicitContext</code>
 * associated with the communicator.
 *
 * Ice provides several implementations of <code>ImplicitContext</code>. The implementation
 * used depends on the value of the <code>Ice.ImplicitContext</code> property.
 * <dl>
 * <dt><code>None</code> (default)</dt>
 * <dd>No implicit context at all.</dd>
 * <dt><code>PerThread</code></dt>
 * <dd>The implementation maintains a context per thread.</dd>
 * <dt><code>Shared</code></dt>
 * <dd>The implementation maintains a single context shared by all threads.</dd>
 * </dl>
 *
 * <code>ImplicitContext</code> also provides a number of operations to create, update or retrieve
 * an entry in the underlying context without first retrieving a copy of the entire
 * context. These operations correspond to a subset of the <code>java.util.Map</code> methods,
 * with <code>java.lang.Object</code> replaced by <code>string</code> and null replaced by the empty-string.
 **/
public interface ImplicitContext
{
    /**
     * Get a copy of the underlying context.
     * @return A copy of the underlying context.
     **/
    java.util.Map<java.lang.String, java.lang.String> getContext();

    /**
     * Set the underlying context.
     * @param newContext The new context.
     **/
    void setContext(java.util.Map<java.lang.String, java.lang.String> newContext);

    /**
     * Check if this key has an associated value in the underlying context.
     * @param key The key.
     * @return True if the key has an associated value, False otherwise.
     **/
    boolean containsKey(String key);

    /**
     * Get the value associated with the given key in the underlying context.
     * Returns an empty string if no value is associated with the key.
     * {@link #containsKey} allows you to distinguish between an empty-string value and
     * no value at all.
     * @param key The key.
     * @return The value associated with the key.
     **/
    String get(String key);

    /**
     * Create or update a key/value entry in the underlying context.
     * @param key The key.
     * @param value The value.
     * @return The previous value associated with the key, if any.
     **/
    String put(String key, String value);

    /**
     * Remove the entry for the given key in the underlying context.
     * @param key The key.
     * @return The value associated with the key, if any.
     **/
    String remove(String key);
}
