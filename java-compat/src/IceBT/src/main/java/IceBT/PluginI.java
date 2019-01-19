//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceBT;

import IceInternal.UnderlyingEndpointFactory;

class PluginI implements Ice.Plugin
{
    public PluginI(Ice.Communicator communicator)
    {
        final IceInternal.ProtocolPluginFacade f = IceInternal.Util.getProtocolPluginFacade(communicator);

        //
        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plug-in is fully initialized.
        //
        Instance bt = new Instance(communicator, Ice.BTEndpointType.value, "bt");
        f.addEndpointFactory(new EndpointFactoryI(bt));

        Instance bts = new Instance(communicator, Ice.BTSEndpointType.value, "bts");
        f.addEndpointFactory(new UnderlyingEndpointFactory(bts, Ice.SSLEndpointType.value, Ice.BTEndpointType.value));
    }

    @Override
    public void initialize()
    {
    }

    @Override
    public void destroy()
    {
    }
}
