// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

import com.zeroc.Ice.EndpointFactory;
import com.zeroc.Ice.ProtocolInstance;

final class EndpointFactoryI implements EndpointFactory {
    EndpointFactoryI(Instance instance) {
        _instance = instance;
    }

    @Override
    public short type() {
        return _instance.type();
    }

    @Override
    public String protocol() {
        return _instance.protocol();
    }

    @Override
    public com.zeroc.Ice.EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint) {
        EndpointI endpt = new EndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    @Override
    public com.zeroc.Ice.EndpointI read(com.zeroc.Ice.InputStream s) {
        return new EndpointI(_instance, s);
    }

    @Override
    public EndpointFactory clone(ProtocolInstance instance) {
        return new EndpointFactoryI(
                new Instance(_instance.communicator(), instance.type(), instance.protocol()));
    }

    private final Instance _instance;
}
