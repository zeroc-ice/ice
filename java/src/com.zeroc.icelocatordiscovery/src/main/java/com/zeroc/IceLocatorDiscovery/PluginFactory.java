// Copyright (c) ZeroC, Inc.

package com.zeroc.IceLocatorDiscovery;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;

public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    @Override
    public String getPluginName() {
        return "IceLocatorDiscovery";
    }

    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginI(communicator);
    }
}
