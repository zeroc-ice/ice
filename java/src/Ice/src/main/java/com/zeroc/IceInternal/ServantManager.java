// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public final class ServantManager
{
    public synchronized void
    addServant(com.zeroc.Ice.Object servant, com.zeroc.Ice.Identity ident, String facet)
    {
        assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.Map<String, com.zeroc.Ice.Object> m = _servantMapMap.get(ident);
        if(m == null)
        {
            m = new java.util.HashMap<String, com.zeroc.Ice.Object>();
            _servantMapMap.put(ident, m);
        }
        else
        {
            if(m.containsKey(facet))
            {
                com.zeroc.Ice.AlreadyRegisteredException ex = new com.zeroc.Ice.AlreadyRegisteredException();
                ex.id = com.zeroc.Ice.Util.identityToString(ident, _instance.toStringMode());
                ex.kindOfObject = "servant";
                if(facet.length() > 0)
                {
                    ex.id += " -f " + com.zeroc.IceUtilInternal.StringUtil.escapeString(facet, "",
                                                                                        _instance.toStringMode());
                }
                throw ex;
            }
        }

        m.put(facet, servant);
    }

    public synchronized void
    addDefaultServant(com.zeroc.Ice.Object servant, String category)
    {
        assert(_instance != null); // Must not be called after destruction

        com.zeroc.Ice.Object obj = _defaultServantMap.get(category);
        if(obj != null)
        {
            com.zeroc.Ice.AlreadyRegisteredException ex = new com.zeroc.Ice.AlreadyRegisteredException();
            ex.kindOfObject = "default servant";
            ex.id = category;
            throw ex;
        }

        _defaultServantMap.put(category, servant);
    }

    public synchronized com.zeroc.Ice.Object
    removeServant(com.zeroc.Ice.Identity ident, String facet)
    {
        assert(_instance != null); // Must not be called after destruction.

        if(facet == null)
        {
            facet = "";
        }

        java.util.Map<String, com.zeroc.Ice.Object> m = _servantMapMap.get(ident);
        com.zeroc.Ice.Object obj = null;
        if(m == null || (obj = m.remove(facet)) == null)
        {
            com.zeroc.Ice.NotRegisteredException ex = new com.zeroc.Ice.NotRegisteredException();
            ex.id = com.zeroc.Ice.Util.identityToString(ident, _instance.toStringMode());
            ex.kindOfObject = "servant";
            if(facet.length() > 0)
            {
                ex.id += " -f " + com.zeroc.IceUtilInternal.StringUtil.escapeString(facet, "",
                                                                                    _instance.toStringMode());
            }
            throw ex;
        }

        if(m.isEmpty())
        {
            _servantMapMap.remove(ident);
        }
        return obj;
    }

    public synchronized com.zeroc.Ice.Object
    removeDefaultServant(String category)
    {
        assert(_instance != null); // Must not be called after destruction.

        com.zeroc.Ice.Object obj = _defaultServantMap.get(category);
        if(obj == null)
        {
            com.zeroc.Ice.NotRegisteredException ex = new com.zeroc.Ice.NotRegisteredException();
            ex.kindOfObject = "default servant";
            ex.id = category;
            throw ex;
        }

        _defaultServantMap.remove(category);
        return obj;
    }

    public synchronized java.util.Map<String, com.zeroc.Ice.Object>
    removeAllFacets(com.zeroc.Ice.Identity ident)
    {
        assert(_instance != null); // Must not be called after destruction.

        java.util.Map<String, com.zeroc.Ice.Object> m = _servantMapMap.get(ident);
        if(m == null)
        {
            com.zeroc.Ice.NotRegisteredException ex = new com.zeroc.Ice.NotRegisteredException();
            ex.id = com.zeroc.Ice.Util.identityToString(ident, _instance.toStringMode());
            ex.kindOfObject = "servant";
            throw ex;
        }

        _servantMapMap.remove(ident);

        return m;
    }

    public synchronized com.zeroc.Ice.Object
    findServant(com.zeroc.Ice.Identity ident, String facet)
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

        java.util.Map<String, com.zeroc.Ice.Object> m = _servantMapMap.get(ident);
        com.zeroc.Ice.Object obj = null;
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

    public synchronized com.zeroc.Ice.Object
    findDefaultServant(String category)
    {
        assert(_instance != null); // Must not be called after destruction.

        return _defaultServantMap.get(category);
    }

    public synchronized java.util.Map<String, com.zeroc.Ice.Object>
    findAllFacets(com.zeroc.Ice.Identity ident)
    {
        assert(_instance != null); // Must not be called after destruction.

        java.util.Map<String, com.zeroc.Ice.Object> m = _servantMapMap.get(ident);
        if(m != null)
        {
            return new java.util.HashMap<String, com.zeroc.Ice.Object>(m);
        }

        return new java.util.HashMap<String, com.zeroc.Ice.Object>();
    }

    public synchronized boolean
    hasServant(com.zeroc.Ice.Identity ident)
    {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        //assert(_instance != null); // Must not be called after destruction.

        java.util.Map<String, com.zeroc.Ice.Object> m = _servantMapMap.get(ident);
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
    addServantLocator(com.zeroc.Ice.ServantLocator locator, String category)
    {
        assert(_instance != null); // Must not be called after destruction.

        com.zeroc.Ice.ServantLocator l = _locatorMap.get(category);
        if(l != null)
        {
            com.zeroc.Ice.AlreadyRegisteredException ex = new com.zeroc.Ice.AlreadyRegisteredException();
            ex.id = com.zeroc.IceUtilInternal.StringUtil.escapeString(category, "",
                                                                      _instance.toStringMode());
            ex.kindOfObject = "servant locator";
            throw ex;
        }

        _locatorMap.put(category, locator);
    }

    public synchronized com.zeroc.Ice.ServantLocator
    removeServantLocator(String category)
    {
        com.zeroc.Ice.ServantLocator l = null;
        assert(_instance != null); // Must not be called after destruction.

        l = _locatorMap.remove(category);
        if(l == null)
        {
            com.zeroc.Ice.NotRegisteredException ex = new com.zeroc.Ice.NotRegisteredException();
            ex.id = com.zeroc.IceUtilInternal.StringUtil.escapeString(category, "",
                                                                      _instance.toStringMode());
            ex.kindOfObject = "servant locator";
            throw ex;
        }
        return l;
    }

    public synchronized com.zeroc.Ice.ServantLocator
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
    // Only for use by com.zeroc.Ice.ObjectAdatperI.
    //
    public
    ServantManager(Instance instance, String adapterName)
    {
        _instance = instance;
        _adapterName = adapterName;
    }

    //
    // Only for use by com.zeroc.Ice.ObjectAdapterI.
    //
    public void
    destroy()
    {
        java.util.Map<String, com.zeroc.Ice.ServantLocator> locatorMap =
            new java.util.HashMap<String, com.zeroc.Ice.ServantLocator>();
        com.zeroc.Ice.Logger logger = null;
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

        for(java.util.Map.Entry<String, com.zeroc.Ice.ServantLocator> p : locatorMap.entrySet())
        {
            com.zeroc.Ice.ServantLocator locator = p.getValue();
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
    private java.util.Map<com.zeroc.Ice.Identity, java.util.Map<String, com.zeroc.Ice.Object> > _servantMapMap =
        new java.util.HashMap<com.zeroc.Ice.Identity, java.util.Map<String, com.zeroc.Ice.Object> >();
    private java.util.Map<String, com.zeroc.Ice.Object> _defaultServantMap =
        new java.util.HashMap<String, com.zeroc.Ice.Object>();
    private java.util.Map<String, com.zeroc.Ice.ServantLocator> _locatorMap =
        new java.util.HashMap<String, com.zeroc.Ice.ServantLocator>();
}
