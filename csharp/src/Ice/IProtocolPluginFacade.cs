//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    public interface IProtocolPluginFacade
    {
        //
        // Get the Communicator instance with which this facade is
        // associated.
        //
        Ice.Communicator getCommunicator();

        //
        // Register an EndpointFactory.
        //
        void addEndpointFactory(IEndpointFactory factory);

        //
        // Get an EndpointFactory.
        //
        IEndpointFactory getEndpointFactory(short type);

        //
        // Obtain the type for a name.
        //
        System.Type? findType(string name);
    }

    public sealed class ProtocolPluginFacade : IProtocolPluginFacade
    {
        public ProtocolPluginFacade(Ice.Communicator communicator) => _communicator = communicator;

        //
        // Get the Communicator instance with which this facade is
        // associated.
        //
        public Ice.Communicator getCommunicator() => _communicator;

        //
        // Register an EndpointFactory.
        //
        public void addEndpointFactory(IEndpointFactory factory) => _communicator.EndpointFactoryManager().add(factory);

        //
        // Get an EndpointFactory.
        //
        public IEndpointFactory getEndpointFactory(short type) => _communicator.EndpointFactoryManager().get(type);

        //
        // Obtain the type for a name.
        //
        public System.Type? findType(string name) => AssemblyUtil.findType(name);

        private Ice.Communicator _communicator;
    }
}
