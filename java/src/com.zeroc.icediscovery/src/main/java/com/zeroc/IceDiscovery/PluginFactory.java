// Copyright (c) ZeroC, Inc.

package com.zeroc.IceDiscovery;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;

/** Creates IceDiscovery plug-ins. */
public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginI(communicator);
    }
}
