// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public final class PluginManagerI implements PluginManager
{
    public synchronized Plugin
    getPlugin(String name)
    {
        Plugin p = (Plugin)_plugins.get(name);
        if (p != null)
        {
            return p;
        }
        throw new PluginNotFoundException();
    }

    public synchronized void
    addPlugin(String name, Plugin plugin)
    {
        if (_plugins.containsKey(name))
        {
            throw new PluginExistsException();
        }
        _plugins.put(name, plugin);
    }

    public synchronized void
    destroy()
    {
        java.util.Iterator i = _plugins.entrySet().iterator();
        while (i.hasNext())
        {
            Plugin p = (Plugin)i.next();
            p.destroy();
        }
    }

    public
    PluginManagerI(IceInternal.Instance instance)
    {
        _instance = instance;
    }

    public void
    loadPlugins(StringSeqHolder cmdArgs)
    {
        //
        // Load and initialize the plug-ins defined in the property set
        // with the prefix "Ice.Plugin.". These properties should
        // have the following format:
        //
        // Ice.Plugin.name=entry_point [args]
        //
        final String prefix = "Ice.Plugin.";
        Ice.Properties properties = _instance.properties();
        String[] plugins = properties.getProperties(prefix);
        for (int i = 0; i < plugins.length; i += 2)
        {
            String name = plugins[i].substring(prefix.length());
            String value = plugins[i + 1];

            //
            // Separate the entry point from the arguments.
            //
            String className;
            String[] args;
            int pos = value.indexOf(' ');
            if (pos == -1)
            {
                pos = value.indexOf('\t');
            }
            if (pos == -1)
            {
                pos = value.indexOf('\n');
            }
            if (pos == -1)
            {
                className = value;
                args = new String[0];
            }
            else
            {
                className = value.substring(0, pos);
                args = value.substring(pos).trim().split("[ \t\n]+", pos);
            }

            //
            // Convert command-line options into properties. First we
            // convert the options from the plug-in configuration, then
            // we convert the options from the application command-line.
            //
            args = properties.parseCommandLineOptions(name, args);
            cmdArgs.value = properties.parseCommandLineOptions(name, cmdArgs.value);

            loadPlugin(name, className, args);
        }
    }

    private void
    loadPlugin(String name, String className, String[] args)
    {
        //
        // Instantiate the class.
        //
        PluginFactory factory = null;
        try
        {
            Class c = Class.forName(className);
            java.lang.Object obj = c.newInstance();
            try
            {
                factory = (PluginFactory)obj;
            }
            catch (ClassCastException ex)
            {
                _instance.logger().error("PluginManager: class " + className +
                                         " does not implement Ice.PluginFactory");
                throw new SystemException();
            }
        }
        catch (ClassNotFoundException ex)
        {
            _instance.logger().error("PluginManager: class " + className + " not found");
            throw new SystemException();
        }
        catch (IllegalAccessException ex)
        {
            _instance.logger().error("PluginManager: unable to access default constructor in class " + className);
            throw new SystemException();
        }
        catch (InstantiationException ex)
        {
            _instance.logger().error("PluginManager: unable to instantiate class " + className);
            throw new SystemException();
        }

        //
        // Invoke the factory.
        //
        Plugin plugin = null;
        try
        {
            plugin = factory.create(_instance.communicator(), name, args);
        }
        catch (Exception ex)
        {
            _instance.logger().error("PluginManager: exception in factory " + className);
            SystemException e = new SystemException();
            e.initCause(ex);
            throw e;
        }

        _plugins.put(name, plugin);
    }

    private IceInternal.Instance _instance;
    private java.util.HashMap _plugins = new java.util.HashMap();
}
