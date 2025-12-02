// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Applications implement this interface to provide a plug-in factory to the Ice runtime. */
public interface PluginFactory {
    /**
     * Returns the default and preferred name for plug-ins created by this factory.
     *
     * @return the name for plug-ins created by this factory
     */
    String getPluginName();

    /**
     * A plug-in factory function responsible for creating an Ice plug-in.
     *
     * @param communicator the communicator in which the plug-in will be installed
     * @param name the name assigned to the plug-in
     * @param args additional arguments included in the plug-in's configuration
     * @return a new plug-in object
     */
    Plugin create(Communicator communicator, String name, String[] args);
}
