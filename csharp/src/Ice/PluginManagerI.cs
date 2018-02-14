// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#if !SILVERLIGHT
namespace Ice
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;

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

    public sealed class PluginManagerI : PluginManager
    {
        private static string _kindOfObject = "plugin";

        public void initializePlugins()
        {
            if(_initialized)
            {
                InitializationException ex = new InitializationException();
                ex.reason = "plug-ins already initialized";
                throw ex;
            }

            //
            // Invoke initialize() on the plug-ins, in the order they were loaded.
            //
            ArrayList initializedPlugins = new ArrayList();
            try
            {
                foreach(PluginInfo p in _plugins)
                {
                    try
                    {
                        p.plugin.initialize();
                    }
                    catch(PluginInitializationException ex)
                    {
                        throw ex;
                    }
                    catch(System.Exception ex)
                    {
                        PluginInitializationException e = new PluginInitializationException(ex);
                        e.reason = "plugin `" + p.name + "' initialization failed";
                        throw e;
                    }
                    initializedPlugins.Add(p.plugin);
                }
            }
            catch(System.Exception)
            {
                //
                // Destroy the plug-ins that have been successfully initialized, in the
                // reverse order.
                //
                initializedPlugins.Reverse();
                foreach(Plugin p in initializedPlugins)
                {
                    try
                    {
                        p.destroy();
                    }
                    catch(System.Exception)
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
            lock(this)
            {
                ArrayList names = new ArrayList();
                foreach(PluginInfo p in _plugins)
                {
                    names.Add(p.name);
                }
                return (string[])names.ToArray(typeof(string));
            }
        }

        public Plugin getPlugin(string name)
        {
            lock(this)
            {
                if(_communicator == null)
                {
                    throw new CommunicatorDestroyedException();
                }

                Plugin p = findPlugin(name);
                if(p != null)
                {
                    return p;
                }

                NotRegisteredException ex = new NotRegisteredException();
                ex.id = name;
                ex.kindOfObject = _kindOfObject;
                throw ex;
            }
        }

        public void addPlugin(string name, Plugin plugin)
        {
            lock(this)
            {
                if(_communicator == null)
                {
                    throw new CommunicatorDestroyedException();
                }

                if(findPlugin(name) != null)
                {
                    AlreadyRegisteredException ex = new AlreadyRegisteredException();
                    ex.id = name;
                    ex.kindOfObject = _kindOfObject;
                    throw ex;
                }

                PluginInfo info = new PluginInfo();
                info.name = name;
                info.plugin = plugin;
                _plugins.Add(info);
            }
        }

        public void destroy()
        {
            lock(this)
            {
                if(_communicator != null)
                {
                    if(_initialized)
                    {
                        ArrayList plugins = (ArrayList)_plugins.Clone();
                        plugins.Reverse();
                        foreach(PluginInfo p in plugins)
                        {
                            try
                            {
                                p.plugin.destroy();
                            }
                            catch(System.Exception ex)
                            {
                                Ice.Util.getProcessLogger().warning("unexpected exception raised by plug-in `" +
                                                                    p.name + "' destruction:\n" + ex.ToString());
                            }
                        }
                    }

                    _communicator = null;
                }
            }
        }

        public PluginManagerI(Communicator communicator)
        {
            _communicator = communicator;
            _plugins = new ArrayList();
            _initialized = false;
        }

        public void loadPlugins(ref string[] cmdArgs)
        {
            Debug.Assert(_communicator != null);

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
            string prefix = "Ice.Plugin.";
            Properties properties = _communicator.getProperties();
            Dictionary<string, string> plugins = properties.getPropertiesForPrefix(prefix);

            string[] loadOrder = properties.getPropertyAsList("Ice.PluginLoadOrder");
            for(int i = 0; i < loadOrder.Length; ++i)
            {
                if(loadOrder[i].Length == 0)
                {
                    continue;
                }

                if(findPlugin(loadOrder[i]) != null)
                {
                    PluginInitializationException e = new PluginInitializationException();
                    e.reason = "plug-in `" + loadOrder[i] + "' already loaded";
                    throw e;
                }

                string key = "Ice.Plugin." + loadOrder[i] + ".clr";
                bool hasKey = plugins.ContainsKey(key);
                if(hasKey)
                {
                    plugins.Remove("Ice.Plugin." + loadOrder[i]);
                }
                else
                {
                    key = "Ice.Plugin." + loadOrder[i];
                    hasKey = plugins.ContainsKey(key);
                }

                if(hasKey)
                {
                    string value = plugins[key];
                    loadPlugin(loadOrder[i], value, ref cmdArgs);
                    plugins.Remove(key);
                }
                else
                {
                    PluginInitializationException e = new PluginInitializationException();
                    e.reason = "plug-in `" + loadOrder[i] + "' not defined";
                    throw e;
                }
            }

            //
            // Load any remaining plug-ins that weren't specified in PluginLoadOrder.
            //
            while(plugins.Count > 0)
            {
                IEnumerator<KeyValuePair<string, string>> p = plugins.GetEnumerator();
                p.MoveNext();
                string key = p.Current.Key;
                string val = p.Current.Value;
                string name = key.Substring(prefix.Length);

                int dotPos = name.LastIndexOf('.');
                if(dotPos != -1)
                {
                    string suffix = name.Substring(dotPos + 1);
                    if(suffix.Equals("cpp") || suffix.Equals("java"))
                    {
                        //
                        // Ignored
                        //
                        plugins.Remove(key);
                    }
                    else if(suffix.Equals("clr"))
                    {
                        name = name.Substring(0, dotPos);
                        loadPlugin(name, val, ref cmdArgs);
                        plugins.Remove(key);
                        plugins.Remove("Ice.Plugin." + name);

                    }
                    else
                    {
                        //
                        // Name is just a regular name that happens to contain a dot
                        //
                        dotPos = -1;
                    }
                }

                if(dotPos == -1)
                {
                    plugins.Remove(key);

                    //
                    // Is there a .clr entry?
                    //
                    string clrKey = "Ice.Plugin." + name + ".clr";
                    if(plugins.ContainsKey(clrKey))
                    {
                        val = plugins[clrKey];
                        plugins.Remove(clrKey);
                    }
                    loadPlugin(name, val, ref cmdArgs);
                }
            }
        }

        private void loadPlugin(string name, string pluginSpec, ref string[] cmdArgs)
        {
            Debug.Assert(_communicator != null);

            //
            // Split the entire property value into arguments. An entry point containing spaces
            // must be enclosed in quotes.
            //
            string[] args = null;
            try
            {
                args = IceUtilInternal.Options.split(pluginSpec);
            }
            catch(IceUtilInternal.Options.BadQuote ex)
            {
                PluginInitializationException e = new PluginInitializationException();
                e.reason = "invalid arguments for plug-in `" + name + "':\n" + ex.Message;
                throw e;
            }

            Debug.Assert(args.Length > 0);

            string entryPoint = args[0];

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

            //
            // Extract the assembly name and the class name.
            //
            string err = "unable to load plug-in `" + entryPoint + "': ";
            int sepPos = entryPoint.IndexOf(':');
            if(sepPos != -1 && IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows)
            {
                const string driveLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
                if(entryPoint.Length > 3 &&
                   sepPos == 1 &&
                   driveLetters.IndexOf(entryPoint[0]) != -1 &&
                   (entryPoint[2] == '\\' || entryPoint[2] == '/'))
                {
                    sepPos = entryPoint.IndexOf(':', 3);
                }
            }
            if(sepPos == -1)
            {
                PluginInitializationException e = new PluginInitializationException();
                e.reason = err + "invalid entry point format";
                throw e;
            }

            System.Reflection.Assembly pluginAssembly = null;
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
                try
                {
                    pluginAssembly = System.Reflection.Assembly.Load(assemblyName);
                }
                catch(System.IO.IOException ex)
                {
                    try
                    {
                        pluginAssembly = System.Reflection.Assembly.LoadFrom(assemblyName);
                    }
                    catch(System.IO.IOException)
                    {
                         throw ex;
                    }
                }
            }
            catch(System.Exception ex)
            {
#if COMPACT
                //
                // IceSSL is not supported with the Compact Framework.
                //
                if(name == "IceSSL")
                {
                    if(!_sslWarnOnce)
                    {
                        _communicator.getLogger().warning(
                            "IceSSL plug-in not loaded: IceSSL is not supported with the .NET Compact Framework");
                        _sslWarnOnce = true;
                    }
                    return;
                }
#else
                //
                // IceSSL is not yet supported with Mono. We avoid throwing an exception in that case,
                // so the same configuration can be used with Mono or Visual C#.
                //
                if(IceInternal.AssemblyUtil.runtime_ == IceInternal.AssemblyUtil.Runtime.Mono && name == "IceSSL")
                {
                    if(!_sslWarnOnce)
                    {
                        _communicator.getLogger().warning(
                            "IceSSL plug-in not loaded: IceSSL is not supported with Mono");
                        _sslWarnOnce = true;
                    }
                    return;
                }
#endif

                PluginInitializationException e = new PluginInitializationException();
                e.reason = err + "unable to load assembly: `" + assemblyName + "': " + ex.ToString();
                throw e;
            }

            //
            // Instantiate the class.
            //
            PluginFactory pluginFactory = null;
            System.Type c = null;
            try
            {
                c = pluginAssembly.GetType(className, true);
            }
            catch(System.Exception ex)
            {
                PluginInitializationException e = new PluginInitializationException(ex);
                e.reason = err + "GetType failed for `" + className + "'";
                throw e;
            }

            try
            {
                pluginFactory = (PluginFactory)IceInternal.AssemblyUtil.createInstance(c);
                if(pluginFactory == null)
                {
                    PluginInitializationException e = new PluginInitializationException();
                    e.reason = err + "can't find constructor for `" + className + "'";
                    throw e;
                }
            }
            catch(System.InvalidCastException ex)
            {
                PluginInitializationException e = new PluginInitializationException(ex);
                e.reason = err + "InvalidCastException to Ice.PluginFactory";
                throw e;
            }
            catch(System.UnauthorizedAccessException ex)
            {
                PluginInitializationException e = new PluginInitializationException(ex);
                e.reason = err + "UnauthorizedAccessException: " + ex.ToString();
                throw e;
            }
            catch(System.Exception ex)
            {
                PluginInitializationException e = new PluginInitializationException(ex);
                e.reason = err + "System.Exception: " + ex.ToString();
                throw e;
            }

            Plugin plugin = null;
            try
            {
                plugin = pluginFactory.create(_communicator, name, args);
            }
            catch(PluginInitializationException ex)
            {
                ex.reason = err + ex.reason;
                throw ex;
            }
            catch(System.Exception ex)
            {
                PluginInitializationException e = new PluginInitializationException(ex);
                e.reason = err + "System.Exception in factory.create: " + ex.ToString();
                throw e;
            }

            if(plugin == null)
            {
                PluginInitializationException ex = new PluginInitializationException();
                ex.reason = err + "factory.create returned null plug-in";
                throw ex;
            }

            PluginInfo info = new PluginInfo();
            info.name = name;
            info.plugin = plugin;
            _plugins.Add(info);
        }

        private Plugin findPlugin(string name)
        {
            foreach(PluginInfo p in _plugins)
            {
                if(name.Equals(p.name))
                {
                    return p.plugin;
                }
            }
            return null;
        }

        internal class PluginInfo
        {
            internal string name;
            internal Plugin plugin;
        }

        private Communicator _communicator;
        private ArrayList _plugins;
        private bool _initialized;
        private static bool _sslWarnOnce = false;
    }
}
#endif
