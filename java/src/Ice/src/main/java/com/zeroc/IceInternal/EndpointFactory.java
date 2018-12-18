// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

public interface EndpointFactory
{
    default void initialize()
    {
        // Nothing to do, can be overriden by specialization to finish initialization.
    }

    short type();
    String protocol();
    EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint);
    EndpointI read(com.zeroc.Ice.InputStream s);
    void destroy();

    EndpointFactory clone(ProtocolInstance instance);
}
