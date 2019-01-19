//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceDiscovery;

/**
 * Creates IceDiscovery plug-ins.
 **/
public class PluginFactory implements com.zeroc.Ice.PluginFactory
{
    @Override
    public com.zeroc.Ice.Plugin create(com.zeroc.Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI(communicator);
    }
}
