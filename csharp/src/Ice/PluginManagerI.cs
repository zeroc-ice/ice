//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace Ice
{
    /// <summary>
    /// Applications implement this interface to provide a plug-in factory
    /// to the Ice run time.
    /// </summary>
    public interface PluginFactory
    {
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

    public sealed partial class Communicator
    {
        public static void RegisterPluginFactory(string name, PluginFactory factory, bool loadOnInit)
        {
            if (!_factories.ContainsKey(name))
            {
                _factories[name] = factory;
                if (loadOnInit)
                {
                    _loadOnInitialization.Add(name);
                }
            }
        }

        public void InitializePlugins()
        {
            if (_initialized)
            {
                InitializationException ex = new InitializationException();
                ex.reason = "plug-ins already initialized";
                throw ex;
            }

            //
            // Invoke initialize() on the plug-ins, in the order they were loaded.
            //
            List<Plugin> initializedPlugins = new List<Plugin>();
            try
            {
                foreach (var p in _plugins)
                {
                    p.Plugin.initialize();
                    initializedPlugins.Add(p.Plugin);
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

        public string[] GetPlugins()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                return _plugins.Select(p => p.Name).ToArray();
            }
        }

        public Plugin? GetPlugin(string name)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                return FindPlugin(name);
            }
        }

        public void AddPlugin(string name, Plugin plugin)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (FindPlugin(name) != null)
                {
                    throw new ArgumentException("A plugin named `{name}' is already resgistered", nameof(name));
                }

                _plugins.Add((name, plugin));
            }
        }

        private void LoadPlugins(ref string[] cmdArgs)
        {
            const string prefix = "Ice.Plugin.";
            Dictionary<string, string> plugins = GetProperties(forPrefix: prefix);

            //
            // First, load static plugin factories which were setup to load on
            // communicator initialization. If a matching plugin property is
            // set, we load the plugin with the plugin specification. The
            // entryPoint will be ignored but the rest of the plugin
            // specification might be used.
            //
            foreach (var name in _loadOnInitialization)
            {
                string key = $"Ice.Plugin.{name}.clr";
                string? r;
                plugins.TryGetValue(key, out r);
                if (r == null)
                {
                    key = $"Ice.Plugin.{name}";
                    plugins.TryGetValue(key, out r);
                }

                if (r != null)
                {
                    LoadPlugin(name, r, ref cmdArgs);
                    plugins.Remove(key);
                }
                else
                {
                    LoadPlugin(name, "", ref cmdArgs);
                }
            }

            //
            // Load and initialize the plug-ins defined in the property set
            // with the prefix "Ice.Plugin.". These properties should
            // have the following format:
            //
            // Ice.Plugin.name[.<language>]=entry_point [args]
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

            string[] loadOrder = GetPropertyAsList("Ice.PluginLoadOrder") ?? Array.Empty<string>();
            foreach (var name in loadOrder)
            {
                if (name.Length == 0)
                {
                    continue;
                }

                if (FindPlugin(name) != null)
                {
                    throw new InvalidOperationException($"plug-in `{name}' already loaded");
                }

                string key = $"Ice.Plugin.{name}clr";
                string? value;
                plugins.TryGetValue(key, out value);
                if (value == null)
                {
                    key = $"Ice.Plugin.{name}";
                    plugins.TryGetValue(key, out value);
                }

                if (value != null)
                {
                    LoadPlugin(name, value, ref cmdArgs);
                    plugins.Remove(key);
                }
                else
                {
                    throw new InvalidOperationException($"plug-in `{name}' not defined");
                }
            }

            //
            // Load any remaining plug-ins that weren't specified in PluginLoadOrder.
            //
            while (plugins.Count > 0)
            {
                IEnumerator<KeyValuePair<string, string>> p = plugins.GetEnumerator();
                p.MoveNext();
                string key = p.Current.Key;
                string val = p.Current.Value;
                string name = key.Substring(prefix.Length);

                int dotPos = name.LastIndexOf('.');
                if (dotPos != -1)
                {
                    string suffix = name.Substring(dotPos + 1);
                    if (suffix == "cpp" || suffix == "java")
                    {
                        //
                        // Ignored
                        //
                        plugins.Remove(key);
                    }
                    else if (suffix == "clr")
                    {
                        name = name.Substring(0, dotPos);
                        LoadPlugin(name, val, ref cmdArgs);
                        plugins.Remove(key);
                        plugins.Remove($"Ice.Plugin.{name}");

                    }
                    else
                    {
                        //
                        // Name is just a regular name that happens to contain a dot
                        //
                        dotPos = -1;
                    }
                }

                if (dotPos == -1)
                {
                    plugins.Remove(key);

                    //
                    // Is there a .clr entry?
                    //
                    string clrKey = $"Ice.Plugin.{name}.clr";
                    if (plugins.ContainsKey(clrKey))
                    {
                        val = plugins[clrKey];
                        plugins.Remove(clrKey);
                    }
                    LoadPlugin(name, val, ref cmdArgs);
                }
            }
        }

        private void LoadPlugin(string name, string pluginSpec, ref string[] cmdArgs)
        {
            string[] args = Array.Empty<string>();
            string? entryPoint = null;
            if (pluginSpec.Length > 0)
            {
                //
                // Split the entire property value into arguments. An entry point containing spaces
                // must be enclosed in quotes.
                //
                try
                {
                    args = IceUtilInternal.Options.split(pluginSpec);
                }
                catch (FormatException ex)
                {
                    throw new ArgumentException($"invalid arguments for plug-in `{name}", nameof(pluginSpec), ex);
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
                Dictionary<string, string> properties = new Dictionary<string, string>();
                properties.ParseArgs(ref args, name);
                properties.ParseArgs(ref cmdArgs, name);
                foreach (var p in properties)
                {
                    SetProperty(p.Key, p.Value);
                }
            }
            Debug.Assert(entryPoint != null);
            //
            // Always check the static plugin factory table first, it takes
            // precedence over the the entryPoint specified in the plugin
            // property value.
            //
            PluginFactory? pluginFactory;
            if (!_factories.TryGetValue(name, out pluginFactory))
            {
                //
                // Extract the assembly name and the class name.
                //
                int sepPos = entryPoint.IndexOf(':');
                if (sepPos != -1)
                {
                    const string driveLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
                    if (entryPoint.Length > 3 &&
                       sepPos == 1 &&
                       driveLetters.IndexOf(entryPoint[0]) != -1 &&
                       (entryPoint[2] == '\\' || entryPoint[2] == '/'))
                    {
                        sepPos = entryPoint.IndexOf(':', 3);
                    }
                }
                if (sepPos == -1)
                {
                    throw new FormatException($"error loading plug-in `{entryPoint}': invalid entry point format");
                }

                System.Reflection.Assembly? pluginAssembly;
                string assemblyName = entryPoint.Substring(0, sepPos);
                string className = entryPoint.Substring(sepPos + 1);

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
#pragma warning disable CA2200 // Rethrow to preserve stack details.
                            throw ex;
#pragma warning restore CA2200 // Rethrow to preserve stack details.
                        }
                    }
                }
                catch (System.Exception ex)
                {
                    throw new InvalidOperationException(
                        $"error loading plug-in `{entryPoint}': unable to load assembly: `{assemblyName}'", ex);
                }

                //
                // Instantiate the class.
                //
                Type c;
                try
                {
                    c = pluginAssembly.GetType(className, true);
                }
                catch (System.Exception ex)
                {
                    throw new InvalidOperationException(
                        $"error loading plug-in `{ entryPoint}': cannot find the plugin factory class `{className}'", ex);
                }

                try
                {
                    pluginFactory = (PluginFactory?)IceInternal.AssemblyUtil.createInstance(c);
                }
                catch (System.Exception ex)
                {
                    throw new InvalidOperationException($"error loading plug-in `{entryPoint}'", ex);
                }

                if (pluginFactory == null)
                {
                    throw new InvalidOperationException(
                        $"error loading plug-in `{ entryPoint}': can't find constructor for `{className}'");
                }
            }

            _plugins.Add((name, pluginFactory.create(this, name, args)));
        }

        private Plugin? FindPlugin(string name)
        {
            return _plugins.FirstOrDefault(p => p.Name == name).Plugin;
        }

        private readonly List<(string Name, Plugin Plugin)> _plugins = new List<(string Name, Plugin Plugin)>();
        private bool _initialized;

        private static Dictionary<string, PluginFactory> _factories = new Dictionary<string, PluginFactory>();
        private static List<string> _loadOnInitialization = new List<string>();
    }
}
