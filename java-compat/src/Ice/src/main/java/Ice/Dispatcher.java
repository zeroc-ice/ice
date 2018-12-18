// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * You can control which thread receives operation invocations and AMI
 * callbacks by implementing the <code>Dispatcher</code> interface and
 * supplying an instance in <code>InitializationData</code> when
 * initializing a communicator.
 * <p>
 * For example, you can use this dispatching facility to ensure that
 * all invocations and callbacks are dispatched in a GUI event loop
 * thread so that it is safe to invoke directly on GUI objects.
 **/
public interface Dispatcher
{
    /**
     * Responsible for dispatching an invocation or AMI callback.
     * The method must eventually invoke <code>run</code> on the
     * supplied <code>Runnable</code> object.
     *
     * @param runnable The object encapsulating the invocation or
     * callback to be dispatched.
     * @param con The connection associated with the dispatch.
     **/
    void dispatch(Runnable runnable, Ice.Connection con);
}
