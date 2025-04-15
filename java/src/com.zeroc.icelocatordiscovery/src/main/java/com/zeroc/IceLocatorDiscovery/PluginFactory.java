// Copyright (c) ZeroC, Inc.

package com.zeroc.IceLocatorDiscovery;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginInitializationException;

public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    @Override
    public String getPluginName() {
        return "IceLocatorDiscovery";
    }

    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {

        if (name == null || !name.equals(getPluginName())) {
            throw new PluginInitializationException(
                "The Locator Discovery plug-in must be named '" + getPluginName() + "'.");
        }

        return new PluginI(communicator);
    }
}
