//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.Instrumentation;

/**
 * The communicator observer interface used by the Ice run-time to
 * obtain and update observers for its observable objects. This
 * interface should be implemented by add-ins that wish to observe Ice
 * objects in order to collect statistics. An instance of this
 * interface can be provided to the Ice run-time through the Ice
 * communicator initialization data.
 **/
public interface CommunicatorObserver
{
    /**
     * This method should return an observer for the given endpoint
     * information and connector. The Ice run-time calls this method
     * for each connection establishment attempt.
     * @param endpt The endpoint.
     * @param connector The description of the connector. For IP
     * transports, this is typically the IP address to connect to.
     * @return The observer to instrument the connection establishment.
     **/
    Observer getConnectionEstablishmentObserver(com.zeroc.Ice.Endpoint endpt, String connector);

    /**
     * This method should return an observer for the given endpoint
     * information. The Ice run-time calls this method to resolve an
     * endpoint and obtain the list of connectors.
     *
     * For IP endpoints, this typically involves doing a DNS lookup to
     * obtain the IP addresses associated with the DNS name.
     * @param endpt The endpoint.
     * @return The observer to instrument the endpoint lookup.
     **/
    Observer getEndpointLookupObserver(com.zeroc.Ice.Endpoint endpt);

    /**
     * This method should return a connection observer for the given
     * connection. The Ice run-time calls this method for each new
     * connection and for all the Ice communicator connections when
     * {@link ObserverUpdater#updateConnectionObservers} is called.
     * @param c The connection information.
     * @param e The connection endpoint.
     * @param s The state of the connection.
     * @param o The old connection observer if one is already set or a
     * null reference otherwise.
     * @return The connection observer to instrument the connection.
     **/
    ConnectionObserver getConnectionObserver(com.zeroc.Ice.ConnectionInfo c, com.zeroc.Ice.Endpoint e, ConnectionState s, ConnectionObserver o);

    /**
     * This method should return a thread observer for the given
     * thread. The Ice run-time calls this method for each new thread
     * and for all the Ice communicator threads when
     * {@link ObserverUpdater#updateThreadObservers} is called.
     * @param parent The parent of the thread.
     * @param id The ID of the thread to observe.
     * @param s The state of the thread.
     * @param o The old thread observer if one is already set or a
     * null reference otherwise.
     * @return The thread observer to instrument the thread.
     **/
    ThreadObserver getThreadObserver(String parent, String id, ThreadState s, ThreadObserver o);

    /**
     * This method should return an invocation observer for the given
     * invocation. The Ice run-time calls this method for each new
     * invocation on a proxy.
     * @param prx The proxy used for the invocation.
     * @param operation The name of the invocation.
     * @param ctx The context specified by the user.
     * @return The invocation observer to instrument the invocation.
     **/
    InvocationObserver getInvocationObserver(com.zeroc.Ice.ObjectPrx prx, String operation, java.util.Map<java.lang.String, java.lang.String> ctx);

    /**
     * This method should return a dispatch observer for the given
     * dispatch. The Ice run-time calls this method each time it
     * receives an incoming invocation to be dispatched for an Ice
     * object.
     * @param c The current object as provided to the Ice servant
     * dispatching the invocation.
     * @param size The size of the dispatch.
     * @return The dispatch observer to instrument the dispatch.
     **/
    DispatchObserver getDispatchObserver(com.zeroc.Ice.Current c, int size);

    /**
     * The Ice run-time calls this method when the communicator is
     * initialized. The add-in implementing this interface can use
     * this object to get the Ice run-time to re-obtain observers for
     * observed objects.
     * @param updater The observer updater object.
     **/
    void setObserverUpdater(ObserverUpdater updater);
}
