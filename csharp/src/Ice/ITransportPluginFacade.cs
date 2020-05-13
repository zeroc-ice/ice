//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

namespace IceInternal
{
    public interface ITransportPluginFacade
    {
        //
        // Get the Communicator instance with which this facade is
        // associated.
        //
        Communicator Communicator { get; }

        //
        // Register an EndpointFactory.
        //
        void AddEndpointFactory(IEndpointFactory factory);

        //
        // Get an EndpointFactory.
        //
        IEndpointFactory? GetEndpointFactory(EndpointType type);

        //
        // Obtain the type for a name.
        //
        System.Type? FindType(string name);
    }

    public sealed class TransportPluginFacade : ITransportPluginFacade
    {
        public TransportPluginFacade(Communicator communicator) => Communicator = communicator;

        //
        // Get the Communicator instance with which this facade is
        // associated.
        //
        public Communicator Communicator { get; private set; }

        //
        // Register an EndpointFactory.
        //
        public void AddEndpointFactory(IEndpointFactory factory) => Communicator.AddEndpointFactory(factory);

        //
        // Get an EndpointFactory.
        //
        public IEndpointFactory? GetEndpointFactory(EndpointType type) => Communicator.FindEndpointFactory(type);

        //
        // Obtain the type for a name.
        //
        public System.Type? FindType(string name) => AssemblyUtil.FindType(name);
    }
}
