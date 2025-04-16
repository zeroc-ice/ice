// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

/// <summary>Provides limited access to the internal endpoint factory manager. Used by transport plug-ins.</summary>
public sealed class ProtocolPluginFacade
{
    private readonly Instance _instance;

    public ProtocolPluginFacade(Communicator communicator) => _instance = communicator.instance;

    // Registers an EndpointFactory.
    public void addEndpointFactory(EndpointFactory factory) => _instance.endpointFactoryManager().add(factory);

    // Gets an EndpointFactory.
    public EndpointFactory getEndpointFactory(short type) => _instance.endpointFactoryManager().get(type);
}
