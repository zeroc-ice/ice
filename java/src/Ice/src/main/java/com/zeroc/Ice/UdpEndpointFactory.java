//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

final class UdpEndpointFactory implements EndpointFactory {
    UdpEndpointFactory(ProtocolInstance instance) {
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
    public EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint) {
        IPEndpointI endpt = new UdpEndpointI(_instance);
        endpt.initWithOptions(args, oaEndpoint);
        return endpt;
    }

    @Override
    public EndpointI read(InputStream s) {
        return new UdpEndpointI(_instance, s);
    }

    @Override
    public EndpointFactory clone(ProtocolInstance instance) {
        return new UdpEndpointFactory(instance);
    }

    private final ProtocolInstance _instance;
}
