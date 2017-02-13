// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
            _instance = Util.getInstance(communicator);
        }

        //
        // Get the Communicator instance with which this facade is
        // associated.
        //
        public Ice.Communicator getCommunicator()
        {
            return _communicator;
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
