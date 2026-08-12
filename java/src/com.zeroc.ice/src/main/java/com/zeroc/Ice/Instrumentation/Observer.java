// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.Instrumentation;

/**
 * The object observer interface used by instrumented objects to notify the observer of their existence.
 *
 * <p>The Ice runtime calls observers while updating its internal state and does not guard against exceptions they
 * throw. Implementations of this interface and its derived interfaces must not throw exceptions: a thrown exception
 * can leave the Ice runtime in an inconsistent state or terminate an Ice runtime thread.
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
