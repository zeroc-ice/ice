// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceWS;

class PluginI implements Ice.Plugin
{
    public PluginI(Ice.Communicator communicator)
    {
        IceInternal.ProtocolPluginFacade facade = IceInternal.Util.getProtocolPluginFacade(communicator);
        IceInternal.EndpointFactory tcpFactory = facade.getEndpointFactory(Ice.TCPEndpointType.value);
        if(tcpFactory != null)
        {
            Instance tcpInstance = new Instance(communicator, WSEndpointType.value, "ws");
            facade.addEndpointFactory(new EndpointFactoryI(tcpInstance, tcpFactory.clone(tcpInstance)));
        }
        IceInternal.EndpointFactory sslFactory = facade.getEndpointFactory((short)2); // 2 = SSLEndpointType
        if(sslFactory != null)
        {
            Instance sslInstance = new Instance(communicator, WSSEndpointType.value, "wss");
            facade.addEndpointFactory(new EndpointFactoryI(sslInstance, sslFactory.clone(sslInstance)));
        }
    }

    public void initialize()
    {
    }

    public void destroy()
    {
    }
}
