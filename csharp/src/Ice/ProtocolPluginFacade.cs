//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
            _communicator.endpointFactoryManager().add(factory);
        }

        //
        // Get an EndpointFactory.
        //
        public EndpointFactory getEndpointFactory(short type)
        {
            return _communicator.endpointFactoryManager().get(type);
        }

        //
        // Obtain the type for a name.
        //
        public System.Type findType(string name)
        {
            return AssemblyUtil.findType(name);
        }

        private Ice.Communicator _communicator;
    }
}
