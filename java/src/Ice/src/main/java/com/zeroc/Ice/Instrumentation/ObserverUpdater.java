//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.Instrumentation;

/**
 * The observer updater interface. This interface is implemented by
 * the Ice run-time and an instance of this interface is provided by
 * the Ice communicator on initialization to the
 * {@link CommunicatorObserver} object set with the communicator
 * initialization data. The Ice communicator calls
 * {@link CommunicatorObserver#setObserverUpdater} to provide the observer
 * updater.
 *
 * This interface can be used by add-ins implementing the
 * {@link CommunicatorObserver} interface to update the observers of
 * connections and threads.
 **/
public interface ObserverUpdater
{
    /**
     * Update connection observers associated with each of the Ice
     * connection from the communicator and its object adapters.
     *
     * When called, this method goes through all the connections and
     * for each connection {@link CommunicatorObserver#getConnectionObserver}
     * is called. The implementation of getConnectionObserver has the
     * possibility to return an updated observer if necessary.
     **/
    void updateConnectionObservers();

    /**
     * Update thread observers associated with each of the Ice thread
     * from the communicator and its object adapters.
     *
     * When called, this method goes through all the threads and for
     * each thread {@link CommunicatorObserver#getThreadObserver} is
     * called. The implementation of getThreadObserver has the
     * possibility to return an updated observer if necessary.
     **/
    void updateThreadObservers();
}
