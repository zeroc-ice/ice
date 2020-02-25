//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class Plugin : Ice.IPlugin
{
    internal Plugin(Ice.Communicator communicator) => _communicator = communicator;

    public void Initialize()
    {
        IceInternal.ITransportPluginFacade facade = IceInternal.Util.GetTransportPluginFacade(_communicator);
        for (short s = 0; s < 100; ++s)
        {
            IceInternal.IEndpointFactory? factory = facade.GetEndpointFactory(s);
            if (factory != null)
            {
                facade.AddEndpointFactory(new EndpointFactory(factory));
            }
        }
    }

    public void Destroy()
    {
    }

    private Ice.Communicator _communicator;
}
