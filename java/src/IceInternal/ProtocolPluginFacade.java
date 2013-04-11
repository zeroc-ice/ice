// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface ProtocolPluginFacade
{
    //
    // Get the Communicator instance with which this facade is
    // associated.
    //
    Ice.Communicator getCommunicator();

    //
    // Get the endpoint host resolver.
    //
    IceInternal.EndpointHostResolver getEndpointHostResolver();

    //
    // Get the protocol support.
    //
    int getProtocolSupport();

    //
    // Get the protocol support.
    //
    boolean getPreferIPv6();

    //
    // Get the network proxy.
    //
    NetworkProxy getNetworkProxy();

    //
    // Get the default encoding to be used in endpoints.
    //
    Ice.EncodingVersion getDefaultEncoding();

    //
    // Get the default hostname to be used in endpoints.
    //
    String getDefaultHost();

    //
    // Get the network trace level and category name.
    //
    int getNetworkTraceLevel();
    String getNetworkTraceCategory();

    //
    // Register an EndpointFactory.
    //
    void addEndpointFactory(EndpointFactory factory);

    //
    // Get an EndpointFactory.
    //
    EndpointFactory getEndpointFactory(short type);

    //
    // Look up a Java class by name.
    //
    Class<?> findClass(String className);
}
