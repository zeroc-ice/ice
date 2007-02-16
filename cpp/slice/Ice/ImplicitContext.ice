// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_IMPLICIT_CONTEXT_ICE
#define ICE_IMPLICIT_CONTEXT_ICE

#include <Ice/LocalException.ice>
#include <Ice/Current.ice>

module Ice
{

/**
 *
 * An interface to associate implict contexts with communicators.
 *
 * When you make a remote invocation without an explicit [Context] parameter,
 * Ice uses the per-proxy [Context] (if any) combined with the <tt>ImplicitContext</tt>
 * associated with the communicator.</p>
 * <p>Ice provides several implementations of <tt>ImplicitContext</tt>. The implementation
 * used depends on the value of the <tt>Ice.ImplicitContext</tt> property.
 * <dl>
 * <dt><tt>None</tt> (default)</dt>
 * <dd>No implicit context at all.</dd>
 * <dt><tt>PerThread</tt></dt>
 * <dd>The implementation maintains a [Context] per thread.</dd>
 * <dt><tt>Shared</tt></dt>
 * <dd>The implementation maintains a single [Context] shared 
 * by all threads.</dd>
 * </dl><p>
 *  
 * <tt>ImplicitContext</tt> also provides a number of operations to create, update or retrieve
 * an entry in the underlying context without first retrieving a copy of the entire
 * context. These operations correspond to a subset of the <tt>java.util.Map</tt> methods,
 * with <tt>java.lang.Object</tt> replaced by <tt>string</tt> and null replaced by the empty-string.
 * 
 **/

local interface ImplicitContext
{
    /**
     * Get a copy of the underlying context.
     * @return A copy of the underlying context.
     *
     **/
    ["cpp:const"] Context getContext();
    
    /**
     * Set the underlying context.
     *
     * @param newContext The new context.
     * 
     **/
    void setContext(Context newContext);

    /**
     * Check if this key has an associated value in the underlying context. 
     *
     * @param key The key.
     *
     * @return True if the key has an associated value, False otherwise.
     *
     **/
    ["cpp:const"] bool containsKey(string key);
 
    /**
     * Get the value associated with the given key in the underlying context.
     * Returns an empty string if no value is associated with the key.
     * [containsKey] allows you to distinguish between an empty-string value and 
     * no value at all.
     *
     * @param key The key.
     *
     * @return The value associated with the key.
     *
     **/
    ["cpp:const"] string get(string key);

    /**
     * Create or update a key/value entry in the underlying context.
     *
     * @param key The key.
     *
     * @param value The value.
     *
     * @return The previous value associated with the key, if any.
     *
     **/
    string put(string key, string value);

    /**
     * Remove the entry for the given key in the underlying context.
     *
     * @param key The key.
     *
     * @return The value associated with the key, if any.
     *
     **/
    string remove(string key);
};
};

#endif
