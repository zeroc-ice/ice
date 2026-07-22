// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.Instrumentation;

/**
 * The object observer interface used by instrumented objects to notify the observer of their existence.
 */
public interface Observer {
    /**
     * Notifies the observer that an instrumented object was created.
     */
    void attach();

    /**
     * Notifies the observer that an instrumented object was destroyed.
     */
    void detach();

    /**
     * Notifies the observer of a failure.
     *
     * @param exceptionName The name of the exception.
     */
    void failed(String exceptionName);
}
