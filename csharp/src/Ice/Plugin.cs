//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
namespace Ice
{
    public partial interface Plugin
    {
        /// <summary>
        /// Perform any necessary initialization steps.
        /// </summary>
        void initialize();

        /// <summary>
        /// Called when the communicator is being destroyed.
        /// </summary>
        void destroy();
    }

    public partial interface PluginManager
    {
        /// <summary>
        /// Initialize the configured plug-ins.
        /// The communicator automatically initializes
        /// the plug-ins by default, but an application may need to interact directly with
        /// a plug-in prior to initialization. In this case, the application must set
        /// Ice.InitPlugins=0 and then invoke initializePlugins
        /// manually. The plug-ins are initialized in the order in which they are loaded.
        /// If a plug-in raises an exception during initialization, the communicator
        /// invokes destroy on the plug-ins that have already been initialized.
        ///
        /// </summary>
        /// <exception name="InitializationException">Raised if the plug-ins have already been initialized.</exception>
        void initializePlugins();

        /// <summary>
        /// Get a list of plugins installed.
        /// </summary>
        /// <returns>The names of the plugins installed.
        ///
        /// </returns>
        string[] getPlugins();

        /// <summary>
        /// Obtain a plug-in by name.
        /// </summary>
        /// <param name="name">The plug-in's name.
        ///
        /// </param>
        /// <returns>The plug-in.
        ///
        /// </returns>
        /// <exception name="NotRegisteredException">Raised if no plug-in is found with the given name.</exception>
        Plugin getPlugin(string name);

        /// <summary>
        /// Install a new plug-in.
        /// </summary>
        /// <param name="name">The plug-in's name.
        ///
        /// </param>
        /// <param name="pi">The plug-in.
        ///
        /// </param>
        /// <exception name="AlreadyRegisteredException">Raised if a plug-in already exists with the given name.</exception>
        void addPlugin(string name, Plugin pi);

        /// <summary>
        /// Called when the communicator is being destroyed.
        /// </summary>
        void destroy();
    }
}
