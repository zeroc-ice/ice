// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;

/** Creates IceBT plug-ins. */
public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    @Override
    public String getPluginName() {
        return "IceBT";
    }

    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginI(communicator);
    }
}
