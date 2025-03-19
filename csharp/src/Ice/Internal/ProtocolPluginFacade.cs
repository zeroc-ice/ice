// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

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
}

public sealed class ProtocolPluginFacadeI : ProtocolPluginFacade
{
    public ProtocolPluginFacadeI(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _instance = communicator.instance;
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

    private readonly Instance _instance;
    private readonly Ice.Communicator _communicator;
}
