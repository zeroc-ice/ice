//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.Instrumentation;

/**
 * The child invocation observer to instrument remote or collocated
 * invocations.
 **/
public interface ChildInvocationObserver extends Observer
{
    /**
     * Reply notification.
     * @param size The size of the reply.
     **/
    void reply(int size);
}
