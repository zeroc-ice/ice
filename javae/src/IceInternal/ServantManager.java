// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ServantManager
{
    public synchronized void
    addServant(Ice.Object servant, Ice.Identity ident, String facet)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_instance != null); // Must not be called after destruction.
	}

        if(facet == null)
        {
            facet = "";
        }

        java.util.Hashtable m = (java.util.Hashtable)_servantMapMap.get(ident);
        if(m == null)
        {
            m = new java.util.Hashtable();
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
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_instance != null); // Must not be called after destruction.
	}

        if(facet == null)
        {
            facet = "";
        }

        java.util.Hashtable m = (java.util.Hashtable)_servantMapMap.get(ident);
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

    public synchronized java.util.Hashtable
    removeAllFacets(Ice.Identity ident)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_instance != null); // Must not be called after destruction.
	}

        java.util.Hashtable m = (java.util.Hashtable)_servantMapMap.get(ident);
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
	//if(IceUtil.Debug.ASSERT)
	//{
	//    IceUtil.Debug.Assert(_instance != null); // Must not be called after destruction.
	//}

        if(facet == null)
        {
            facet = "";
        }

        java.util.Hashtable m = (java.util.Hashtable)_servantMapMap.get(ident);
        Ice.Object obj = null;
        if(m != null)
        {
            obj = (Ice.Object)m.get(facet);
        }

        return obj;
    }

    public synchronized java.util.Hashtable
    findAllFacets(Ice.Identity ident)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_instance != null); // Must not be called after destruction.
	}

        java.util.Hashtable m = (java.util.Hashtable)_servantMapMap.get(ident);
        if(m != null)
        {
	    java.util.Hashtable result = new java.util.Hashtable(m.size());
	    java.util.Enumeration e = m.keys();
	    while(e.hasMoreElements())
	    {
		java.lang.Object key = e.nextElement();
		java.lang.Object value = m.get(key);
		result.put(key, value);
	    }
	    return result;
        }

        return new java.util.Hashtable();
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
	//if(IceUtil.Debug.ASSERT)
	//{
	//    IceUtil.Debug.Assert(_instance != null); // Must not be called after destruction.
	//}

        java.util.Hashtable m = (java.util.Hashtable)_servantMapMap.get(ident);
        if(m == null)
        {
            return false;
        }
        else
        {
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(!m.isEmpty());
	    }
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
	//IceUtil.Debug.FinalizerAssert(_instance == null);
    }

    //
    // Only for use by Ice.ObjectAdapter.
    //
    public synchronized void
    destroy()
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_instance != null); // Must not be called after destruction.
	}

	_servantMapMap.clear();
	_instance = null;
    }

    private Instance _instance;
    final private String _adapterName;
    private java.util.Hashtable _servantMapMap = new java.util.Hashtable();
}
