// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Reference
{
    public final static int ModeTwoway = 0;
    public final static int ModeOneway = 1;
    public final static int ModeBatchOneway = 2;
    public final static int ModeDatagram = 3;
    public final static int ModeBatchDatagram = 4;
    public final static int ModeLast = ModeBatchDatagram;

    public final int
    getMode()
    {
        return _mode;
    }

    public final boolean
    getSecure()
    {
        return _secure;
    }

    public final Ice.Identity
    getIdentity()
    {
        return _identity;
    }

    public final String
    getFacet()
    {
        return _facet;
    }

    public final Instance
    getInstance()
    {
        return _instance;
    }

    public final java.util.Hashtable
    getContext()
    {
	return _context;
    }

    public final Ice.Communicator getCommunicator()
    {
        return _communicator;
    }

    public RouterInfo
    getRouterInfo()
    {
        return null;
    }

    public LocatorInfo
    getLocatorInfo()
    {
        return null;
    }

    public abstract Endpoint[] getEndpoints();
    public abstract String getAdapterId();

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    public final Reference
    changeContext(java.util.Hashtable newContext)
    {
	if(newContext == null)
	{
	    newContext = _emptyContext;
	}
	Reference r = _instance.referenceFactory().copy(this);
	if(newContext.isEmpty())
	{
	    r._context = _emptyContext;
	}
	else
	{
	    java.util.Hashtable newTable = new java.util.Hashtable(newContext.size());
	    java.util.Enumeration e = newContext.keys();
	    while(e.hasMoreElements())
	    {
		java.lang.Object key = e.nextElement();
		newTable.put(key, newContext.get(key));
	    }
	    r._context = newTable;
	}
	return r;
    }

    public final Reference
    changeMode(int newMode)
    {
        if(newMode == _mode)
	{
	    return this;
	}
	Reference r = _instance.referenceFactory().copy(this);
	r._mode = newMode;
	return r;
    }

    public final Reference
    changeSecure(boolean newSecure)
    {
        if(newSecure == _secure)
	{
	    return this;
	}
	Reference r = _instance.referenceFactory().copy(this);
	r._secure = newSecure;
	return r;
    }

    public final Reference
    changeIdentity(Ice.Identity newIdentity)
    {
        if(newIdentity.equals(_identity))
	{
	    return this;
	}
	Reference r = _instance.referenceFactory().copy(this);
	try
	{
	    r._identity = (Ice.Identity)newIdentity.ice_clone();
	}
	catch(IceUtil.CloneException ex)
	{
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(false);
	    }
	}
	return r;
    }

    public final Reference
    changeFacet(String newFacet)
    {
        if(newFacet.equals(_facet))
	{
	    return this;
	}
	Reference r = _instance.referenceFactory().copy(this);
	r._facet = newFacet;
	return r;
    }

    public Reference
    changeTimeout(int newTimeout)
    {
	if(_overrideTimeout && _timeout == newTimeout)
	{
	    return this;
	}
        Reference r = getInstance().referenceFactory().copy(this);
	r._timeout = newTimeout;
	r._overrideTimeout = true;
	return r;	
    }

    public abstract Reference changeAdapterId(String newAdapterId);
    public abstract Reference changeRouter(Ice.RouterPrx newRouter);
    public abstract Reference changeLocator(Ice.LocatorPrx newLocator);

    public synchronized int
    hashCode()
    {
	if(_hashInitialized)
	{
	    return _hashValue;
	}
        
        int h = _mode;

        int sz = _identity.name.length();
        for(int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)_identity.name.charAt(i);
        }

        sz = _identity.category.length();
        for(int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)_identity.category.charAt(i);
        }

        sz = _facet.length();
        for(int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)_facet.charAt(i);
        }

	h = 5 * h + (getSecure() ? 1 : 0);

	_hashValue = h;
	_hashInitialized = true;

        return h;
    }

    //
    // Marshal the reference.
    //
    public void
    streamWrite(BasicStream s)
    {
        //
        // Don't write the identity here. Operations calling streamWrite
        // write the identity.
        //

        //
        // For compatibility with the old FacetPath.
        //
        if(_facet.length() == 0)
        {
            s.writeStringSeq(null);
        }
        else
        {
            String[] facetPath = { _facet };
            s.writeStringSeq(facetPath);
        }

        s.writeByte((byte)_mode);

	s.writeBool(getSecure());

	// Derived class writes the remainder of the reference.
    }

    //
    // Convert the reference to its string form.
    //
    public String
    toString()
    {
	//
	// WARNING: Certain features, such as proxy validation in Glacier2,
	// depend on the format of proxy strings. Changes to toString() and
	// methods called to generate parts of the reference string could break
	// these features. Please review for all features that depend on the
	// format of proxyToString() before changing this and related code.
	//
	StringBuffer s = new StringBuffer();

	//
	// If the encoded identity string contains characters which
	// the reference parser uses as separators, then we enclose
	// the identity string in quotes.
	//
	String id = _instance.identityToString(_identity);
	if(IceUtil.StringUtil.findFirstOf(id, " \t\n\r:@") != -1)
	{
	    s.append('"');
	    s.append(id);
	    s.append('"');
	}
	else
	{
	    s.append(id);
	}

	if(_facet.length() > 0)
	{
	    //
	    // If the encoded facet string contains characters which
	    // the reference parser uses as separators, then we enclose
	    // the facet string in quotes.
	    //
	    s.append(" -f ");
	    String fs = IceUtil.StringUtil.escapeString(_facet, "");
	    if(IceUtil.StringUtil.findFirstOf(fs, " \t\n\r:@") != -1)
	    {
		s.append('"');
		s.append(fs);
		s.append('"');
	    }
	    else
	    {
		s.append(fs);
	    }
	}

	switch(_mode)
	{
	    case ModeTwoway:
		{
		    s.append(" -t");
		    break;
		}

	    case ModeOneway:
		{
		    s.append(" -o");
		    break;
		}

	    case ModeBatchOneway:
		{
		    s.append(" -O");
		    break;
		}

	    case ModeDatagram:
		{
		    s.append(" -d");
		    break;
		}

	    case ModeBatchDatagram:
		{
		    s.append(" -D");
		    break;
		}
	}

	if(getSecure())
	{
	    s.append(" -s");
	}

	return s.toString();

	// Derived class writes the remainder of the string.
    }

    public abstract Ice.Connection getConnection();

    public boolean
    equals(java.lang.Object obj)
    {
	//
	// Note: if(this == obj) and type test are performed by each non-abstract derived class.
	//

        Reference r = (Reference)obj; // Guaranteed to succeed.

        if(_mode != r._mode)
        {
            return false;
        }

	if(_secure != r._secure)
	{
	    return false;
	}

        if(!_identity.equals(r._identity))
        {
            return false;
        }

	if(!IceUtil.Hashtable.equals(_context, r._context))
	{
	    return false;
	}

        if(!_facet.equals(r._facet))
        {
            return false;
        }

	if(_overrideTimeout != r._overrideTimeout)
	{
	   return false;
	}
	if(_overrideTimeout && _timeout != r._timeout)
	{
	    return false;
	}	

        return true;
    }

    protected void
    shallowCopy(Reference dest)
    {
	dest._instance = _instance;
	dest._mode = _mode;
	dest._identity = _identity;
	dest._context = _context;
	dest._emptyContext = _emptyContext;
	dest._facet = _facet;
	dest._timeout = _timeout;
	dest._overrideTimeout = _overrideTimeout;
	dest._hashInitialized = false;
    }

    public java.lang.Object
    ice_clone()
    {
	//
	// This should not be called. The cloning operation will be handled by descendents.
	//
	IceUtil.Debug.Assert(false);
	return null;
    }

    private Instance _instance;
    private Ice.Communicator _communicator;

    private int _mode;
    private boolean _secure;
    private Ice.Identity _identity;
    private java.util.Hashtable _context;
    private static java.util.Hashtable _emptyContext = new java.util.Hashtable();
    private String _facet;

    //
    // NOTE: The override timeout should theoritically be in
    // RoutableReference. But for consistency with the C++ version we
    // keep it here (see also comment in src/IceE/Reference.h)
    //
    private boolean _overrideTimeout;
    private int _timeout; // Only used if _overrideTimeout == true

    protected int _hashValue;
    protected boolean _hashInitialized;

    protected
    Reference()
    {
	//
	// Default constructor required for cloning operation.
	//
    }
    
    protected
    Reference(Instance inst,
              Ice.Communicator com,
              Ice.Identity ident,
              java.util.Hashtable context,
              String fac,
              int md,
	      boolean sec)
    {
        //
        // Validate string arguments.
        //
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(ident.name != null);
	    IceUtil.Debug.Assert(ident.category != null);
	    IceUtil.Debug.Assert(fac != null);
	}

        _instance = inst;
        _communicator = com;
        _mode = md;
        _secure = sec;
        _identity = ident;
	_context = context == null ? _emptyContext : context;
        _facet = fac;
	_overrideTimeout = false;
	_timeout = -1;
	_hashInitialized = false;
    }

    protected void
    applyOverrides(Endpoint[] endpts)
    {
	for(int i = 0; i < endpts.length; ++i)
	{
	    if(_overrideTimeout)
	    {
		endpts[i] = endpts[i].timeout(_timeout);		    
	    }
	}
    }

    //
    // Filter endpoints based on criteria from this reference.
    //
    protected Endpoint[]
    filterEndpoints(Endpoint[] allEndpoints)
    {
        java.util.Vector endpoints = new java.util.Vector();

        //
        // Filter out unknown endpoints.
        //
        for(int i = 0; i < allEndpoints.length; i++)
        {
            if(!allEndpoints[i].unknown())
            {
                endpoints.addElement(allEndpoints[i]);
            }
        }

        //
        // Filter out endpoints according to the mode of the reference.
        //
        switch(getMode())
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeBatchOneway:
            {
                //
                // Filter out datagram endpoints. 
		//
		for(int i = endpoints.size(); i > 0; --i)
		{
		    if(((Endpoint)endpoints.elementAt(i - 1)).datagram())
		    {
			endpoints.removeElementAt(i - 1);
		    }
		}
                break;
            }

            case Reference.ModeDatagram:
            case Reference.ModeBatchDatagram:
            {
                //
                // Filter out non-datagram endpoints.
                //
		for(int i = endpoints.size(); i > 0; --i)
		{
		    if(!((Endpoint)endpoints.elementAt(i - 1)).datagram())
		    {
			endpoints.removeElementAt(i - 1);
		    }
		}
                break;
            }
        }

        if(endpoints.size() > 1)
        {
            //
            // Randomize the order of the endpoints.
            //
            java.util.Vector randomizedEndpoints = new java.util.Vector();
            randomizedEndpoints.setSize(endpoints.size());
            java.util.Random r = new java.util.Random();
            java.util.Enumeration e = endpoints.elements();
            while(e.hasMoreElements())
            {
                int index;
                do
                {
                    index = Math.abs(r.nextInt() % endpoints.size());
                }   
                while(randomizedEndpoints.elementAt(index) != null);
                randomizedEndpoints.setElementAt(e.nextElement(), index);
            }
            endpoints = randomizedEndpoints;
        }

        if(endpoints.size() > 1)
        {
            //
            // If a secure connection is requested or secure overrides is
            // set, remove all non-secure endpoints. Otherwise if preferSecure is set
            // make secure endpoints prefered. By default make non-secure
            // endpoints preferred over secure endpoints.
            //
            java.util.Vector secureEndpoints = new java.util.Vector();
	    for(int i = endpoints.size(); i > 0; --i)
	    {
		if(((Endpoint)endpoints.elementAt(i - 1)).secure())
		{
		    secureEndpoints.addElement(endpoints.elementAt(i - 1));
		    endpoints.removeElementAt(i - 1);
		}
	    }
            if(getSecure())
            {
                endpoints = secureEndpoints;
            }
            else
            {
		java.util.Enumeration e = secureEndpoints.elements();
		while(e.hasMoreElements())
		{
		    endpoints.addElement(e.nextElement());
		}
            }
        }
        else if(endpoints.size() == 1)
        {
            Endpoint endpoint = (Endpoint)endpoints.elementAt(0);
            if(getSecure() && !endpoint.secure())
            {
                endpoints.removeElementAt(0);
            }
        }

        //
        // Copy the endpoints into an array.
        //
        Endpoint[] arr = new Endpoint[endpoints.size()];
	endpoints.copyInto(arr);
        return arr;
    }

    protected boolean
    compare(Endpoint[] arr1, Endpoint[] arr2)
    {
        if(arr1 == arr2)
        {
            return true;
        }

        if(arr1.length == arr2.length)
        {
            for(int i = 0; i < arr1.length; i++)
            {
                if(!arr1[i].equals(arr2[i]))
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }
}
