// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.EndpointFactory;
import com.zeroc.Ice.EndpointFactoryWithUnderlying;
import com.zeroc.Ice.InputStream;
import com.zeroc.Ice.ProtocolInstance;

import java.util.ArrayList;

public final class EndpointFactoryI extends EndpointFactoryWithUnderlying {
    public EndpointFactoryI(Instance instance, short type) {
        super(instance, type);
        _instance = instance;
    }

    @Override
    public EndpointFactory cloneWithUnderlying(
            ProtocolInstance instance, short underlying) {
        return new EndpointFactoryI(
                new Instance(_instance.engine(), instance.type(), instance.protocol()), underlying);
    }

    @Override
    public com.zeroc.Ice.EndpointI createWithUnderlying(
            com.zeroc.Ice.EndpointI underlying,
            ArrayList<String> args,
            boolean oaEndpoint) {
        return new EndpointI(_instance, underlying);
    }

    @Override
    public com.zeroc.Ice.EndpointI readWithUnderlying(
            com.zeroc.Ice.EndpointI underlying, InputStream s) {
        return new EndpointI(_instance, underlying);
    }

    private final Instance _instance;
}
