// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginInitializationException;

/** Creates IceBT plug-ins. */
public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    @Override
    public String getPluginName() {
        return "IceBT";
    }

    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {

        if (name == null || !name.equals(getPluginName())) {
            throw new PluginInitializationException(
                "The Bluetooth plug-in must be named '" + getPluginName() + "'.");
        }

        return new PluginI(communicator);
    }
}
