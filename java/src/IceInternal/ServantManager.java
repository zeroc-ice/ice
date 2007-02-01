// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ServantManager
{
    public synchronized void
    addServant(Ice.Object servant, Ice.Identity ident, String facet)
    {
        assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.HashMap m = (java.util.HashMap)_servantMapMap.get(ident);
        if(m == null)
        {
            m = new java.util.HashMap();
            _servantMapMap.put(ident, m);
        }
        else
        {
            if(m.containsKey(facet))
            {
                Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                ex.id = _instance.identityToString(ident);
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

    public synchronized Ice.Object
    removeServant(Ice.Identity ident, String facet)
    {
        assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.HashMap m = (java.util.HashMap)_servantMapMap.get(ident);
        Ice.Object obj = null;
        if(m == null || (obj = (Ice.Object)m.remove(facet)) == null)
        {
            Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
            ex.id = _instance.identityToString(ident);
            ex.kindOfObject = "servant";
            if(facet.length() > 0)
            {
                ex.id += " -f " + IceUtil.StringUtil.escapeString(facet, "");
            }
            throw ex;
        }

        if(m.isEmpty())
        {
            _servantMapMap.remove(ident);
        }
        return obj;
    }

    public synchronized java.util.Map
    removeAllFacets(Ice.Identity ident)
    {
        assert(_instance != null); // Must not be called after destruction.

        java.util.HashMap m = (java.util.HashMap)_servantMapMap.get(ident);
        if(m == null)
        {
            Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
            ex.id = _instance.identityToString(ident);
            ex.kindOfObject = "servant";
            throw ex;
        }

        _servantMapMap.remove(ident);

        return m;
    }

    public synchronized Ice.Object
    findServant(Ice.Identity ident, String facet)
    {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        //assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.HashMap m = (java.util.HashMap)_servantMapMap.get(ident);
        Ice.Object obj = null;
        if(m != null)
        {
            obj = (Ice.Object)m.get(facet);
        }

        return obj;
    }

    public synchronized java.util.Map
    findAllFacets(Ice.Identity ident)
    {
        assert(_instance != null); // Must not be called after destruction.

        java.util.HashMap m = (java.util.HashMap)_servantMapMap.get(ident);
        if(m != null)
        {
            return new java.util.HashMap(m);
        }

        return new java.util.HashMap();
    }

    public synchronized boolean
    hasServant(Ice.Identity ident)
    {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        //assert(_instance != null); // Must not be called after destruction.

        java.util.HashMap m = (java.util.HashMap)_servantMapMap.get(ident);
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
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        //assert(_instance != null); // Must not be called after destruction.

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
        //IceUtil.Assert.FinalizerAssert(_instance == null);
        
        super.finalize();
    }

    //
    // Only for use by Ice.ObjectAdapterI.
    //
    public synchronized void
    destroy()
    {
        assert(_instance != null); // Must not be called after destruction.

        _servantMapMap.clear();
        
        java.util.Iterator p = _locatorMap.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry e = (java.util.Map.Entry)p.next();
            Ice.ServantLocator locator = (Ice.ServantLocator)e.getValue();
            try
            {
                locator.deactivate((String)e.getKey());
            }
            catch(java.lang.Exception ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "exception during locator deactivation:\n" + "object adapter: `" + _adapterName + "'\n" +
                    "locator category: `" + e.getKey() + "'\n" + sw.toString();
                _instance.initializationData().logger.error(s);
            }
        }

        _locatorMap.clear();

        _instance = null;
    }

    private Instance _instance;
    final private String _adapterName;
    private java.util.HashMap _servantMapMap = new java.util.HashMap();
    private java.util.HashMap _locatorMap = new java.util.HashMap();
}
