// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
 * When you make a remote invocation without an explicit [Context] parameter,
 * Ice uses the per-proxy [Context] (if any) combined with the [ImplicitContext]
 * associated with the communicator.</dd>
 * <dd>Ice provides several implementations of [ImplicitContext]. The implementation
 * used depends on the value of the Ice.ImplicitContext property.
 * <dl>
 * <dt>None (default)</dt>
 * <dd>No implicit context at all.</dd>
 * <dt>PerThread</dt>
 * <dd>The implementation maintains a [Context] per thread.</dd>
 * <dt>Shared</dt>
 * <dd>The implementation maintains a single [Context] shared 
 * by all threads, and serializes access to this [Context].</dd>
 * <dt>SharedWithoutLocking</dt>
 * <dd>The implementation maintains a single 
 * [Context] shared by all threads, and does not serialize access to this [Context].</dd>
 * </dt>
 * </dl>
 * </dd>
 *
 **/

local interface ImplicitContext
{
    /**
     * Get the underlying context.
     *
     * @return The underlying context.
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
     * Get the value associated with the given key in the underlying context.
     * Throws [NotSetException] when no value is associated with the given key.
     *
     * @param key The key.
     *
     * @return The value associated with the key.
     *
     **/
    ["cpp:const"] string get(string key);

    /**
     * Get the value associated with the given key in the underlying context.
     *
     * @param key The key.
     *
     * @param defaultValue The default value
     *
     * @return The value associated with the key, or defaultValue when no
     * value is associated with the given key.
     *
     **/
    ["cpp:const"] string getWithDefault(string key, string defaultValue);

    /**
     * Set the value associated with the given key in the underlying context.
     *
     * @param key The key.
     *
     * @param value The value.
     *
     **/
    void set(string key, string value);

    /**
     * Remove the value associated with the given key in the underlying context
     * Throws [NotSetException] when no value is associated with the given key.
     *
     * @param key The key.
     *
     **/
    void remove(string key);
};
};

#endif
