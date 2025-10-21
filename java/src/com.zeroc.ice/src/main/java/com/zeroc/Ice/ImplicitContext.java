// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.Map;

/**
 * Represents the request context associated with a communicator.
 * When you make a remote invocation without an explicit request context parameter, Ice uses the per-proxy request
 * context (if any) combined with the {@code ImplicitContext} associated with your communicator.
 * 
 * <p>The property {@code Ice.ImplicitContext} controls if your communicator has an associated implicit context,
 * and when it does, whether this implicit context is per-thread or shared by all threads:
 * <dl>
 *   <dt>{@code None} (default)
 *   <dd>No implicit context at all.
 *   <dt>{@code PerThread}
 *   <dd>The implementation maintains a context per thread.
 *   <dt>{@code Shared}
 *   <dd>The implementation maintains a single context shared by all threads.
 * </dl>
 */
public interface ImplicitContext {
    /**
     * Gets a copy of the request context maintained by this object.
     *
     * @return a copy of the request context
     */
    Map<String, String> getContext();

    /**
     * Sets the request context.
     *
     * @param newContext the new request context
     */
    void setContext(Map<String, String> newContext);

    /**
     * Checks if the specified key has an associated value in the request context.
     *
     * @param key the key
     * @return {@code true} if the key has an associated value, {@code false} otherwise
     */
    boolean containsKey(String key);

    /**
     * Gets the value associated with the specified key in the request context.
     *
     * @param key the key
     * @return the value associated with the key, or the empty string if no value is associated with the key.
     *     {@link #containsKey} allows you to distinguish between an empty-string value and no value at all.
     */
    String get(String key);

    /**
     * Creates or updates a key/value entry in the request context.
     *
     * @param key the key
     * @param value the value
     * @return the previous value associated with the key, if any
     */
    String put(String key, String value);

    /**
     * Removes the entry for the specified key in the request context.
     *
     * @param key the key
     * @return the value associated with the key, if any
     */
    String remove(String key);
}
