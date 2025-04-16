// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

import com.zeroc.Ice.BTEndpointType;
import com.zeroc.Ice.BTSEndpointType;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.ProtocolPluginFacade;
import com.zeroc.Ice.SSLEndpointType;
import com.zeroc.Ice.UnderlyingEndpointFactory;

class PluginI implements Plugin {
    public PluginI(Communicator communicator) {
        final ProtocolPluginFacade facade = new ProtocolPluginFacade(communicator);

        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plug-in is fully initialized.
        Instance bt = new Instance(communicator, BTEndpointType.value, "bt");
        facade.addEndpointFactory(new EndpointFactoryI(bt));

        Instance bts = new Instance(communicator, BTSEndpointType.value, "bts");
        facade.addEndpointFactory(
            new UnderlyingEndpointFactory(bts, SSLEndpointType.value, BTEndpointType.value));
    }

    @Override
    public void initialize() {}

    @Override
    public void destroy() {}
}
