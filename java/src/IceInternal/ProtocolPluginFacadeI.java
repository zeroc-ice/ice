// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ProtocolPluginFacadeI implements ProtocolPluginFacade
{
    public
    ProtocolPluginFacadeI(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _instance = Ice.Util.getInstance(communicator);
    }

    //
    // Get the Communicator instance with which this facade is
    // associated.
    //
    public Ice.Communicator
    getCommunicator()
    {
        return _communicator;
    }

    //
    // Get the default hostname to be used in endpoints.
    //
    public String
    getDefaultHost()
    {
        return _instance.defaultsAndOverrides().defaultHost;
    }

    //
    // Get the network trace level and category name.
    //
    public int
    getNetworkTraceLevel()
    {
        return _instance.traceLevels().network;
    }

    public String
    getNetworkTraceCategory()
    {
        return _instance.traceLevels().networkCat;
    }

    //
    // Register an EndpointFactory.
    //
    public void
    addEndpointFactory(EndpointFactory factory)
    {
        _instance.endpointFactoryManager().add(factory);
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
}
