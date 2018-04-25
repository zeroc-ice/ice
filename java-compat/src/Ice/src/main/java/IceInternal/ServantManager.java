// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

        java.util.Map<String, Ice.Object> m = _servantMapMap.get(ident);
        if(m == null)
        {
            m = new java.util.HashMap<String, Ice.Object>();
            _servantMapMap.put(ident, m);
        }
        else
        {
            if(m.containsKey(facet))
            {
                Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                ex.id = Ice.Util.identityToString(ident, _instance.toStringMode());
                ex.kindOfObject = "servant";
                if(facet.length() > 0)
                {
                    ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "", _instance.toStringMode());
                }
                throw ex;
            }
        }

        m.put(facet, servant);
    }

    public synchronized void
    addDefaultServant(Ice.Object servant, String category)
    {
        assert(_instance != null); // Must not be called after destruction

        Ice.Object obj = _defaultServantMap.get(category);
        if(obj != null)
        {
            Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
            ex.kindOfObject = "default servant";
            ex.id = category;
            throw ex;
        }

        _defaultServantMap.put(category, servant);
    }

    public synchronized Ice.Object
    removeServant(Ice.Identity ident, String facet)
    {
        assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.Map<String, Ice.Object> m = _servantMapMap.get(ident);
        Ice.Object obj = null;
        if(m == null || (obj = m.remove(facet)) == null)
        {
            Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
            ex.id = Ice.Util.identityToString(ident, _instance.toStringMode());
            ex.kindOfObject = "servant";
            if(facet.length() > 0)
            {
                ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "", _instance.toStringMode());
            }
            throw ex;
        }

        if(m.isEmpty())
        {
            _servantMapMap.remove(ident);
        }
        return obj;
    }

    public synchronized Ice.Object
    removeDefaultServant(String category)
    {
        assert(_instance != null); // Must not be called after destruction.

        Ice.Object obj = _defaultServantMap.get(category);
        if(obj == null)
        {
            Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
            ex.kindOfObject = "default servant";
            ex.id = category;
            throw ex;
        }

        _defaultServantMap.remove(category);
        return obj;
    }

    public synchronized java.util.Map<String, Ice.Object>
    removeAllFacets(Ice.Identity ident)
    {
        assert(_instance != null); // Must not be called after destruction.

        java.util.Map<String, Ice.Object> m = _servantMapMap.get(ident);
        if(m == null)
        {
            Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
            ex.id = Ice.Util.identityToString(ident, _instance.toStringMode());
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

        java.util.Map<String, Ice.Object> m = _servantMapMap.get(ident);
        Ice.Object obj = null;
        if(m == null)
        {
            obj = _defaultServantMap.get(ident.category);
            if(obj == null)
            {
                obj = _defaultServantMap.get("");
            }
        }
        else
        {
            obj = m.get(facet);
        }

        return obj;
    }

    public synchronized Ice.Object
    findDefaultServant(String category)
    {
        assert(_instance != null); // Must not be called after destruction.

        return _defaultServantMap.get(category);
    }

    public synchronized java.util.Map<String, Ice.Object>
    findAllFacets(Ice.Identity ident)
    {
        assert(_instance != null); // Must not be called after destruction.

        java.util.Map<String, Ice.Object> m = _servantMapMap.get(ident);
        if(m != null)
        {
            return new java.util.HashMap<String, Ice.Object>(m);
        }

        return new java.util.HashMap<String, Ice.Object>();
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

        java.util.Map<String, Ice.Object> m = _servantMapMap.get(ident);
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

        Ice.ServantLocator l = _locatorMap.get(category);
        if(l != null)
        {
            Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
            ex.id = IceUtilInternal.StringUtil.escapeString(category, "", _instance.toStringMode());
            ex.kindOfObject = "servant locator";
            throw ex;
        }

        _locatorMap.put(category, locator);
    }

    public synchronized Ice.ServantLocator
    removeServantLocator(String category)
    {
        Ice.ServantLocator l = null;
        assert(_instance != null); // Must not be called after destruction.

        l = _locatorMap.remove(category);
        if(l == null)
        {
            Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
            ex.id = IceUtilInternal.StringUtil.escapeString(category, "", _instance.toStringMode());
            ex.kindOfObject = "servant locator";
            throw ex;
        }
        return l;
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

        return _locatorMap.get(category);
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

    //
    // Only for use by Ice.ObjectAdapterI.
    //
    public void
    destroy()
    {
        java.util.Map<String, Ice.ServantLocator> locatorMap = new java.util.HashMap<String, Ice.ServantLocator>();
        Ice.Logger logger = null;
        synchronized(this)
        {
            //
            // If the ServantManager has already been destroyed, we're done.
            //
            if(_instance == null)
            {
                return;
            }

            logger = _instance.initializationData().logger;

            _servantMapMap.clear();

            _defaultServantMap.clear();

            locatorMap.putAll(_locatorMap);
            _locatorMap.clear();

            _instance = null;
        }

        for(java.util.Map.Entry<String, Ice.ServantLocator> p : locatorMap.entrySet())
        {
            Ice.ServantLocator locator = p.getValue();
            try
            {
                locator.deactivate(p.getKey());
            }
            catch(java.lang.Exception ex)
            {
                String s = "exception during locator deactivation:\n" + "object adapter: `" + _adapterName + "'\n" +
                    "locator category: `" + p.getKey() + "'\n" + Ex.toString(ex);
                logger.error(s);
            }
        }
    }

    private Instance _instance;
    final private String _adapterName;
    private java.util.Map<Ice.Identity, java.util.Map<String, Ice.Object> > _servantMapMap =
        new java.util.HashMap<Ice.Identity, java.util.Map<String, Ice.Object> >();
    private java.util.Map<String, Ice.Object> _defaultServantMap = new java.util.HashMap<String, Ice.Object>();
    private java.util.Map<String, Ice.ServantLocator> _locatorMap = new java.util.HashMap<String, Ice.ServantLocator>();
}
