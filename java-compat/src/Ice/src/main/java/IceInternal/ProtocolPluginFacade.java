// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
