// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class Reference implements Cloneable
{
    public final static int ModeTwoway = 0;
    public final static int ModeOneway = 1;
    public final static int ModeBatchOneway = 2;
    public final static int ModeLast = ModeBatchOneway;

    public final int
    getMode()
    {
        return _mode;
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

    public final java.util.Map
    getContext()
    {
	return _hasContext ? _context : _instance.getDefaultContext();
    }

    public final Reference
    defaultContext()
    {
        if(!_hasContext)
	{
	    return this;
	}
	Reference r = _instance.referenceFactory().copy(this);
	r._hasContext = false;
	r._context = _emptyContext;
	return r;

    }

    public abstract Endpoint[] getEndpoints();

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    public final Reference
    changeContext(java.util.Map newContext)
    {
	if(newContext == null)
	{
	    newContext = _emptyContext;
	}
        if(_hasContext && newContext.equals(_context))
	{
	    return this;
	}
	Reference r = _instance.referenceFactory().copy(this);
	r._hasContext = true;
	if(newContext.isEmpty())
	{
	    r._context = _emptyContext;
	}
	else
	{
	    r._context = new java.util.HashMap(newContext);
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
    changeIdentity(Ice.Identity newIdentity)
    {
        if(newIdentity.equals(_identity))
	{
	    return this;
	}
	Reference r = _instance.referenceFactory().copy(this);
	try
	{
	    r._identity = (Ice.Identity)newIdentity.clone();
	}
	catch(CloneNotSupportedException ex)
	{
	    assert(false);
	}
	return r;
    }

    public final boolean
    hasContext()
    {
        return _hasContext;
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

    //
    // Return a reference in the default configuration.
    //
    public Reference
    changeDefault()
    {
	Reference r = _instance.referenceFactory().copy(this);
	r._mode = ModeTwoway;
	r._hasContext = false;
	r._context = _emptyContext;
	r._facet = "";
	return r;
    }

    public abstract Reference changeRouter(Ice.RouterPrx newRouter);
    public abstract Reference changeLocator(Ice.LocatorPrx newLocator);
    public abstract Reference changeTimeout(int newTimeout);

    public final synchronized int
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

	if(_hasContext)
	{
	    h = 5 * h + _context.entrySet().hashCode();
	}

        sz = _facet.length();
        for(int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)_facet.charAt(i);
        }

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

	// Derived class writes the remainder of the reference.
    }

    //
    // Convert the reference to its string form.
    //
    public String
    toString()
    {
        StringBuffer s = new StringBuffer();

        //
        // If the encoded identity string contains characters which
        // the reference parser uses as separators, then we enclose
        // the identity string in quotes.
        //
        String id = Ice.Util.identityToString(_identity);
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
        }

        return s.toString();

	// Derived class writes the remainder of the string.
    }

    public abstract Ice.ConnectionI getConnection();

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

        if(!_identity.equals(r._identity))
        {
            return false;
        }

	if(_hasContext != r._hasContext)
	{
	    return false;
	}

	if(!_context.equals(r._context))
	{
	    return false;
	}

        if(!_facet.equals(r._facet))
        {
            return false;
        }

        return true;
    }

    public Object clone()
    {
	//
	// A member-wise copy is safe because the members are immutable.
	//
	Object o = null;
	try
	{
	    o = super.clone();
	}
	catch(CloneNotSupportedException ex)
	{
	}
	return o;
    }

    private Instance _instance;

    private int _mode;
    private Ice.Identity _identity;
    private boolean _hasContext;
    private java.util.Map _context;
    private static java.util.HashMap _emptyContext = new java.util.HashMap();
    private String _facet;

    private int _hashValue;
    private boolean _hashInitialized;

    protected
    Reference(Instance inst,
              Ice.Identity ident,
	      java.util.Map ctx,
              String fac,
              int md)
    {
        //
        // Validate string arguments.
        //
        assert(ident.name != null);
        assert(ident.category != null);
        assert(fac != null);

        _instance = inst;
        _mode = md;
        _identity = ident;
	_hasContext = ctx != null && !ctx.isEmpty();
	_context = ctx == null ? _emptyContext : ctx;
        _facet = fac;
	_hashInitialized = false;
    }

    //
    // Filter endpoints based on criteria from this reference.
    //
    protected Endpoint[]
    filterEndpoints(Endpoint[] allEndpoints)
    {
        java.util.ArrayList endpoints = new java.util.ArrayList();

        //
        // Filter out unknown endpoints.
        //
        for(int i = 0; i < allEndpoints.length; i++)
        {
            if(!allEndpoints[i].unknown())
            {
                endpoints.add(allEndpoints[i]);
            }
        }

        //
        // Randomize the order of endpoints.
        //
        java.util.Collections.shuffle(endpoints);

        Endpoint[] arr = new Endpoint[endpoints.size()];
        endpoints.toArray(arr);
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
