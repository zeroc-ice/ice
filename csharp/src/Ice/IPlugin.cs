// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A plug-in adds a feature to a communicator, such as support for a transport. The plug-ins are created
    /// when the communicator is constructed, and are later activated asynchronously when the communicator is
    /// activated asynchronously.</summary>
    public interface IPlugin : IAsyncDisposable
    {
        /// <summary>Performs any necessary activation steps.</summary>
        /// <param name="cancel">The cancellation token.</param>
        /// <returns>A task that completes once the activation completes.</returns>
        Task ActivateAsync(CancellationToken cancel);
    }

    /// <summary>Applications implement this interface to provide a plug-in factory to the Ice runtime.</summary>
    public interface IPluginFactory
    {
        /// <summary>Creates a new plug-in.</summary>
        /// <param name="communicator">The communicator being constructed.</param>
        /// <param name="name">The name of the plug-in.</param>
        /// <param name="args">The arguments specified in the plug-in configuration.</param>
        /// <returns>The plug-in created by this factory.</returns>
        IPlugin Create(Communicator communicator, string name, string[] args);
    }

    internal static class PluginLoader
    {
        /// <summary>Loads all the plug-ins of this communicator. This helper method is called by a communicator during
        /// its construction.</summary>
        /// <param name="communicator">The communicator being constructed.</param>
        /// <param name="cmdArgs">The remaining command-line arguments.</param>
        internal static void LoadPlugins(Communicator communicator, ref string[] cmdArgs)
        {
            const string prefix = "Ice.Plugin.";
            Dictionary<string, string> plugins = communicator.GetProperties(forPrefix: prefix);

            // Loads the plug-ins defined in the property set with the prefix "Ice.Plugin.". These properties should
            // have the following format:
            //
            // Ice.Plugin.name[.<language>]=entry_point [args]
            //
            // If the Ice.PluginLoadOrder property is defined, load the specified plug-ins in the specified order, then
            // load any remaining plug-ins.

            string[] loadOrder = communicator.GetPropertyAsList("Ice.PluginLoadOrder") ?? Array.Empty<string>();
            foreach (string name in loadOrder)
            {
                if (name.Length == 0)
                {
                    continue;
                }

                if (communicator.Plugins.Any(p => p.Name == name))
                {
                    throw new InvalidConfigurationException($"plug-in `{name}' already loaded");
                }

                string key = $"Ice.Plugin.{name}.clr";
                if (!plugins.TryGetValue(key, out string? value))
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
                    throw new InvalidConfigurationException($"plug-in `{name}' not defined");
                }
            }

            // Load any remaining plug-ins that weren't specified in PluginLoadOrder.
            while (plugins.Count > 0)
            {
                IEnumerator<KeyValuePair<string, string>> p = plugins.GetEnumerator();
                p.MoveNext();
                string key = p.Current.Key;
                string val = p.Current.Value;
                string name = key[prefix.Length..];

                int dotPos = name.LastIndexOf('.');
                if (dotPos != -1)
                {
                    string suffix = name[(dotPos + 1)..];
                    if (suffix == "cpp" || suffix == "java")
                    {
                        // Ignored
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
                        // Name is just a regular name that happens to contain a dot
                        dotPos = -1;
                    }
                }

                if (dotPos == -1)
                {
                    plugins.Remove(key);

                    // Is there a .clr entry?
                    string clrKey = $"Ice.Plugin.{name}.clr";
                    if (plugins.ContainsKey(clrKey))
                    {
                        val = plugins[clrKey];
                        plugins.Remove(clrKey);
                    }
                    LoadPlugin(name, val, ref cmdArgs);
                }
            }

            void LoadPlugin(string name, string pluginSpec, ref string[] cmdArgs)
            {
                string[] args = Array.Empty<string>();
                string? entryPoint = null;
                if (pluginSpec.Length > 0)
                {
                    // Split the entire property value into arguments. An entry point containing spaces must be enclosed
                    // in quotes.
                    try
                    {
                        args = Options.Split(pluginSpec);
                    }
                    catch (FormatException ex)
                    {
                        throw new InvalidConfigurationException($"invalid arguments for plug-in `{name}'", ex);
                    }

                    Debug.Assert(args.Length > 0);

                    entryPoint = args[0];

                    args = args.Skip(1).ToArray();

                    // Convert command-line options into properties. First we convert the options from the plug-in
                    // configuration, then we convert the options from the application command-line.
                    var properties = new Dictionary<string, string>();
                    properties.ParseArgs(ref args, name);
                    properties.ParseArgs(ref cmdArgs, name);
                    foreach (KeyValuePair<string, string> p in properties)
                    {
                        communicator.SetProperty(p.Key, p.Value);
                    }
                }
                Debug.Assert(entryPoint != null);

                // Extract the assembly name and the class name.
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

                Assembly? pluginAssembly;
                string assemblyName = entryPoint[..sepPos];
                string className = entryPoint[(sepPos + 1)..];

                try
                {
                    // First try to load the assembly using Assembly.Load, which will succeed if a fully-qualified
                    // name is provided or if a partial name has been qualified in configuration. If that fails, try
                    // Assembly.LoadFrom(), which will succeed if a file name is configured or a partial name is
                    // configured and DEVPATH is used.
                    //
                    // We catch System.Exception as this can fail with System.ArgumentNullException or
                    // System.IO.IOException depending of the .NET framework and platform.
                    try
                    {
                        pluginAssembly = Assembly.Load(assemblyName);
                    }
                    catch (Exception ex)
                    {
                        try
                        {
                            pluginAssembly = Assembly.LoadFrom(assemblyName);
                        }
                        catch (System.IO.IOException)
                        {
                            throw ExceptionUtil.Throw(ex);
                        }
                    }
                }
                catch (Exception ex)
                {
                    throw new LoadException(
                        $"error loading plug-in `{entryPoint}': unable to load assembly: `{assemblyName}'", ex);
                }

                // Instantiate the class.
                Type? c;
                try
                {
                    c = pluginAssembly.GetType(className, true);
                }
                catch (Exception ex)
                {
                    throw new LoadException(
                        $"error loading plug-in `{entryPoint}': cannot find the plugin factory class `{className}'",
                        ex);
                }
                Debug.Assert(c != null);

                IPluginFactory pluginFactory;
                try
                {
                    pluginFactory = (IPluginFactory?)Activator.CreateInstance(c)!;
                }
                catch (Exception ex)
                {
                    throw new LoadException($"error loading plug-in `{entryPoint}'", ex);
                }
                communicator.AddPlugin(name, pluginFactory.Create(communicator, name, args));
            }
        }
    }
}
