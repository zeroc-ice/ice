// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceBT;

import com.zeroc.IceInternal.ProtocolPluginFacade;
import com.zeroc.Ice.BTEndpointType;
import com.zeroc.Ice.BTSEndpointType;
import com.zeroc.Ice.SSLEndpointType;
import com.zeroc.IceInternal.UnderlyingEndpointFactory;

class PluginI implements com.zeroc.Ice.Plugin
{
    public PluginI(com.zeroc.Ice.Communicator communicator)
    {
        final ProtocolPluginFacade f = com.zeroc.IceInternal.Util.getProtocolPluginFacade(communicator);

        //
        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plug-in is fully initialized.
        //
        Instance bt = new Instance(communicator, BTEndpointType.value, "bt");
        f.addEndpointFactory(new EndpointFactoryI(bt));

        Instance bts = new Instance(communicator, BTSEndpointType.value, "bts");
        f.addEndpointFactory(new UnderlyingEndpointFactory(bts, SSLEndpointType.value, BTEndpointType.value));
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
