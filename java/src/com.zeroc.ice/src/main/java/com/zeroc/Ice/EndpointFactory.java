// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * @hidden Public because it's used by IceBT, SSL, and the 'Ice/background' test.
 */
public interface EndpointFactory {
    default void initialize() {
        // Nothing to do, can be overridden by specialization to finish initialization.
    }

    short type();

    String protocol();

    EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint);

    EndpointI read(InputStream s);

    EndpointFactory clone(ProtocolInstance instance);
}
