// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

public interface ProtocolPluginFacade
{
    //
    // Get the Communicator instance with which this facade is
    // associated.
    //
    com.zeroc.Ice.Communicator getCommunicator();

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
