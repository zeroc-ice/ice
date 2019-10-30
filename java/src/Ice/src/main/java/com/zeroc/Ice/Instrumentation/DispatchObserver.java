//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.Instrumentation;

/**
 * The dispatch observer to instrument servant dispatch.
 **/
public interface DispatchObserver extends Observer
{
    /**
     * Notification of a user exception.
     **/
    void userException();

    /**
     * Reply notification.
     * @param size The size of the reply.
     **/
    void reply(int size);
}
