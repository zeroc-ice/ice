// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    public final static int ModeDatagram = 3;
    public final static int ModeBatchDatagram = 4;
    public final static int ModeLast = ModeBatchDatagram;

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
        return _context;
    }

    public final Reference
    defaultContext()
    {
        Reference r = _instance.referenceFactory().copy(this);
        r._context = _instance.getDefaultContext();
        return r;

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

    public abstract boolean getSecure();
    public abstract boolean getPreferSecure();
    public abstract String getAdapterId();
    public abstract EndpointI[] getEndpoints();
    public abstract boolean getCollocationOptimization();
    public abstract int getLocatorCacheTimeout();
    public abstract boolean getCacheConnection();
    public abstract Ice.EndpointSelectionType getEndpointSelection();
    public abstract boolean getThreadPerConnection();

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
        Reference r = _instance.referenceFactory().copy(this);
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
        r._identity = (Ice.Identity)newIdentity.clone();
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

    public abstract Reference changeSecure(boolean newSecure);
    public abstract Reference changePreferSecure(boolean newPreferSecure);
    public abstract Reference changeRouter(Ice.RouterPrx newRouter);
    public abstract Reference changeLocator(Ice.LocatorPrx newLocator);
    public abstract Reference changeCompress(boolean newCompress);
    public abstract Reference changeTimeout(int newTimeout);
    public abstract Reference changeConnectionId(String connectionId);
    public abstract Reference changeCollocationOptimization(boolean newCollocationOptimization);
    public abstract Reference changeAdapterId(String newAdapterId);
    public abstract Reference changeEndpoints(EndpointI[] newEndpoints);
    public abstract Reference changeLocatorCacheTimeout(int newTimeout);
    public abstract Reference changeCacheConnection(boolean newCache);
    public abstract Reference changeEndpointSelection(Ice.EndpointSelectionType newType);
    public abstract Reference changeThreadPerConnection(boolean newTpc);

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

        h = 5 * h + _context.entrySet().hashCode();

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

    public abstract Ice.ConnectionI getConnection(Ice.BooleanHolder comp);

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
    private Ice.Communicator _communicator;

    private int _mode;
    private Ice.Identity _identity;
    private java.util.Map _context;
    private static java.util.HashMap _emptyContext = new java.util.HashMap();
    private String _facet;

    protected int _hashValue;
    protected boolean _hashInitialized;

    protected
    Reference(Instance inst,
              Ice.Communicator communicator,
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
        _communicator = communicator;
        _mode = md;
        _identity = ident;
        _context = ctx == null ? _emptyContext : ctx;
        _facet = fac;
        _hashInitialized = false;
    }
}
