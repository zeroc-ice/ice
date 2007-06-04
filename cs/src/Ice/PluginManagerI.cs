// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System.Collections;
    using System.Diagnostics;

    public interface PluginFactory
    {
        Plugin create(Communicator communicator, string name, string[] args);
    }

    public interface LoggerFactory
    {
        Logger create(Communicator communicator, string[] args);
    }

    public sealed class PluginManagerI : PluginManager
    {
        private static string _kindOfObject = "plugin";

        public void initializePlugins()
        {
            if(_initialized)
            {
                InitializationException ex = new InitializationException();
                ex.reason = "plugins already initialized";
                throw ex;
            }

            //
            // Invoke initialize() on the plugins, in the order they were loaded.
            //
            ArrayList initializedPlugins = new ArrayList();
            try
            {
                foreach(Plugin p in _initOrder)
                {
                    p.initialize();
                    initializedPlugins.Add(p);
                }
            }
            catch(Exception)
            {
                //
                // Destroy the plugins that have been successfully initialized, in the
                // reverse order.
                //
                initializedPlugins.Reverse();
                foreach(Plugin p in initializedPlugins)
                {
                    try
                    {
                        p.destroy();
                    }
                    catch(Exception)
                    {
                        // Ignore.
                    }
                }
                throw;
            }

            _initialized = true;
        }

        public Plugin getPlugin(string name)
        {
            lock(this)
            {
                if(_communicator == null)
                {
                    throw new CommunicatorDestroyedException();
                }
            
                Plugin p = (Plugin)_plugins[name];
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
            
                if(_plugins.Contains(name))
                {
                    AlreadyRegisteredException ex = new AlreadyRegisteredException();
                    ex.id = name;
                    ex.kindOfObject = _kindOfObject;
                    throw ex;
                }
                _plugins[name] = plugin;
            }
        }

        public void destroy()
        {
            lock(this)
            {
                if(_communicator != null)
                {
                    foreach(Plugin plugin in _plugins.Values)
                    {
                        plugin.destroy();
                    }
                
                    _logger = null;
                    _communicator = null;
                }
            }
        }
        
        public PluginManagerI(Communicator communicator)
        {
            _communicator = communicator;
            _plugins = new Hashtable();
            _initOrder = new ArrayList();
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
            // Ice.Plugin.name=entry_point [args]
            //
            // The code below is different from the Java/C++ algorithm
            // because C# must support full assembly names such as:
            //
            // Ice.Plugin.Logger=logger, Version=0.0.0.0, Culture=neutral:LoginPluginFactory
            //
            // If the Ice.PluginLoadOrder property is defined, load the
            // specified plugins in the specified order, then load any
            // remaining plugins.
            //
            string prefix = "Ice.Plugin.";
            Properties properties = _communicator.getProperties();
            PropertyDict plugins = properties.getPropertiesForPrefix(prefix);

            string loadOrder = properties.getProperty("Ice.PluginLoadOrder");
            if(loadOrder.Length > 0)
            {
                char[] delims = { ',', ' ', '\t', '\n' };
                string[] names = loadOrder.Split(delims);
                for(int i = 0; i < names.Length; ++i)
                {
                    if(names[i].Length == 0)
                    {
                        continue;
                    }

                    if(_plugins.Contains(names[i]))
                    {
                        PluginInitializationException e = new PluginInitializationException();
                        e.reason = "plugin `" + names[i] + "' already loaded";
                        throw e;
                    }

                    string key = "Ice.Plugin." + names[i];
                    if(plugins.Contains(key))
                    {
                        string value = (string)plugins[key];
                        loadPlugin(names[i], value, ref cmdArgs, false);
                        plugins.Remove(key);
                    }
                    else
                    {
                        PluginInitializationException e = new PluginInitializationException();
                        e.reason = "plugin `" + names[i] + "' not defined";
                        throw e;
                    }
                }
            }

            //
            // Load any remaining plugins that weren't specified in PluginLoadOrder.
            //
            foreach(DictionaryEntry entry in plugins)
            {
                string name = ((string)entry.Key).Substring(prefix.Length);
                string val = (string)entry.Value;
                loadPlugin(name, val, ref cmdArgs, false);
            }

            //
            // Check for a Logger Plugin
            //
            string loggerStr = properties.getProperty("Ice.LoggerPlugin");
            if(loggerStr.Length != 0)
            {
                loadPlugin("Logger", loggerStr, ref cmdArgs, true);
            }

            //      
            // An application can set Ice.InitPlugins=0 if it wants to postpone
            // initialization until after it has interacted directly with the
            // plugins.
            //      
            if(properties.getPropertyAsIntWithDefault("Ice.InitPlugins", 1) > 0)
            {           
                initializePlugins();
            }
        }
        
        private void loadPlugin(string name, string pluginSpec, ref string[] cmdArgs, bool isLogger)
        {
            Debug.Assert(_communicator != null);

            //
            // Separate the entry point from the arguments. First
            // look for the :, then for the next whitespace. This
            // represents the end of the entry point.
            //
            // The remainder of the configuration line represents
            // the arguments.
            //
            string entryPoint = pluginSpec;
            string[] args = new string[0];
            int start = pluginSpec.IndexOf(':');
            if(start != -1)
            {
                //
                // Skip drive letter, if any.
                //
                if(pluginSpec.Length > 3 &&
                   start == 1 &&
                   System.Char.IsLetter(pluginSpec[0]) &&
                   (pluginSpec[2] == '\\' || pluginSpec[2] == '/'))
                {
                    start = pluginSpec.IndexOf(':', 3);
                }

                //
                // Find the whitespace.
                //
                int pos = pluginSpec.IndexOf(' ', start);
                if(pos == -1)
                {
                    pos = pluginSpec.IndexOf('\t', start);
                }
                if(pos == -1)
                {
                    pos = pluginSpec.IndexOf('\n', start);
                }
                if(pos != -1)
                {
                    entryPoint = pluginSpec.Substring(0, pos);
                    char[] delims = { ' ', '\t', '\n' };
                    args = pluginSpec.Substring(pos).Trim().Split(delims, pos);
                }
            }
            
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
            // Retrieve the assembly name and the type.
            //
            string err = "unable to load plugin '" + entryPoint + "': ";
            int sepPos = entryPoint.IndexOf(':');
            if (sepPos == -1)
            {
                PluginInitializationException e = new PluginInitializationException();
                e.reason = err + "invalid entry point format";
                throw e;
            }
            
            System.Reflection.Assembly pluginAssembly = null;
            string assemblyName = entryPoint.Substring(0, sepPos);
            try
            {
                if (System.IO.File.Exists(assemblyName))
                {
                    pluginAssembly = System.Reflection.Assembly.LoadFrom(assemblyName);
                }
                else
                {
                    pluginAssembly = System.Reflection.Assembly.Load(assemblyName);
                }
            }
            catch(System.Exception ex)
            {
                //
                // IceSSL is not supported with Mono 1.2. We avoid throwing an exception in that case,
                // so the same configuration can be used with Mono or Visual C#.
                //
                if(IceInternal.AssemblyUtil.runtime_ == IceInternal.AssemblyUtil.Runtime.Mono && name == "IceSSL")
                {
                    if(!_sslWarnOnce)
                    {
                        _communicator.getLogger().warning(
                            "IceSSL plugin not loaded: IceSSL is not supported with Mono");
                        _sslWarnOnce = true;
                    }
                    return;
                }

                PluginInitializationException e = new PluginInitializationException();
                e.reason = err + "unable to load assembly: '" + assemblyName + "': " + ex.ToString();
                throw e;
            }
            
            //
            // Instantiate the class.
            //
            PluginFactory pluginFactory = null;
            LoggerFactory loggerFactory = null;
            string className = entryPoint.Substring(sepPos + 1);
            System.Type c = pluginAssembly.GetType(className);
            if(c == null)
            {
                PluginInitializationException e = new PluginInitializationException();
                e.reason = err + "GetType failed for '" + className + "'";
                throw e;
            }

            try
            {
                if(isLogger)
                {
                    loggerFactory = (LoggerFactory)IceInternal.AssemblyUtil.createInstance(c);
                    if(loggerFactory == null)
                    {
                        PluginInitializationException e = new PluginInitializationException();
                        e.reason = err + "Can't find constructor for '" + className + "'";
                        throw e;
                    }
                }
                else
                {
                    pluginFactory = (PluginFactory)IceInternal.AssemblyUtil.createInstance(c);
                    if(pluginFactory == null)
                    {
                        PluginInitializationException e = new PluginInitializationException();
                        e.reason = err + "Can't find constructor for '" + className + "'";
                        throw e;
                    }
                }
            }
            catch(System.InvalidCastException ex)
            {
                PluginInitializationException e = new PluginInitializationException(ex);
                e.reason = err + "InvalidCastException to " + (isLogger ? "Ice.LoggerFactory" : "Ice.PluginFactory");
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
            
            //
            // Invoke the factory.
            //
            if(isLogger)
            {
                try
                {
                    _logger = loggerFactory.create(_communicator, args);
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
            
                if(_logger == null)
                {
                    PluginInitializationException ex = new PluginInitializationException();
                    ex.reason = err + "factory.create returned null logger";
                    throw ex;
                }
            }
            else
            {
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

                _plugins[name] = plugin;
                _initOrder.Add(plugin);
            }
        }

        public Logger
        getLogger()
        {
            return _logger;
        }
        
        private Communicator _communicator;
        private Hashtable _plugins;
        private ArrayList _initOrder;
        private Logger _logger = null;
        private bool _initialized;
        private static bool _sslWarnOnce = false;
    }
}
