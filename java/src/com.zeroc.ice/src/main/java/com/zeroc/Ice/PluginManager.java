// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Manages the plug-ins of a communicator. */
public interface PluginManager {
    /**
     * Initializes the configured plug-ins. The communicator automatically initializes the plug-ins by default, but
     * an application may need to interact directly with a plug-in prior to initialization. In this case, the
     * application must set `Ice.InitPlugins=0` and then invoke `initializePlugins` manually. The plug-ins are
     * initialized in the order in which they are loaded. If a plug-in throws an exception during initialization,
     * the communicator calls {@link Plugin#destroy} on the plug-ins that have already been initialized.
     *
     * @throws InitializationException if the plug-ins have already been initialized
     */
    void initializePlugins();

    /**
     * Gets the installed plug-ins.
     *
     * @return the names of the installed plug-ins
     * @see #getPlugin
     */
    String[] getPlugins();

    /**
     * Gets a plug-in by name.
     *
     * @param name the plug-in's name
     * @return the plug-in
     * @throws NotRegisteredException if no plug-in is found with the given name
     */
    Plugin getPlugin(String name);

    /**
     * Installs a new plug-in.
     *
     * @param name the plug-in's name
     * @param pi the plug-in
     * @throws AlreadyRegisteredException if a plug-in already exists with the given name
     */
    void addPlugin(String name, Plugin pi);

    /** Destroys this plug-in manager. Called when the communicator is being destroyed. */
    void destroy();
}
