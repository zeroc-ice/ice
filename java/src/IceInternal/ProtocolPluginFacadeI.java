// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        _instance = Util.getInstance(communicator);
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
    // Get the endpoint host resolver.
    //
    public EndpointHostResolver
    getEndpointHostResolver()
    {
        return _instance.endpointHostResolver();
    }    

    //
    // Get the protocol support.
    //
    public int
    getProtocolSupport()
    {
        return _instance.protocolSupport();
    }    

    public boolean
    getPreferIPv6()
    {
        return _instance.preferIPv6();
    }

    //
    // Get the network proxy.
    //
    public NetworkProxy getNetworkProxy()
    {
        return _instance.networkProxy();
    }

    //
    // Get the default encoding to be used in endpoints.
    //
    public Ice.EncodingVersion
    getDefaultEncoding()
    {
        return _instance.defaultsAndOverrides().defaultEncoding;
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

    //
    // Register an EndpointFactory.
    //
    public EndpointFactory
    getEndpointFactory(short type)
    {
        return _instance.endpointFactoryManager().get(type);
    }

    //
    // Look up a Java class by name.
    //
    public Class<?>
    findClass(String className)
    {
        return _instance.findClass(className);
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
}
