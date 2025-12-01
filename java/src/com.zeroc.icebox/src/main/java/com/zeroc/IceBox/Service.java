// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBox;

import com.zeroc.Ice.Communicator;

/** An application service managed by a {@link ServiceManager}. */
public interface Service {
    /**
     * Start the service. The given communicator is created by the {@link ServiceManager} for use by
     * the service. This communicator may also be used by other services, depending on the service configuration.
     *
     * <p class="Note">The {@link ServiceManager} owns this communicator, and is responsible for destroying it.
     *
     * @param name the service's name, as determined by the configuration
     * @param communicator a communicator for use by the service
     * @param args the service arguments that were not converted into properties
     * @throws FailureException if {@link #start} failed.
     */
    void start(String name, Communicator communicator, String[] args);

    /** Stop the service. */
    void stop();
}
