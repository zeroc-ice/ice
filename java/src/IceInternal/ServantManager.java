// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package IceInternal;

public final class ServantManager extends Thread
{
    public synchronized void
    addServant(Ice.Object servant, Ice.Identity ident)
    {
	assert(_instance != null); // Must not be called after destruction.
	
	Ice.Object o = (Ice.Object)_servantMap.get(ident);
	if(o != null)
	{
	    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
	    ex.id = Ice.Util.identityToString(ident);
	    ex.kindOfObject = "servant";
	    throw ex;
	}

        _servantMap.put(ident, servant);
    }

    public synchronized void
    removeServant(Ice.Identity ident)
    {
	assert(_instance != null); // Must not be called after destruction.
	
	Ice.Object o = (Ice.Object)_servantMap.get(ident);
	if(o == null)
	{
	    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
	    ex.id = Ice.Util.identityToString(ident);
	    ex.kindOfObject = "servant";
	    throw ex;
	}

        _servantMap.remove(ident);
    }

    public synchronized Ice.Object
    findServant(Ice.Identity ident)
    {
	assert(_instance != null); // Must not be called after destruction.

        return (Ice.Object)_servantMap.get(ident);
    }

    public synchronized void
    addServantLocator(Ice.ServantLocator locator, String prefix)
    {
	assert(_instance != null); // Must not be called after destruction.

	Ice.ServantLocator l = (Ice.ServantLocator)_locatorMap.get(prefix);
	if(l != null)
	{
	    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
	    ex.id = prefix;
	    ex.kindOfObject = "servant locator";
	    throw ex;
	}

        _locatorMap.put(prefix, locator);
    }

    public void
    removeServantLocator(String prefix)
    {
	Ice.ServantLocator locator;

	synchronized(this)
	{
	    assert(_instance != null); // Must not be called after destruction.
	    
	    locator = (Ice.ServantLocator)_locatorMap.remove(prefix);
	    if(locator == null)
	    {
		Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
		ex.id = prefix;
		ex.kindOfObject = "servant locator";
		throw ex;
	    }
	}

	locator.deactivate();
    }

    public synchronized Ice.ServantLocator
    findServantLocator(String prefix)
    {
	assert(_instance != null); // Must not be called after destruction.

        return (Ice.ServantLocator)_locatorMap.get(prefix);
    }

    //
    // Only for use by Ice.ObjectAdatperI.
    //
    public
    ServantManager(Instance instance, String adapterName)
    {
	_instance = instance;
	_adapterName = adapterName;
    }

    protected void
    finalize()
        throws Throwable
    {
	//
	// Don't check whether destroy() has been called. It might have
	// not been called if the associated object adapter was not
	// properly deactivated.
	//
	//assert(_instance == null);
	
        super.finalize();
    }

    //
    // Only for use by Ice.ObjectAdatperI.
    //
    public synchronized void
    destroy()
    {
	assert(_instance != null); // Must not be called after destruction.

	_servantMap.clear();
	
	java.util.Iterator p = _locatorMap.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry e = (java.util.Map.Entry)p.next();
	    Ice.ServantLocator locator = (Ice.ServantLocator)e.getValue();
	    try
	    {
		locator.deactivate();
	    }
	    catch(RuntimeException ex)
	    {
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		String s = "exception during locator deactivation:\n" + "object adapter: `" + _adapterName + "'\n" +
		    "locator prefix: `" + e.getKey() + "'\n" + sw.toString();
		_instance.logger().error(s);
	    }
	}

	_locatorMap.clear();

	_instance = null;
    }

    private Instance _instance;
    final private String _adapterName;
    private java.util.HashMap _servantMap = new java.util.HashMap();
    private java.util.HashMap _locatorMap = new java.util.HashMap();
}
