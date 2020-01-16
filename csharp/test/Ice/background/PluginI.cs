//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class Plugin : Ice.IPlugin
{
    internal Plugin(Ice.Communicator communicator) => _communicator = communicator;

    public void initialize()
    {
        IceInternal.IProtocolPluginFacade facade = IceInternal.Util.getProtocolPluginFacade(_communicator);
        for (short s = 0; s < 100; ++s)
        {
            IceInternal.IEndpointFactory factory = facade.getEndpointFactory(s);
            if (factory != null)
            {
                facade.addEndpointFactory(new EndpointFactory(factory));
            }
        }
    }

    public void destroy()
    {
    }

    private Ice.Communicator _communicator;
}
