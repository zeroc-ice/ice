// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
	    ex.id = Ice.Util.identityToString(ident);
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
	    ex.id = Ice.Util.identityToString(ident);
	    ex.kindOfObject = "servant";
	    throw ex;
	}

	_servantMapMap.remove(ident);

        return m;
    }

    public synchronized Ice.Object
    findServant(Ice.Identity ident, String facet)
    {
	assert(_instance != null); // Must not be called after destruction.

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
	assert(_instance != null); // Must not be called after destruction.

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
    // Only for use by Ice.ObjectAdapterI.
    //
    public synchronized void
    destroy()
    {
	assert(_instance != null); // Must not be called after destruction.

	_servantMapMap.clear();
	_instance = null;
    }

    private Instance _instance;
    final private String _adapterName;
    private java.util.HashMap _servantMapMap = new java.util.HashMap();
}
