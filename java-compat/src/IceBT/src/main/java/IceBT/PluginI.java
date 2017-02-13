// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceBT;

class PluginI implements Ice.Plugin
{
    public PluginI(Ice.Communicator communicator)
    {
        final IceInternal.ProtocolPluginFacade facade = IceInternal.Util.getProtocolPluginFacade(communicator);

        //
        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plug-in is fully initialized.
        //
        EndpointFactoryI factory = new EndpointFactoryI(new Instance(communicator, Ice.BTEndpointType.value, "bt"));
        facade.addEndpointFactory(factory);

        IceInternal.EndpointFactory sslFactory = facade.getEndpointFactory(Ice.SSLEndpointType.value);
        if(sslFactory != null)
        {
            Instance instance = new Instance(communicator, Ice.BTSEndpointType.value, "bts");
            facade.addEndpointFactory(sslFactory.clone(instance, new EndpointFactoryI(instance)));
        }
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
