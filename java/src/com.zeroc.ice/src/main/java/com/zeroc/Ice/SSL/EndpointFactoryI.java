// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

public final class EndpointFactoryI extends com.zeroc.Ice.EndpointFactoryWithUnderlying {
    public EndpointFactoryI(Instance instance, short type) {
        super(instance, type);
        _instance = instance;
    }

    @Override
    public com.zeroc.Ice.EndpointFactory cloneWithUnderlying(
            com.zeroc.Ice.ProtocolInstance instance, short underlying) {
        return new EndpointFactoryI(
                new Instance(_instance.engine(), instance.type(), instance.protocol()), underlying);
    }

    @Override
    public com.zeroc.Ice.EndpointI createWithUnderlying(
            com.zeroc.Ice.EndpointI underlying,
            java.util.ArrayList<String> args,
            boolean oaEndpoint) {
        return new EndpointI(_instance, underlying);
    }

    @Override
    public com.zeroc.Ice.EndpointI readWithUnderlying(
            com.zeroc.Ice.EndpointI underlying, com.zeroc.Ice.InputStream s) {
        return new EndpointI(_instance, underlying);
    }

    private final Instance _instance;
}
