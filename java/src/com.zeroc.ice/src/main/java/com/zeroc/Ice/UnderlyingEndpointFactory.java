// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;

/**
 * The underlying endpoint factory creates endpoints with a factory of the given type. If this
 * factory is of the EndpointFactoryWithUnderlying type, it will delegate to the given underlying
 * factory (this is used by IceIAP/IceBT plugins for the BTS/iAPS endpoint factories).
 *
 * @hidden Public because it's used by IceBT.
 */
public class UnderlyingEndpointFactory implements EndpointFactory {
    public UnderlyingEndpointFactory(ProtocolInstance instance, short type, short underlying) {
        _instance = instance;
        _type = type;
        _underlying = underlying;
    }

    public void initialize() {
        // Get the endpoint factory of the given endpoint type. If it's a factory that
        // delegates to an underlying endpoint, clone it and instruct it to delegate to our underlying factory.
        EndpointFactory factory = _instance.getEndpointFactory(_type);
        if (factory != null && factory instanceof EndpointFactoryWithUnderlying) {
            EndpointFactoryWithUnderlying f = (EndpointFactoryWithUnderlying) factory;
            _factory = f.cloneWithUnderlying(_instance, _underlying);
            _factory.initialize();
        }
    }

    public short type() {
        return _instance.type();
    }

    public String protocol() {
        return _instance.protocol();
    }

    public EndpointI create(ArrayList<String> args, boolean oaEndpoint) {
        if (_factory == null) {
            return null;
        }
        return _factory.create(args, oaEndpoint);
    }

    public EndpointI read(InputStream s) {
        if (_factory == null) {
            return null;
        }
        return _factory.read(s);
    }

    public EndpointFactory clone(ProtocolInstance instance) {
        return new UnderlyingEndpointFactory(instance, _type, _underlying);
    }

    protected final ProtocolInstance _instance;

    private final short _type;
    private final short _underlying;
    private EndpointFactory _factory;
}
