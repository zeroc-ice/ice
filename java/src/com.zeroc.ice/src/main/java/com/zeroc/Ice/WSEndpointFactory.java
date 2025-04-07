// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;

final class WSEndpointFactory extends EndpointFactoryWithUnderlying {
    public WSEndpointFactory(ProtocolInstance instance, short type) {
        super(instance, type);
    }

    @Override
    public EndpointFactory cloneWithUnderlying(ProtocolInstance instance, short underlying) {
        return new WSEndpointFactory(instance, underlying);
    }

    @Override
    public EndpointI createWithUnderlying(
            EndpointI underlying, ArrayList<String> args, boolean oaEndpoint) {
        return new WSEndpoint(_instance, underlying, args);
    }

    @Override
    public EndpointI readWithUnderlying(EndpointI underlying, InputStream s) {
        return new WSEndpoint(_instance, underlying, s);
    }
}
