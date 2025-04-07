// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;

final class TcpEndpointFactory implements EndpointFactory {
    TcpEndpointFactory(ProtocolInstance instance) {
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
    public EndpointI create(ArrayList<String> args, boolean oaEndpoint) {
        IPEndpointI endpt = new TcpEndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    @Override
    public EndpointI read(InputStream s) {
        return new TcpEndpointI(_instance, s);
    }

    @Override
    public EndpointFactory clone(ProtocolInstance instance) {
        return new TcpEndpointFactory(instance);
    }

    private final ProtocolInstance _instance;
}
