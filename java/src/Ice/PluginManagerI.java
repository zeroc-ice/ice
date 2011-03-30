// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class PluginManagerI implements PluginManager
{
    private static String _kindOfObject = "plugin";

    public synchronized void
    initializePlugins()
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
        java.util.List<Plugin> initializedPlugins = new java.util.ArrayList<Plugin>();
        try
        {
            for(Plugin p : _initOrder)
            {
                p.initialize();
                initializedPlugins.add(p);
            }
        }
        catch(RuntimeException ex)
        {
            //
            // Destroy the plug-ins that have been successfully initialized, in the
            // reverse order.
            //
            java.util.ListIterator<Plugin> i = initializedPlugins.listIterator(initializedPlugins.size());
            while(i.hasPrevious())
            {
                Plugin p = i.previous();
                try
                {
                    p.destroy();
                }
                catch(RuntimeException e)
                {
                    // Ignore.
                }
            }
            throw ex;
        }

        _initialized = true;
    }

    public synchronized String[]
    getPlugins()
    {
        java.util.ArrayList<String> names = new java.util.ArrayList<String>();
        for(java.util.Map.Entry<String, Plugin> p : _plugins.entrySet())
        {
            names.add(p.getKey());
        }
        return names.toArray(new String[0]);
    }

    public synchronized Plugin
    getPlugin(String name)
    {
        if(_communicator == null)
        {
            throw new CommunicatorDestroyedException();
        }

        Plugin p = _plugins.get(name);
        if(p != null)
        {
            return p;
        }
        NotRegisteredException ex = new NotRegisteredException();
        ex.id = name;
        ex.kindOfObject = _kindOfObject;
        throw ex;
    }

    public synchronized void
    addPlugin(String name, Plugin plugin)
    {
        if(_communicator == null)
        {
            throw new CommunicatorDestroyedException();
        }

        if(_plugins.containsKey(name))
        {
            AlreadyRegisteredException ex = new AlreadyRegisteredException();
            ex.id = name;
            ex.kindOfObject = _kindOfObject;
            throw ex;
        }
        _plugins.put(name, plugin);
    }

    public synchronized void
    destroy()
    {
        if(_communicator != null)
        {
            if(_initialized)
            {
                for(java.util.Map.Entry<String, Plugin> p : _plugins.entrySet())
                {
                    try
                    {
                        p.getValue().destroy();
                    }
                    catch(RuntimeException ex)
                    {
                        Ice.Util.getProcessLogger().warning("unexpected exception raised by plug-in `" + p.getKey() +
                                                            "' destruction:\n" + ex.toString());
                    }
                }
            }

            _communicator = null;
        }
    }

    public
    PluginManagerI(Communicator communicator)
    {
        _communicator = communicator;
        _initialized = false;
    }

    public void
    loadPlugins(StringSeqHolder cmdArgs)
    {
        assert(_communicator != null);

        //
        // Load and initialize the plug-ins defined in the property set
        // with the prefix "Ice.Plugin.". These properties should
        // have the following format:
        //
        // Ice.Plugin.name[.<language>]=entry_point [args]
        //
        // If the Ice.PluginLoadOrder property is defined, load the
        // specified plug-ins in the specified order, then load any
        // remaining plug-ins.
        //
        final String prefix = "Ice.Plugin.";
        Properties properties = _communicator.getProperties();
        java.util.Map<String, String> plugins = properties.getPropertiesForPrefix(prefix);

        final String[] loadOrder = properties.getPropertyAsList("Ice.PluginLoadOrder");
        for(String name : loadOrder)
        {
            if(_plugins.containsKey(name))
            {
                PluginInitializationException ex = new PluginInitializationException();
                ex.reason = "plug-in `" + name + "' already loaded";
                throw ex;
            }

            String key = "Ice.Plugin." + name + ".java";
            boolean hasKey = plugins.containsKey(key);
            if(hasKey)
            {
                plugins.remove("Ice.Plugin." + name);
            }
            else
            {
                key = "Ice.Plugin." + name;
                hasKey = plugins.containsKey(key);
            }
            
            if(hasKey)
            {
                final String value = plugins.get(key);
                loadPlugin(name, value, cmdArgs);
                plugins.remove(key);
            }
            else
            {
                PluginInitializationException ex = new PluginInitializationException();
                ex.reason = "plug-in `" + name + "' not defined";
                throw ex;
            }
        }

        //
        // Load any remaining plug-ins that weren't specified in PluginLoadOrder.
        //
        while(!plugins.isEmpty())
        {
            java.util.Iterator<java.util.Map.Entry<String, String> > p = plugins.entrySet().iterator();
            java.util.Map.Entry<String, String> entry = p.next();

            String name = entry.getKey().substring(prefix.length());

            int dotPos = name.lastIndexOf('.');
            if(dotPos != -1)
            {
                String suffix = name.substring(dotPos + 1);
                if(suffix.equals("cpp") || suffix.equals("clr"))
                {
                    //
                    // Ignored
                    //
                    p.remove();
                }
                else if(suffix.equals("java"))
                {
                    name = name.substring(0, dotPos);
                    loadPlugin(name, entry.getValue(), cmdArgs);
                    p.remove();
                    
                    //
                    // Don't want to load this one if it's there!
                    // 
                    plugins.remove("Ice.Plugin." + name);
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
                //
                // Is there a .java entry?
                //
                String value = entry.getValue();
                p.remove();

                String javaValue = plugins.remove("Ice.Plugin." + name + ".java");
                if(javaValue != null)
                {
                    value = javaValue;
                }
                
                loadPlugin(name, value, cmdArgs);
            }
        }
    }

    private void
    loadPlugin(String name, String pluginSpec, StringSeqHolder cmdArgs)
    {
        assert(_communicator != null);

        //
        // Separate the entry point from the arguments.
        //
        String className;
        String[] args;
        int pos = pluginSpec.indexOf(' ');
        if(pos == -1)
        {
            pos = pluginSpec.indexOf('\t');
        }
        if(pos == -1)
        {
            pos = pluginSpec.indexOf('\n');
        }
        if(pos == -1)
        {
            className = pluginSpec;
            args = new String[0];
        }
        else
        {
            className = pluginSpec.substring(0, pos);
            args = pluginSpec.substring(pos).trim().split("[ \t\n]+", pos);
        }

        //
        // Convert command-line options into properties. First we
        // convert the options from the plug-in configuration, then
        // we convert the options from the application command-line.
        //
        Properties properties = _communicator.getProperties();
        args = properties.parseCommandLineOptions(name, args);
        cmdArgs.value = properties.parseCommandLineOptions(name, cmdArgs.value);

        //
        // Instantiate the class.
        //
        PluginFactory pluginFactory = null;
        try
        {
            Class<?> c = IceInternal.Util.getInstance(_communicator).findClass(className);
            if(c == null)
            {
                PluginInitializationException e = new PluginInitializationException();
                e.reason = "class " + className + " not found";
                throw e;
            }
            java.lang.Object obj = c.newInstance();
            try
            {
                pluginFactory = (PluginFactory)obj;
            }
            catch(ClassCastException ex)
            {
                throw new PluginInitializationException(
                    "class " + className + " does not implement Ice.PluginFactory", ex);
            }
        }
        catch(IllegalAccessException ex)
        {
            throw new PluginInitializationException("unable to access default constructor in class " + className, ex);
        }
        catch(InstantiationException ex)
        {
            throw new PluginInitializationException("unable to instantiate class " + className, ex);
        }

        //
        // Invoke the factory.
        //
        Plugin plugin = null;
        try
        {
            plugin = pluginFactory.create(_communicator, name, args);
        }
        catch(PluginInitializationException ex)
        {
            throw ex;
        }
        catch(Throwable ex)
        {
            throw new PluginInitializationException("exception in factory " + className, ex);
        }
   
        if(plugin == null)
        {
            throw new PluginInitializationException("failure in factory " + className);
        }

        _plugins.put(name, plugin);
        _initOrder.add(plugin);
    }

    private Communicator _communicator;
    private java.util.Map<String, Plugin> _plugins = new java.util.HashMap<String, Plugin>();
    private java.util.List<Plugin> _initOrder = new java.util.ArrayList<Plugin>();
    private boolean _initialized;
}
