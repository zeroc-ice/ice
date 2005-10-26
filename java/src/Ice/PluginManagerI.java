// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class PluginManagerI extends LocalObjectImpl implements PluginManager
{
    private static String _kindOfObject = "plug-in";

    public synchronized Plugin
    getPlugin(String name)
    {
	if(_communicator == null)
	{
	    throw new CommunicatorDestroyedException();
	}

        Plugin p = (Plugin)_plugins.get(name);
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
	    java.util.Iterator i = _plugins.values().iterator();
	    while(i.hasNext())
	    {
		Plugin p = (Plugin)i.next();
		p.destroy();
	    }

	    _communicator = null;
	}
    }

    public
    PluginManagerI(Communicator communicator)
    {
        _communicator = communicator;
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
        // Ice.Plugin.name=entry_point [args]
        //
        final String prefix = "Ice.Plugin.";
        Ice.Properties properties = _communicator.getProperties();
        java.util.Map plugins = properties.getPropertiesForPrefix(prefix);
        java.util.Iterator p = plugins.entrySet().iterator();
        while(p.hasNext())
        {
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String name = ((String)entry.getKey()).substring(prefix.length());
            String value = (String)entry.getValue();

            //
            // Separate the entry point from the arguments.
            //
            String className;
            String[] args;
            int pos = value.indexOf(' ');
            if(pos == -1)
            {
                pos = value.indexOf('\t');
            }
            if(pos == -1)
            {
                pos = value.indexOf('\n');
            }
            if(pos == -1)
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
	assert(_communicator != null);

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
            catch(ClassCastException ex)
            {
                PluginInitializationException e = new PluginInitializationException();
                e.reason = "class " + className + " does not implement Ice.PluginFactory";
                e.initCause(ex);
                throw e;
            }
        }
        catch(ClassNotFoundException ex)
        {
            PluginInitializationException e = new PluginInitializationException();
            e.reason = "class " + className + " not found";
            e.initCause(ex);
            throw e;
        }
        catch(IllegalAccessException ex)
        {
            PluginInitializationException e = new PluginInitializationException();
            e.reason = "unable to access default constructor in class " + className;
            e.initCause(ex);
            throw e;
        }
        catch(InstantiationException ex)
        {
            PluginInitializationException e = new PluginInitializationException();
            e.reason = "unable to instantiate class " + className;
            e.initCause(ex);
            throw e;
        }

        //
        // Invoke the factory.
        //
        Plugin plugin = null;
        try
        {
            plugin = factory.create(_communicator, name, args);
        }
        catch(PluginInitializationException ex)
        {
            throw ex;
        }
        catch(Throwable ex)
        {
            PluginInitializationException e = new PluginInitializationException();
            e.reason = "exception in factory " + className;
            e.initCause(ex);
            throw e;
        }

	if(plugin == null)
	{
            PluginInitializationException e = new PluginInitializationException();
            e.reason = "failure in factory " + className;
            throw e;
	}

        _plugins.put(name, plugin);
    }

    private Communicator _communicator;
    private java.util.HashMap _plugins = new java.util.HashMap();
}
