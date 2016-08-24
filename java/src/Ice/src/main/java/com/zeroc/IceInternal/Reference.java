// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public abstract class Reference implements Cloneable
{
    public final static int ModeTwoway = 0;
    public final static int ModeOneway = 1;
    public final static int ModeBatchOneway = 2;
    public final static int ModeDatagram = 3;
    public final static int ModeBatchDatagram = 4;
    public final static int ModeLast = ModeBatchDatagram;

    public interface GetConnectionCallback
    {
        void setConnection(com.zeroc.Ice.ConnectionI connection, boolean compress);
        void setException(com.zeroc.Ice.LocalException ex);
    }

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

    public final com.zeroc.Ice.ProtocolVersion
    getProtocol()
    {
        return _protocol;
    }

    public final com.zeroc.Ice.EncodingVersion
    getEncoding()
    {
        return _encoding;
    }

    public final com.zeroc.Ice.Identity
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

    public final java.util.Map<String, String>
    getContext()
    {
        return _context;
    }

    public int
    getInvocationTimeout()
    {
        return _invocationTimeout;
    }

    public final com.zeroc.Ice.Communicator
    getCommunicator()
    {
        return _communicator;
    }

    public abstract EndpointI[] getEndpoints();
    public abstract String getAdapterId();
    public abstract RouterInfo getRouterInfo();
    public abstract LocatorInfo getLocatorInfo();
    public abstract boolean getCollocationOptimized();
    public abstract boolean getCacheConnection();
    public abstract boolean getPreferSecure();
    public abstract com.zeroc.Ice.EndpointSelectionType getEndpointSelection();
    public abstract int getLocatorCacheTimeout();
    public abstract String getConnectionId();

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    public final Reference
    changeContext(java.util.Map<String, String> newContext)
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
            r._context = new java.util.HashMap<>(newContext);
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

    public Reference
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
    changeIdentity(com.zeroc.Ice.Identity newIdentity)
    {
        if(newIdentity.equals(_identity))
        {
            return this;
        }
        Reference r = _instance.referenceFactory().copy(this);
        r._identity = newIdentity.clone();
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

    public final Reference
    changeInvocationTimeout(int newTimeout)
    {
        if(newTimeout == _invocationTimeout)
        {
            return this;
        }
        Reference r = _instance.referenceFactory().copy(this);
        r._invocationTimeout = newTimeout;
        return r;
    }

    public Reference
    changeEncoding(com.zeroc.Ice.EncodingVersion newEncoding)
    {
        if(newEncoding.equals(_encoding))
        {
            return this;
        }
        Reference r = _instance.referenceFactory().copy(this);
        r._encoding = newEncoding;
        return r;
    }

    public Reference
    changeCompress(boolean newCompress)
    {
        if(_overrideCompress && _compress == newCompress)
        {
            return this;
        }
        Reference r = _instance.referenceFactory().copy(this);
        r._compress = newCompress;
        r._overrideCompress = true;
        return r;
    }

    public abstract Reference changeAdapterId(String newAdapterId);
    public abstract Reference changeEndpoints(EndpointI[] newEndpoints);
    public abstract Reference changeLocator(com.zeroc.Ice.LocatorPrx newLocator);
    public abstract Reference changeRouter(com.zeroc.Ice.RouterPrx newRouter);
    public abstract Reference changeCollocationOptimized(boolean newCollocationOptimized);
    public abstract Reference changeCacheConnection(boolean newCache);
    public abstract Reference changePreferSecure(boolean newPreferSecure);
    public abstract Reference changeEndpointSelection(com.zeroc.Ice.EndpointSelectionType newType);
    public abstract Reference changeLocatorCacheTimeout(int newTimeout);

    public abstract Reference changeTimeout(int newTimeout);
    public abstract Reference changeConnectionId(String connectionId);

    @Override
    public synchronized int
    hashCode()
    {
        if(_hashInitialized)
        {
            return _hashValue;
        }

        int h = 5381;
        h = HashUtil.hashAdd(h, _mode);
        h = HashUtil.hashAdd(h, _secure);
        h = HashUtil.hashAdd(h, _identity);
        h = HashUtil.hashAdd(h, _context);
        h = HashUtil.hashAdd(h, _facet);
        h = HashUtil.hashAdd(h, _overrideCompress);
        if(_overrideCompress)
        {
            h = HashUtil.hashAdd(h, _compress);
        }
        h = HashUtil.hashAdd(h, _protocol);
        h = HashUtil.hashAdd(h, _encoding);
        h = HashUtil.hashAdd(h, _invocationTimeout);

        _hashValue = h;
        _hashInitialized = true;

        return _hashValue;
    }

    //
    // Utility methods
    //
    public abstract boolean isIndirect();
    public abstract boolean isWellKnown();

    //
    // Marshal the reference.
    //
    public void
    streamWrite(com.zeroc.Ice.OutputStream s)
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

        s.writeBool(_secure);

        if(!s.getEncoding().equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            _protocol.ice_write(s);
            _encoding.ice_write(s);
        }

        // Derived class writes the remainder of the reference.
    }

    //
    // Convert the reference to its string form.
    //
    @Override
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
        StringBuilder s = new StringBuilder(128);

        //
        // If the encoded identity string contains characters which
        // the reference parser uses as separators, then we enclose
        // the identity string in quotes.
        //
        String id = com.zeroc.Ice.Util.identityToString(_identity);
        if(com.zeroc.IceUtilInternal.StringUtil.findFirstOf(id, " :@") != -1)
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
            String fs = com.zeroc.IceUtilInternal.StringUtil.escapeString(_facet, "");
            if(com.zeroc.IceUtilInternal.StringUtil.findFirstOf(fs, " :@") != -1)
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

        if(_secure)
        {
            s.append(" -s");
        }

        if(!_protocol.equals(com.zeroc.Ice.Util.Protocol_1_0))
        {
            //
            // We only print the protocol if it's not 1.0. It's fine as
            // long as we don't add Ice.Default.ProtocolVersion, a
            // stringified proxy will convert back to the same proxy with
            // stringToProxy.
            //
            s.append(" -p ");
            s.append(com.zeroc.Ice.Util.protocolVersionToString(_protocol));
        }

        //
        // Always print the encoding version to ensure a stringified proxy
        // will convert back to a proxy with the same encoding with
        // stringToProxy (and won't use Ice.Default.EncodingVersion).
        //
        s.append(" -e ");
        s.append(com.zeroc.Ice.Util.encodingVersionToString(_encoding));

        return s.toString();

        // Derived class writes the remainder of the string.
    }

    //
    // Convert the reference to its property form.
    //
    public abstract java.util.Map<String, String> toProperty(String prefix);

    public abstract RequestHandler getRequestHandler(com.zeroc.Ice._ObjectPrxI proxy);

    public abstract BatchRequestQueue getBatchRequestQueue();

    @Override
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

        if(!_context.equals(r._context))
        {
            return false;
        }

        if(!_facet.equals(r._facet))
        {
            return false;
        }

        if(_overrideCompress != r._overrideCompress)
        {
           return false;
        }
        if(_overrideCompress && _compress != r._compress)
        {
            return false;
        }

        if(!_protocol.equals(r._protocol))
        {
            return false;
        }

        if(!_encoding.equals(r._encoding))
        {
            return false;
        }

        if(_invocationTimeout != r._invocationTimeout)
        {
            return false;
        }

        return true;
    }

    @Override
    public Reference clone()
    {
	Reference c = null;
	try
	{
	    c = (Reference)super.clone();
	}
	catch(CloneNotSupportedException ex)
	{
	    assert false;
	}
	return c;
    }

    protected int _hashValue;
    protected boolean _hashInitialized;
    private static java.util.Map<String, String> _emptyContext = new java.util.HashMap<>();

    final private Instance _instance;
    final private com.zeroc.Ice.Communicator _communicator;

    private int _mode;
    private boolean _secure;
    private com.zeroc.Ice.Identity _identity;
    private java.util.Map<String, String> _context;
    private String _facet;
    private com.zeroc.Ice.ProtocolVersion _protocol;
    private com.zeroc.Ice.EncodingVersion _encoding;
    private int _invocationTimeout;
    protected boolean _overrideCompress;
    protected boolean _compress; // Only used if _overrideCompress == true

    protected
    Reference(Instance instance,
              com.zeroc.Ice.Communicator communicator,
              com.zeroc.Ice.Identity identity,
              String facet,
              int mode,
              boolean secure,
              com.zeroc.Ice.ProtocolVersion protocol,
              com.zeroc.Ice.EncodingVersion encoding,
              int invocationTimeout,
              java.util.Map<String, String> context)
    {
        //
        // Validate string arguments.
        //
        assert(identity.name != null);
        assert(identity.category != null);
        assert(facet != null);

        _instance = instance;
        _communicator = communicator;
        _mode = mode;
        _secure = secure;
        _identity = identity;
        _context = context != null ? new java.util.HashMap<>(context) : _emptyContext;
        _facet = facet;
        _protocol = protocol;
        _encoding = encoding;
        _invocationTimeout = invocationTimeout;
        _hashInitialized = false;
        _overrideCompress = false;
        _compress = false;
    }
}
