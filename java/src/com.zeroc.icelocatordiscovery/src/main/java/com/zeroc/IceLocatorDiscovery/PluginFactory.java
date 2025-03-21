// Copyright (c) ZeroC, Inc.

package com.zeroc.IceLocatorDiscovery;

public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    @Override
    public com.zeroc.Ice.Plugin create(
            com.zeroc.Ice.Communicator communicator, String name, String[] args) {
        return new PluginI(communicator);
    }
}
