//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;

internal class Plugin : IPlugin
{
    internal Plugin(Communicator communicator) => _communicator = communicator;

    public void Initialize()
    {
        ITransportPluginFacade facade = Util.GetTransportPluginFacade(_communicator);
        for (short s = 0; s < 100; ++s)
        {
            IEndpointFactory? factory = facade.GetEndpointFactory((EndpointType)s);
            if (factory != null)
            {
                facade.AddEndpointFactory(new EndpointFactory(factory));
            }
        }
    }

    public void Destroy()
    {
    }

    private readonly Communicator _communicator;
}
