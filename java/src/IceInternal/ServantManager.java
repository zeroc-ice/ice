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
    addServant(Ice.Object servant, Ice.Identity ident, String facet)
    {
	assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.HashMap m = (java.util.HashMap)_servantMap.get(ident);
        if(m == null)
        {
            m = new java.util.HashMap();
            _servantMap.put(ident, m);
        }
        else
        {
            if(m.containsKey(facet))
            {
                Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                ex.id = Ice.Util.identityToString(ident);
                ex.kindOfObject = "servant";
                if(facet.length() > 0)
                {
                    ex.id += " -f " + IceUtil.StringUtil.escapeString(facet, "");
                }
                throw ex;
            }
        }

        m.put(facet, servant);
    }

    public synchronized void
    removeServant(Ice.Identity ident, String facet)
    {
	assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.HashMap m = (java.util.HashMap)_servantMap.get(ident);
        Ice.Object obj = null;
        if(m == null || (obj = (Ice.Object)m.get(facet)) == null)
	{
	    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
	    ex.id = Ice.Util.identityToString(ident);
	    ex.kindOfObject = "servant";
            if(facet.length() > 0)
            {
                ex.id += " -f " + IceUtil.StringUtil.escapeString(facet, "");
            }
	    throw ex;
	}

        m.remove(facet);
        if(m.isEmpty())
        {
            _servantMap.remove(ident);
        }
    }

    public synchronized Ice.Object
    findServant(Ice.Identity ident, String facet)
    {
	assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.HashMap m = (java.util.HashMap)_servantMap.get(ident);
        Ice.Object obj = null;
        if(m != null)
        {
            obj = (Ice.Object)m.get(facet);
        }

        return obj;
    }

    public synchronized boolean
    hasServant(Ice.Identity ident)
    {
	assert(_instance != null); // Must not be called after destruction.

        java.util.HashMap m = (java.util.HashMap)_servantMap.get(ident);
        if(m == null)
        {
            return false;
        }
        else
        {
            assert(!m.isEmpty());
            return true;
        }
    }

    public synchronized void
    addServantLocator(Ice.ServantLocator locator, String category)
    {
	assert(_instance != null); // Must not be called after destruction.

	Ice.ServantLocator l = (Ice.ServantLocator)_locatorMap.get(category);
	if(l != null)
	{
	    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
	    ex.id = IceUtil.StringUtil.escapeString(category, "");
	    ex.kindOfObject = "servant locator";
	    throw ex;
	}

        _locatorMap.put(category, locator);
    }

    public synchronized Ice.ServantLocator
    findServantLocator(String category)
    {
	assert(_instance != null); // Must not be called after destruction.

        return (Ice.ServantLocator)_locatorMap.get(category);
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
		locator.deactivate((String)e.getKey());
	    }
	    catch(Exception ex)
	    {
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		String s = "exception during locator deactivation:\n" + "object adapter: `" + _adapterName + "'\n" +
		    "locator category: `" + e.getKey() + "'\n" + sw.toString();
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
