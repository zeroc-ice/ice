//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.Instrumentation;

/**
 * The object observer interface used by instrumented objects to
 * notify the observer of their existence.
 **/
public interface Observer
{
    /**
     * This method is called when the instrumented object is created
     * or when the observer is attached to an existing object.
     **/
    void attach();

    /**
     * This method is called when the instrumented object is destroyed
     * and as a result the observer detached from the object.
     **/
    void detach();

    /**
     * Notification of a failure.
     * @param exceptionName The name of the exception.
     **/
    void failed(String exceptionName);
}
