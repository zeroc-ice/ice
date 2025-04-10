// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Collections;
using System.Diagnostics;

namespace Ice;

/// <summary>
/// Applications implement this interface to provide a plug-in factory
/// to the Ice run time.
/// </summary>
public interface PluginFactory
{
    /// <summary>Gets the preferred name for plug-ins created by this factory.</summary>
    string preferredName { get; }

    /// <summary>
    /// Called by the Ice run time to create a new plug-in.
    /// </summary>
    ///
    /// <param name="communicator">The communicator that is in the process of being initialized.</param>
    /// <param name="name">The name of the plug-in.</param>
    /// <param name="args">The arguments that are specified in the plug-ins configuration.</param>
    /// <returns>The plug-in that was created by this method.</returns>
    Plugin create(Communicator communicator, string name, string[] args);
}

internal sealed class PluginManagerI : PluginManager
{
    private const string _kindOfObject = "plugin";

    public void initializePlugins()
    {
        if (_initialized)
        {
            throw new InitializationException("Plug-ins already initialized.");
        }

        //
        // Invoke initialize() on the plug-ins, in the order they were loaded.
        //
        var initializedPlugins = new ArrayList();
        try
        {
            foreach (PluginInfo p in _plugins)
            {
                try
                {
                    p.plugin.initialize();
                }
                catch (PluginInitializationException)
                {
                    throw;
                }
                catch (System.Exception ex)
                {
                    throw new PluginInitializationException($"Plugin '{p.name}' initialization failed.", ex);
                }
                initializedPlugins.Add(p.plugin);
            }
        }
        catch (System.Exception)
        {
            //
            // Destroy the plug-ins that have been successfully initialized, in the
            // reverse order.
            //
            initializedPlugins.Reverse();
            foreach (Plugin p in initializedPlugins)
            {
                try
                {
                    p.destroy();
                }
                catch (System.Exception)
                {
                    // Ignore.
                }
            }
            throw;
        }

        _initialized = true;
    }

    public string[] getPlugins()
    {
        lock (_mutex)
        {
            var names = new ArrayList();
            foreach (PluginInfo p in _plugins)
            {
                names.Add(p.name);
            }
            return (string[])names.ToArray(typeof(string));
        }
    }

    public Plugin getPlugin(string name)
    {
        lock (_mutex)
        {
            if (_communicator is null)
            {
                throw new CommunicatorDestroyedException();
            }

            Plugin? p = findPlugin(name);
            if (p is not null)
            {
                return p;
            }

            throw new NotRegisteredException(_kindOfObject, name);
        }
    }

    public void addPlugin(string name, Plugin plugin)
    {
        lock (_mutex)
        {
            if (_communicator is null)
            {
                throw new CommunicatorDestroyedException();
            }

            if (findPlugin(name) is not null)
            {
                throw new AlreadyRegisteredException(_kindOfObject, name);
            }

            _plugins.Add(new PluginInfo(name, plugin));
        }
    }

    public void destroy()
    {
        lock (_mutex)
        {
            if (_communicator is not null)
            {
                if (_initialized)
                {
                    var plugins = (ArrayList)_plugins.Clone();
                    plugins.Reverse();
                    foreach (PluginInfo p in plugins)
                    {
                        try
                        {
                            p.plugin.destroy();
                        }
                        catch (System.Exception ex)
                        {
                            Util.getProcessLogger().warning("unexpected exception raised by plug-in `" +
                                                            p.name + "' destruction:\n" + ex.ToString());
                        }
                    }
                }

                _communicator = null;
            }
        }
    }

    internal PluginManagerI(Communicator communicator)
    {
        _communicator = communicator;
        _plugins = new ArrayList();
        _initialized = false;
    }

    public void loadPlugins(ref string[] cmdArgs)
    {
        Debug.Assert(_communicator is not null);
        string prefix = "Ice.Plugin.";
        Properties properties = _communicator.getProperties();
        Dictionary<string, string> plugins = properties.getPropertiesForPrefix(prefix);

        // First, create plug-ins using the plug-in factories from initData, in order.
        foreach (PluginFactory pluginFactory in _communicator.instance.initializationData().pluginFactories)
        {
            string name = pluginFactory.preferredName;
            string key = $"Ice.Plugin.{name}";
            if (plugins.TryGetValue(key, out string? pluginSpec))
            {
                loadPlugin(pluginFactory, name, pluginSpec, ref cmdArgs);
                plugins.Remove(key);
            }
            else
            {
                loadPlugin(pluginFactory, name, "", ref cmdArgs);
            }
        }

        //
        // Load and initialize the plug-ins defined in the property set
        // with the prefix "Ice.Plugin.". These properties should
        // have the following format:
        //
        // Ice.Plugin.<name>=entry_point [args]
        //
        // The code below is different from the Java/C++ algorithm
        // because C# must support full assembly names such as:
        //
        // Ice.Plugin.Logger=logger, Version=0.0.0.0, Culture=neutral:LoginPluginFactory
        //
        // If the Ice.PluginLoadOrder property is defined, load the
        // specified plug-ins in the specified order, then load any
        // remaining plug-ins.
        //

        string[] loadOrder = properties.getIcePropertyAsList("Ice.PluginLoadOrder");
        for (int i = 0; i < loadOrder.Length; ++i)
        {
            if (loadOrder[i].Length == 0)
            {
                continue;
            }

            if (findPlugin(loadOrder[i]) is not null)
            {
                throw new PluginInitializationException($"Plug-in '{loadOrder[i]}' already loaded.");
            }

            string key = $"Ice.Plugin.{loadOrder[i]}";
            plugins.TryGetValue(key, out string? value);
            if (value is not null)
            {
                loadPlugin(null, loadOrder[i], value, ref cmdArgs);
                plugins.Remove(key);
            }
            else
            {
                throw new PluginInitializationException($"Plug-in '{loadOrder[i]}' not defined.");
            }
        }

        //
        // Load any remaining plug-ins that weren't specified in PluginLoadOrder.
        //
        foreach (KeyValuePair<string, string> entry in plugins)
        {
            loadPlugin(null, entry.Key[prefix.Length..], entry.Value, ref cmdArgs);
        }
    }

