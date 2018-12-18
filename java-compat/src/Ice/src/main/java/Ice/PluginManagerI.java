// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

import java.net.URLEncoder;

public final class PluginManagerI implements PluginManager
{
    private static String _kindOfObject = "plugin";

    @Override
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
            for(PluginInfo p : _plugins)
            {
                try
                {
                    p.plugin.initialize();
                }
                catch(Ice.PluginInitializationException ex)
                {
                    throw ex;
                }
                catch(RuntimeException ex)
                {
                    PluginInitializationException e = new PluginInitializationException();
                    e.reason = "plugin `" + p.name + "' initialization failed";
                    e.initCause(ex);
                    throw e;
                }
                initializedPlugins.add(p.plugin);
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

    @Override
    public synchronized String[]
    getPlugins()
    {
        java.util.ArrayList<String> names = new java.util.ArrayList<String>();
        for(PluginInfo p : _plugins)
        {
            names.add(p.name);
        }
        return names.toArray(new String[0]);
    }

    @Override
    public synchronized Plugin
    getPlugin(String name)
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

    @Override
    public synchronized void
    addPlugin(String name, Plugin plugin)
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
        _plugins.add(info);
    }

    @Override
    public synchronized void
    destroy()
    {
        if(_communicator != null)
        {
            if(_initialized)
            {
                java.util.ListIterator<PluginInfo> i = _plugins.listIterator(_plugins.size());
                while(i.hasPrevious())
                {
                    PluginInfo p = i.previous();
                    try
                    {
                        p.plugin.destroy();
                    }
                    catch(RuntimeException ex)
                    {
                        Ice.Util.getProcessLogger().warning("unexpected exception raised by plug-in `" +
                                                            p.name + "' destruction:\n" + ex.toString());
                    }
                }
            }

            _communicator = null;
        }

        _plugins.clear();

        if(_classLoaders != null)
        {
            _classLoaders.clear();
        }
    }

    public
    PluginManagerI(Communicator communicator, IceInternal.Instance instance)
    {
        _communicator = communicator;
        _instance = instance;
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
            if(findPlugin(name) != null)
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
        // We support the following formats:
        //
        // <class-name> [args]
        // <jar-file>:<class-name> [args]
        // <class-dir>:<class-name> [args]
        // "<path with spaces>":<class-name> [args]
        // "<path with spaces>:<class-name>" [args]
        //

        String[] args;
        try
        {
            args = IceUtilInternal.Options.split(pluginSpec);
        }
        catch(IceUtilInternal.Options.BadQuote ex)
        {
            throw new PluginInitializationException("invalid arguments for plug-in `" + name + "':\n" +
                                                    ex.getMessage());
        }

        assert(args.length > 0);

        final String entryPoint = args[0];

        final boolean isWindows = System.getProperty("os.name").startsWith("Windows");
        boolean absolutePath = false;

        //
        // Find first ':' that isn't part of the file path.
        //
        int pos = entryPoint.indexOf(':');
        if(isWindows)
        {
            final String driveLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            if(pos == 1 && entryPoint.length() > 2 && driveLetters.indexOf(entryPoint.charAt(0)) != -1 &&
               (entryPoint.charAt(2) == '\\' || entryPoint.charAt(2) == '/'))
            {
                absolutePath = true;
                pos = entryPoint.indexOf(':', pos + 1);
            }
            if(!absolutePath)
            {
                absolutePath = entryPoint.startsWith("\\\\");
            }
        }
        else
        {
            absolutePath = entryPoint.startsWith("/");
        }

        if((pos == -1 && absolutePath) || (pos != -1 && entryPoint.length() <= pos + 1))
        {
            //
            // Class name is missing.
            //
            throw new PluginInitializationException("invalid entry point for plug-in `" + name + "':\n" + entryPoint);
        }

        //
        // Extract the JAR file or subdirectory, if any.
        //
        String classDir = null; // Path name of JAR file or subdirectory.
        String className;

        if(pos == -1)
        {
            className = entryPoint;
        }
        else
        {
            classDir = entryPoint.substring(0, pos).trim();
            className = entryPoint.substring(pos + 1).trim();
        }

        //
        // Shift the arguments.
        //
        String[] tmp = new String[args.length - 1];
        System.arraycopy(args, 1, tmp, 0, args.length - 1);
        args = tmp;

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
            Class<?> c = null;

            //
            // Use a class loader if the user specified a JAR file or class directory.
            //
            if(classDir != null)
            {
                try
                {
                    if(!absolutePath)
                    {
                        classDir = new java.io.File(System.getProperty("user.dir") + java.io.File.separator +
                            classDir).getCanonicalPath();
                    }

                    if(!classDir.endsWith(java.io.File.separator) && !classDir.toLowerCase().endsWith(".jar"))
                    {
                        classDir += java.io.File.separator;
                    }
                    classDir = URLEncoder.encode(classDir, "UTF-8");

                    //
                    // Reuse an existing class loader if we have already loaded a plug-in with
                    // the same value for classDir, otherwise create a new one.
                    //
                    ClassLoader cl = null;

                    if(_classLoaders == null)
                    {
                        _classLoaders = new java.util.HashMap<String, ClassLoader>();
                    }
                    else
                    {
                        cl = _classLoaders.get(classDir);
                    }

                    if(cl == null)
                    {
                        final java.net.URL[] url = new java.net.URL[] { new java.net.URL("file", null, classDir) };

                        //
                        // Use the custom class loader (if any) as the parent.
                        //
                        if(_instance.initializationData().classLoader != null)
                        {
                            cl = new java.net.URLClassLoader(url, _instance.initializationData().classLoader);
                        }
                        else
                        {
                            cl = new java.net.URLClassLoader(url);
                        }

                        _classLoaders.put(classDir, cl);
                    }

                    c = cl.loadClass(className);
                }
                catch(java.net.MalformedURLException ex)
                {
                    throw new PluginInitializationException("invalid entry point format `" + pluginSpec + "'", ex);
                }
                catch(java.io.IOException ex)
                {
                    throw new PluginInitializationException("invalid path in entry point `" + pluginSpec + "'", ex);
                }
                catch(java.lang.ClassNotFoundException ex)
                {
                    // Ignored
                }
            }
            else
            {
                c = IceInternal.Util.getInstance(_communicator).findClass(className);
            }

            if(c == null)
            {
                throw new PluginInitializationException("class " + className + " not found");
            }

            java.lang.Object obj = c.getDeclaredConstructor().newInstance();
            try
            {
                pluginFactory = (PluginFactory)obj;
            }
            catch(ClassCastException ex)
            {
                throw new PluginInitializationException("class " + className + " does not implement Ice.PluginFactory",
                                                        ex);
            }
        }
        catch(NoSuchMethodException ex)
        {
            throw new PluginInitializationException("unable to instantiate class " + className, ex);
        }
        catch(java.lang.reflect.InvocationTargetException ex)
        {
            throw new PluginInitializationException("unable to instantiate class " + className, ex);
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

        PluginInfo info = new PluginInfo();
        info.name = name;
        info.plugin = plugin;
        _plugins.add(info);
    }

    private Plugin
    findPlugin(String name)
    {
        for(PluginInfo p : _plugins)
        {
            if(name.equals(p.name))
            {
                return p.plugin;
            }
        }
        return null;
    }

    static class PluginInfo
    {
        String name;
        Plugin plugin;
    }

    private Communicator _communicator;
    private IceInternal.Instance _instance;
    private java.util.List<PluginInfo> _plugins = new java.util.ArrayList<PluginInfo>();
    private boolean _initialized;
    private java.util.Map<String, ClassLoader> _classLoaders;
}
