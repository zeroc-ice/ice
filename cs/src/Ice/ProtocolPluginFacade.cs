// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    public interface ProtocolPluginFacade
    {
        //
        // Get the Communicator instance with which this facade is
        // associated.
        //
        Ice.Communicator getCommunicator();

#if !SILVERLIGHT
        //
        // Get the endpoint host resolver.
        //
        IceInternal.EndpointHostResolver getEndpointHostResolver();
#endif
        //
        // Get the protocol support.
        //
        int getProtocolSupport();

        //
        // Get the protocol support.
        //
        bool getPreferIPv6();

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
        string getDefaultHost();

        //
        // Get the network trace level and category name.
        //
        int getNetworkTraceLevel();
        string getNetworkTraceCategory();

        //
        // Register an EndpointFactory.
        //
        void addEndpointFactory(EndpointFactory factory);

        //
        // Get an EndpointFactory.
        //
        EndpointFactory getEndpointFactory(short type);

        //
        // Obtain the type for a name.
        //
        System.Type findType(string name);
    }

    public sealed class ProtocolPluginFacadeI : ProtocolPluginFacade
    {
        public ProtocolPluginFacadeI(Ice.Communicator communicator)
        {
            _communicator = communicator;
            _instance = IceInternal.Util.getInstance(communicator);
        }

        //
        // Get the Communicator instance with which this facade is
        // associated.
        //
        public Ice.Communicator getCommunicator()
        {
            return _communicator;
        }

#if !SILVERLIGHT
        //
        // Get the endpoint host resolver.
        //
        public IceInternal.EndpointHostResolver getEndpointHostResolver()
        {
            return _instance.endpointHostResolver();
        }
#endif

        //
        // Get the protocol support.
        //
        public int getProtocolSupport()
        {
            return _instance.protocolSupport();
        }

        //
        // Get the protocol support.
        //
        public bool getPreferIPv6()
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
        // Get the default hostname to be used in endpoints.
        //
        public Ice.EncodingVersion getDefaultEncoding()
        {
            return _instance.defaultsAndOverrides().defaultEncoding;
        }

        //
        // Get the default hostname to be used in endpoints.
        //
        public string getDefaultHost()
        {
            return _instance.defaultsAndOverrides().defaultHost;
        }

        //
        // Get the network trace level and category name.
        //
        public int getNetworkTraceLevel()
        {
            return _instance.traceLevels().network;
        }

        public string getNetworkTraceCategory()
        {
            return _instance.traceLevels().networkCat;
        }

        //
        // Register an EndpointFactory.
        //
        public void addEndpointFactory(EndpointFactory factory)
        {
            _instance.endpointFactoryManager().add(factory);
        }

        //
        // Get an EndpointFactory.
        //
        public EndpointFactory getEndpointFactory(short type)
        {
            return _instance.endpointFactoryManager().get(type);
        }

        //
        // Obtain the type for a name.
        //
        public System.Type findType(string name)
        {
            return AssemblyUtil.findType(_instance, name);
        }

        private Instance _instance;
        private Ice.Communicator _communicator;
    }
}
