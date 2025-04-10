// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class ProtocolPluginFacadeI implements ProtocolPluginFacade {
    public ProtocolPluginFacadeI(Communicator communicator) {
        _communicator = communicator;
        _instance = communicator.getInstance();
    }

    //
    // Get the Communicator instance with which this facade is
    // associated.
    //
    @Override
    public Communicator getCommunicator() {
        return _communicator;
    }

    //
    // Register an EndpointFactory.
    //
    @Override
    public void addEndpointFactory(EndpointFactory factory) {
        _instance.endpointFactoryManager().add(factory);
    }

    //
    // Register an EndpointFactory.
    //
    @Override
    public EndpointFactory getEndpointFactory(short type) {
        return _instance.endpointFactoryManager().get(type);
    }

    //
    // Look up a Java class by name.
    //
    @Override
    public Class<?> findClass(String className) {
        return _instance.findClass(className);
    }

    private final Instance _instance;
    private final Communicator _communicator;
}
