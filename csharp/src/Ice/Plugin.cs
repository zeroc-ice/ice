// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents a communicator plug-in. A plug-in generally adds a feature to a communicator, such as support for an
/// additional transport.
/// </summary>
/// <remarks>The communicator loads its plug-ins in two stages: the first stage creates the plug-ins, and the second
/// stage calls <see cref="initialize" /> on each one.</remarks>
public interface Plugin
{
    /// <summary>
    /// Performs any necessary initialization steps.
    /// </summary>
    void initialize();

    /// <summary>
    /// Destroys this plugin. This method is called when the communicator is destroyed.
    /// </summary>
    void destroy();
}

/// <summary>
/// Manages the plug-ins of a communicator.
/// </summary>
public interface PluginManager
{
    /// <summary>
    /// Initializes the configured plug-ins. The communicator automatically initializes the plug-ins by default, but
    /// an application may need to interact directly with a plug-in prior to initialization. In this case, the
    /// application must set <c>Ice.InitPlugins=0</c> and then invoke <c>initializePlugins</c> manually. The plug-ins
    /// are initialized in the order in which they are loaded. If a plug-in throws an exception during initialization,
    /// the communicator calls <see cref="Plugin.destroy" /> on the plug-ins that have already been initialized.
    /// </summary>
    /// <exception cref="InitializationException">Thrown when the plug-ins have already been initialized.</exception>
    void initializePlugins();

    /// <summary>
    /// Gets the installed plug-ins.
    /// </summary>
    /// <returns>The names of the installed plug-ins.</returns>
    /// <seealso cref="getPlugin"/>
    string[] getPlugins();

    /// <summary>
    /// Gets a plug-in by name.
    /// </summary>
    /// <param name="name">The plug-in's name.</param>
    /// <returns>The plug-in.</returns>
    /// <exception cref="NotRegisteredException">Thrown when no plug-in is found with the given name.</exception>
    Plugin getPlugin(string name);

    /// <summary>
    /// Installs a new plug-in.
    /// </summary>
    /// <param name="name">The plug-in's name.</param>
    /// <param name="pi">The plug-in.</param>
    /// <exception cref="AlreadyRegisteredException">Thrown when a plug-in already exists with the given name.</exception>
    void addPlugin(string name, Plugin pi);

    /// <summary>
    /// Destroys this plug-in manager. Called when the communicator is being destroyed.
    /// </summary>
    void destroy();
}
