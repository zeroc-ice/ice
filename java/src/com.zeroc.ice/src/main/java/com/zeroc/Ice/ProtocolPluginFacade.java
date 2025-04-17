// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * @hidden Public because it's used by IceBT and the 'Ice/background' test.
 */
public final class ProtocolPluginFacade {
    private final Instance _instance;

    public ProtocolPluginFacade(Communicator communicator) {
        _instance = communicator.getInstance();
    }

    // Registers an EndpointFactory.
    public void addEndpointFactory(EndpointFactory factory) {
        _instance.endpointFactoryManager().add(factory);
    }

    // Gets an EndpointFactory.
    public EndpointFactory getEndpointFactory(short type) {
        return _instance.endpointFactoryManager().get(type);
    }
}
