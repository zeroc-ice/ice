// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    public sealed class PluginManagerI : LocalObjectImpl, PluginManager
    {
	private static string _kindOfObject = "plug-in";

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
		
		    _communicator = null;
		}
	    }
	}
	
	public PluginManagerI(Communicator communicator)
	{
	    _communicator = communicator;
	    _plugins = new Hashtable();
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
	    string prefix = "Ice.Plugin.";
	    Ice.Properties properties = _communicator.getProperties();
	    PropertyDict plugins = properties.getPropertiesForPrefix(prefix);
	    foreach(DictionaryEntry entry in plugins)
	    {
		string name = ((string)entry.Key).Substring(prefix.Length);
		string val = (string)entry.Value;
		
		//
		// Separate the entry point from the arguments.
		//
		string className;
		string[] args;
		int pos = val.IndexOf(' ');
		if(pos == -1)
		{
		    pos = val.IndexOf('\t');
		}
		if(pos == -1)
		{
		    pos = val.IndexOf('\n');
		}
		if(pos == -1)
		{
		    className = val;
		    args = new string[0];
		}
		else
		{
		    className = val.Substring(0, pos);
		    char[] delims = { ' ', '\t', '\n' };
		    args = val.Substring(pos).Trim().Split(delims, pos);
		}
		
		//
		// Convert command-line options into properties. First we
		// convert the options from the plug-in configuration, then
		// we convert the options from the application command-line.
		//
		args = properties.parseCommandLineOptions(name, args);
		cmdArgs = properties.parseCommandLineOptions(name, cmdArgs);
		
		loadPlugin(name, className, args);
	    }
	}
	
	private void loadPlugin(string name, string className, string[] args)
	{
	    Debug.Assert(_communicator != null);
	    
	    //
	    // Instantiate the class.
	    //
	    PluginFactory factory = null;
            System.Type c = System.Type.GetType(className);
            if(c == null)
            {
                PluginInitializationException e = new PluginInitializationException();
                e.reason = "cannot locate metadata for class " + className;
                throw e;
            }
	    try
	    {

                System.Object obj = IceInternal.AssemblyUtil.createInstance(c);
		try
		{
		    factory = (PluginFactory) obj;
		}
		catch(System.InvalidCastException ex)
		{
		    PluginInitializationException e = new PluginInitializationException(ex);
		    e.reason = "class " + className + " does not implement Ice.PluginFactory";
		    throw e;
		}
	    }
	    catch(System.UnauthorizedAccessException ex)
	    {
		PluginInitializationException e = new PluginInitializationException(ex);
		e.reason = "unable to access default constructor in class " + className;
		throw e;
	    }
	    catch(System.Exception ex)
	    {
		PluginInitializationException e = new PluginInitializationException(ex);
		e.reason = "unable to instantiate class " + className;
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
	    catch(System.Exception ex)
	    {
		PluginInitializationException e = new PluginInitializationException(ex);
		e.reason = "exception in factory " + className;
		throw e;
	    }
	    
	    if(plugin == null)
	    {
		PluginInitializationException ex = new PluginInitializationException();
		ex.reason = "failure in factory " + className;
		throw ex;
	    }

	    _plugins[name] = plugin;
	}
	
	private Communicator _communicator;
	private Hashtable _plugins;
    }

}
