//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.Instrumentation;

/**
 * The thread observer interface to instrument Ice threads. This can
 * be threads from the Ice thread pool or utility threads used by the
 * Ice core.
 **/
public interface ThreadObserver extends Observer
{
    /**
     * Notification of thread state change.
     * @param oldState The previous thread state.
     * @param newState The new thread state.
     **/
    void stateChanged(ThreadState oldState, ThreadState newState);
}
