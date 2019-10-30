//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceBox;

/**
 * An application service managed by a {@link ServiceManager}.
 **/
public interface Service
{
    /**
     * Start the service. The given communicator is created by the
     * {@link ServiceManager} for use by the service. This communicator may
     * also be used by other services, depending on the service
     * configuration.
     *
     * <p class="Note">The {@link ServiceManager} owns this communicator, and is
     * responsible for destroying it.
     * @param name The service's name, as determined by the
     * configuration.
     * @param communicator A communicator for use by the service.
     * @param args The service arguments that were not converted into
     * properties.
     * @throws FailureException Raised if {@link #start} failed.
     **/
    void start(String name, com.zeroc.Ice.Communicator communicator, String[] args);

    /**
     * Stop the service.
     **/
    void stop();
}
