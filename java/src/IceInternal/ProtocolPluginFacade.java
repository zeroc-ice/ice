// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
}
