// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceWS
{
    /// <summary>
    /// Plug-in factories must implement this interface.
    /// </summary>
    public sealed class PluginFactory : Ice.PluginFactory
    {
        /// <summary>
        /// Returns a new plug-in.
        /// </summary>
        /// <param name="communicator">The communicator for the plug-in.</param>
        /// <param name="name">The name of the plug-in.</param>
        /// <param name="args">The arguments that are specified in the plug-in's configuration.</param>
        ///
        /// <returns>The new plug-in. null can be returned to indicate
        /// that a general error occurred. Alternatively, create can throw
        /// PluginInitializationException to provide more detailed information.</returns>
        public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
        {
            return new PluginI(communicator);
        }
    }

    public sealed class PluginI : Ice.Plugin
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
}
