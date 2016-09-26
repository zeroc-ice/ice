// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Ice/EndpointF.ice>
#include <Ice/ConnectionF.ice>
#include <Ice/Current.ice>

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * The Instrumentation local interfaces enable observing a number of
 * Ice core internal components (threads, connections, etc).
 *
 **/
["objc:prefix:ICEINSTRUMENTATION"]
module Instrumentation
{

/**
 *
 * The object observer interface used by instrumented objects to
 * notify the observer of their existence.
 *
 **/
local interface Observer
{
    /**
     *
     * This method is called when the instrumented object is created
     * or when the observer is attached to an existing object.
     *
     **/
    void attach();

    /**
     *
     * This method is called when the instrumented object is destroyed
     * and as a result the observer detached from the object.
     *
     **/
    void detach();

    /**
     *
     * Notification of a failure.
     *
     * @param exceptionName The name of the exception.
     *
     **/
    void failed(string exceptionName);
};

/**
 *
 * The thread state enumeration keeps track of the different possible
 * states of Ice threads.
 *
 **/
local enum ThreadState
{
    /**
     *
     * The thread is idle.
     *
     **/
    ThreadStateIdle,

    /**
     *
     * The thread is in use performing reads or writes for Ice
     * connections. This state is only for threads from an Ice thread
     * pool.
     *
     **/
    ThreadStateInUseForIO,

    /**
     *
     * The thread is calling user code (servant implementation, AMI
     * callbacks). This state is only for threads from an Ice thread
     * pool.
     *
     **/
    ThreadStateInUseForUser,

    /**
     *
     * The thread is performing other internal activities (DNS
     * lookups, timer callbacks, etc).
     *
     **/
    ThreadStateInUseForOther,
};

/**
 *
 * The thread observer interface to instrument Ice threads. This can
 * be threads from the Ice thread pool or utility threads used by the
 * Ice core.
 *
 **/
local interface ThreadObserver extends Observer
{
    /**
     *
     * Notification of thread state change.
     *
     * @param oldState The previous thread state.
     *
     * @param newState The new thread state.
     *
     **/
    void stateChanged(ThreadState oldState, ThreadState newState);
};

/**
 *
 * The state of an Ice connection.
 *
 **/
local enum ConnectionState
{
    /**
     *
     * The connection is being validated.
     *
     **/
    ConnectionStateValidating,

    /**
     *
     * The connection is holding the reception of new messages.
     *
     **/
    ConnectionStateHolding,

    /**
     *
     * The connection is active and can send and receive messages.
     *
     **/
    ConnectionStateActive,

    /**
     *
     * The connection is being gracefully shutdown and waits for the
     * peer to close its end of the connection.
     *
     **/
    ConnectionStateClosing,

    /**
     *
     * The connection is closed and waits for potential dispatch to be
     * finished before being destroyed and detached from the observer.
     *
     **/
    ConnectionStateClosed
};


/**
 *
 * The connection observer interface to instrument Ice connections.
 *
 **/
local interface ConnectionObserver extends Observer
{
    /**
     *
     * Notification of sent bytes over the connection.
     *
     * @param num The number of bytes sent.
     *
     **/
    void sentBytes(int num);

    /**
     *
     * Notification of received bytes over the connection.
     *
     * @param num The number of bytes received.
     *
     **/
    void receivedBytes(int num);
};

/**
 *
 * The dispatch observer to instrument servant dispatch.
 *
 **/
local interface DispatchObserver extends Observer
{
    /**
     *
     * Notification of a user exception.
     *
     **/
    void userException();

    /**
     *
     * Reply notification.
     *
     * @param size The size of the reply.
     *
     **/
    void reply(int size);
};

/**
 *
 * The child invocation observer to instrument remote or collocated
 * invocations.
 *
 **/
local interface ChildInvocationObserver extends Observer
{
    /**
     *
     * Reply notification.
     *
     * @param size The size of the reply.
     *
     **/
    void reply(int size);
};


/**
 *
 * The remote observer to instrument invocations that are sent over
 * the wire.
 *
 **/
local interface RemoteObserver extends ChildInvocationObserver
{
};

/**
 *
 * The collocated observer to instrument invocations that are
 * collocated.
 *
 **/
local interface CollocatedObserver extends ChildInvocationObserver
{
};

/**
 *
 * The invocation observer to instrument invocations on proxies. A
 * proxy invocation can either result in a collocated or remote
 * invocation. If it results in a remote invocation, a sub-observer is
 * requested for the remote invocation.
 *
 **/
local interface InvocationObserver extends Observer
{
    /**
     *
     * Notification of the invocation being retried.
     *
     **/
    void retried();

    /**
     *
     * Notification of a user exception.
     *
     **/
    void userException();

    /**
     *
     * Get a remote observer for this invocation.
     *
     * @param con The connection information.
     *
     * @param endpt The connection endpoint.
     *
     * @param requestId The ID of the invocation.
     *
     * @param size The size of the invocation.
     *
     * @return The observer to instrument the remote invocation.
     *
     **/
    RemoteObserver getRemoteObserver(ConnectionInfo con, Endpoint endpt, int requestId, int size);

    /**
     *
     * Get a collocated observer for this invocation.
     *
     * @param adapter The object adapter hosting the collocated Ice object.
     *
     * @param requestId The ID of the invocation.
     *
     * @param size The size of the invocation.
     *
     * @return The observer to instrument the collocated invocation.
     *
     **/
    CollocatedObserver getCollocatedObserver(ObjectAdapter adapter, int requestId, int size);
};

/**
 *
 * The observer updater interface. This interface is implemented by
 * the Ice run-time and an instance of this interface is provided by
 * the Ice communicator on initialization to the {@link
 * CommunicatorObserver} object set with the communicator
 * initialization data. The Ice communicator calls {@link
 * CommunicatorObserver#setObserverUpdater} to provide the observer
 * updater.
 *
 * This interface can be used by add-ins implementing the {@link
 * CommunicatorObserver} interface to update the observers of
 * connections and threads.
 *
 **/
local interface ObserverUpdater
{
    /**
     *
     * Update connection observers associated with each of the Ice
     * connection from the communicator and its object adapters.
     *
     * When called, this method goes through all the connections and
     * for each connection {@link CommunicatorObserver#getConnectionObserver}
     * is called. The implementation of getConnectionObserver has the
     * possibility to return an updated observer if necessary.
     *
     **/
    void updateConnectionObservers();

    /**
     *
     * Update thread observers associated with each of the Ice thread
     * from the communicator and its object adapters.
     *
     * When called, this method goes through all the threads and for
     * each thread {@link CommunicatorObserver#getThreadObserver} is
     * called. The implementation of getThreadObserver has the
     * possibility to return an updated observer if necessary.
     *
     **/
    void updateThreadObservers();
};

/**
 *
 * The communicator observer interface used by the Ice run-time to
 * obtain and update observers for its observable objects. This
 * interface should be implemented by add-ins that wish to observe Ice
 * objects in order to collect statistics. An instance of this
 * interface can be provided to the Ice run-time through the Ice
 * communicator initialization data.
 *
 **/
local interface CommunicatorObserver
{
    /**
     *
     * This method should return an observer for the given endpoint
     * information and connector. The Ice run-time calls this method
     * for each connection establishment attempt.
     *
     * @param endpt The endpoint.
     *
     * @param connector The description of the connector. For IP
     * transports, this is typically the IP address to connect to.
     *
     * @return The observer to instrument the connection establishment.
     *
     **/
    Observer getConnectionEstablishmentObserver(Endpoint endpt, string connector);

    /**
     *
     * This method should return an observer for the given endpoint
     * information. The Ice run-time calls this method to resolve an
     * endpoint and obtain the list of connectors.
     *
     * For IP endpoints, this typically involves doing a DNS lookup to
     * obtain the IP addresses associated with the DNS name.
     *
     * @param endpt The endpoint.
     *
     * @return The observer to instrument the endpoint lookup.
     *
     **/
    Observer getEndpointLookupObserver(Endpoint endpt);

    /**
     *
     * This method should return a connection observer for the given
     * connection. The Ice run-time calls this method for each new
     * connection and for all the Ice communicator connections when
     * {@link ObserverUpdater#updateConnectionObservers} is called.
     *
     * @param c The connection information.
     *
     * @param e The connection endpoint.
     *
     * @param s The state of the connection.
     *
     * @param o The old connection observer if one is already set or a
     * null reference otherwise.
     *
     * @return The connection observer to instrument the connection.
     *
     **/
    ConnectionObserver getConnectionObserver(ConnectionInfo c, Endpoint e, ConnectionState s, ConnectionObserver o);

    /**
     *
     * This method should return a thread observer for the given
     * thread. The Ice run-time calls this method for each new thread
     * and for all the Ice communicator threads when
     * {@link ObserverUpdater#updateThreadObservers} is called.
     *
     * @param parent The parent of the thread.
     *
     * @param id The ID of the thread to observe.
     *
     * @param s The state of the thread.
     *
     * @param o The old thread observer if one is already set or a
     * null reference otherwise.
     *
     * @return The thread observer to instrument the thread.
     *
     **/
    ThreadObserver getThreadObserver(string parent, string id, ThreadState s, ThreadObserver o);

    /**
     *
     * This method should return an invocation observer for the given
     * invocation. The Ice run-time calls this method for each new
     * invocation on a proxy.
     *
     * @param prx The proxy used for the invocation.
     *
     * @param operation The name of the invocation.
     *
     * @param ctx The context specified by the user.
     *
     * @return The invocation observer to instrument the invocation.
     *
     **/
    InvocationObserver getInvocationObserver(Object* prx, string operation, Context ctx);

    /**
     *
     * This method should return a dispatch observer for the given
     * dispatch. The Ice run-time calls this method each time it
     * receives an incoming invocation to be dispatched for an Ice
     * object.
     *
     * @param c The current object as provided to the Ice servant
     * dispatching the invocation.
     *
     * @param size The size of the dispatch.
     *
     * @return The dispatch observer to instrument the dispatch.
     *
     **/
    DispatchObserver getDispatchObserver(Current c, int size);

    /**
     *
     * The Ice run-time calls this method when the communicator is
     * initialized. The add-in implementing this interface can use
     * this object to get the Ice run-time to re-obtain observers for
     * observed objects.
     *
     * @param updater The observer updater object.
     *
     **/
    void setObserverUpdater(ObserverUpdater updater);
};

};

};