    private void loadPlugin(PluginFactory? pluginFactory, string name, string pluginSpec, ref string[] cmdArgs)
    {
        Debug.Assert(_communicator is not null);

        string[] args = [];
        string entryPoint = "";
        if (pluginSpec.Length > 0)
        {
            //
            // Split the entire property value into arguments. An entry point containing spaces
            // must be enclosed in quotes.
            //
            try
            {
                args = Ice.UtilInternal.Options.split(pluginSpec);
            }
            catch (ParseException ex)
            {
                throw new PluginInitializationException($"Invalid arguments for plug-in '{name}'.", ex);
            }

            Debug.Assert(args.Length > 0);

            entryPoint = args[0];

            //
            // Shift the arguments.
            //
            string[] tmp = new string[args.Length - 1];
            Array.Copy(args, 1, tmp, 0, args.Length - 1);
            args = tmp;

            //
            // Convert command-line options into properties. First
            // we convert the options from the plug-in
            // configuration, then we convert the options from the
            // application command-line.
            //
            Properties properties = _communicator.getProperties();
            args = properties.parseCommandLineOptions(name, args);
            cmdArgs = properties.parseCommandLineOptions(name, cmdArgs);
        }

        string err = "unable to load plug-in `" + entryPoint + "': ";

        if (pluginFactory is null)
        {
            //
            // Extract the assembly name and the class name.
            //
            int sepPos = entryPoint!.IndexOf(':', StringComparison.Ordinal);
            if (sepPos != -1)
            {
                const string driveLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
                if (entryPoint.Length > 3 &&
                   sepPos == 1 &&
                   driveLetters.Contains(entryPoint[0], StringComparison.Ordinal) &&
                   (entryPoint[2] == '\\' || entryPoint[2] == '/'))
                {
                    sepPos = entryPoint.IndexOf(':', 3);
                }
            }
            if (sepPos == -1)
            {
                throw new PluginInitializationException($"{err}invalid entry point format");
            }

            string assemblyName = entryPoint[..sepPos];
            string className = entryPoint[(sepPos + 1)..];

            System.Reflection.Assembly? pluginAssembly;
            try
            {
                //
                // First try to load the assembly using Assembly.Load, which will succeed
                // if a fully-qualified name is provided or if a partial name has been qualified
                // in configuration. If that fails, try Assembly.LoadFrom(), which will succeed
                // if a file name is configured or a partial name is configured and DEVPATH is used.
                //
                // We catch System.Exception as this can fail with System.ArgumentNullException
                // or System.IO.IOException depending of the .NET framework and platform.
                //
                try
                {
                    pluginAssembly = System.Reflection.Assembly.Load(assemblyName);
                }
                catch (System.Exception ex)
                {
                    try
                    {
                        pluginAssembly = System.Reflection.Assembly.LoadFrom(assemblyName);
                    }
                    catch (System.IO.IOException)
                    {
#pragma warning disable CA2200 // Rethrow to preserve stack details
                        throw ex;
#pragma warning restore CA2200 // Rethrow to preserve stack details
                    }
                }
            }
            catch (System.Exception ex)
            {
                throw new PluginInitializationException($"{err}unable to load assembly '{assemblyName}'.", ex);
            }

            //
            // Instantiate the class.
            //
            Type? c;
            try
            {
                c = pluginAssembly.GetType(className, true);
            }
            catch (System.Exception ex)
            {
                throw new PluginInitializationException($"{err}GetType failed for '{className}'.", ex);
            }

            try
            {
                pluginFactory = (PluginFactory)Ice.Internal.AssemblyUtil.createInstance(c);
                if (pluginFactory is null)
                {
                    throw new PluginInitializationException($"{err}can't find constructor for '{className}'.");
                }
            }
            catch (System.Exception ex)
            {
                throw new PluginInitializationException($"{err}SystemException", ex);
            }
        }

        Plugin? plugin;
        try
        {
            plugin = pluginFactory.create(_communicator, name, args);
        }
        catch (PluginInitializationException)
        {
            throw;
        }
        catch (System.Exception ex)
        {
            throw new PluginInitializationException($"{err}System.Exception in factory.create", ex);
        }

        if (plugin is null)
        {
            throw new PluginInitializationException($"{err}factory.create returned null plug-in");
        }

        _plugins.Add(new PluginInfo(name, plugin));
    }

    private Plugin? findPlugin(string name)
    {
        foreach (PluginInfo p in _plugins)
        {
            if (name.Equals(p.name, StringComparison.Ordinal))
            {
                return p.plugin;
            }
        }
        return null;
    }

    internal record class PluginInfo(string name, Plugin plugin);

    private Communicator? _communicator;
    private readonly ArrayList _plugins;
    private bool _initialized;
    private readonly object _mutex = new();
}
