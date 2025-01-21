// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * @hidden Public because it's used by IceBT and the 'Ice/background' test.
 */
public interface ProtocolPluginFacade {
    //
    // Get the Communicator instance with which this facade is
    // associated.
    //
    Communicator getCommunicator();

    //
    // Register an EndpointFactory.
    //
    void addEndpointFactory(EndpointFactory factory);

    //
    // Get an EndpointFactory.
    //
    EndpointFactory getEndpointFactory(short type);

    //
    // Look up a Java class by name.
    //
    Class<?> findClass(String className);
}
