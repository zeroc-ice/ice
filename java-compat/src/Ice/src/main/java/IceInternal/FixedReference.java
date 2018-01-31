// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class FixedReference extends Reference
{
    public
    FixedReference(Instance instance,
                   Ice.Communicator communicator,
                   Ice.Identity identity,
                   String facet,
                   int mode,
                   boolean secure,
                   Ice.EncodingVersion encoding,
                   Ice.ConnectionI connection)
    {
        super(instance, communicator, identity, facet, mode, secure, Ice.Util.Protocol_1_0, encoding, -1, null);
        _fixedConnection = connection;
    }

    @Override
    public EndpointI[]
    getEndpoints()
    {
        return _emptyEndpoints;
    }

    @Override
    public String
    getAdapterId()
    {
        return "";
    }

    @Override
    public LocatorInfo
    getLocatorInfo()
    {
        return null;
    }

    @Override
    public RouterInfo
    getRouterInfo()
    {
        return null;
    }

    @Override
    public boolean
    getCollocationOptimized()
    {
        return false;
    }

    @Override
    public final boolean
    getCacheConnection()
    {
        return true;
    }

    @Override
    public boolean
    getPreferSecure()
    {
        return false;
    }

    @Override
    public final Ice.EndpointSelectionType
    getEndpointSelection()
    {
        return Ice.EndpointSelectionType.Random;
    }

    @Override
    public int
    getLocatorCacheTimeout()
    {
        return 0;
    }

    @Override
    public String
    getConnectionId()
    {
        return "";
    }

    @Override
    public Ice.IntOptional
    getTimeout()
    {
        return new Ice.IntOptional();
    }

    @Override
    public Reference
    changeEndpoints(EndpointI[] newEndpoints)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeAdapterId(String newAdapterId)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeCollocationOptimized(boolean newCollocationOptimized)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public final Reference
    changeCacheConnection(boolean newCache)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changePreferSecure(boolean prefSec)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public final Reference
    changeEndpointSelection(Ice.EndpointSelectionType newType)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeLocatorCacheTimeout(int newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeTimeout(int newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeConnectionId(String connectionId)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeConnection(Ice.ConnectionI connection)
    {
        if(_fixedConnection == connection)
        {
            return this;
        }
        FixedReference r = (FixedReference)getInstance().referenceFactory().copy(this);
        r._fixedConnection = connection;
        return r;
    }

    @Override
    public boolean
    isIndirect()
    {
        return false;
    }

    @Override
    public boolean
    isWellKnown()
    {
        return false;
    }

    @Override
    public void
    streamWrite(Ice.OutputStream s)
        throws Ice.MarshalException
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public java.util.Map<String, String>
    toProperty(String prefix)
    {
        throw new Ice.FixedProxyException();
    }

    @Override
    public RequestHandler
    getRequestHandler(Ice.ObjectPrxHelperBase proxy)
    {
        switch(getMode())
        {
        case Reference.ModeTwoway:
        case Reference.ModeOneway:
        case Reference.ModeBatchOneway:
        {
            if(_fixedConnection.endpoint().datagram())
            {
                throw new Ice.NoEndpointException("");
            }
            break;
        }

        case Reference.ModeDatagram:
        case Reference.ModeBatchDatagram:
        {
            if(!_fixedConnection.endpoint().datagram())
            {
                throw new Ice.NoEndpointException("");
            }
            break;
        }
        }

        //
        // If a secure connection is requested or secure overrides is set,
        // check if the connection is secure.
        //
        boolean secure;
        DefaultsAndOverrides defaultsAndOverrides = getInstance().defaultsAndOverrides();
        if(defaultsAndOverrides.overrideSecure)
        {
            secure = defaultsAndOverrides.overrideSecureValue;
        }
        else
        {
            secure = getSecure();
        }
        if(secure && !_fixedConnection.endpoint().secure())
        {
            throw new Ice.NoEndpointException("");
        }

        _fixedConnection.throwException(); // Throw in case our connection is already destroyed.

        boolean compress = false;
        if(defaultsAndOverrides.overrideCompress)
        {
            compress = defaultsAndOverrides.overrideCompressValue;
        }
        else if(_overrideCompress)
        {
            compress = _compress;
        }

        RequestHandler handler = new ConnectionRequestHandler(this, _fixedConnection, compress);
        if(getInstance().queueRequests())
        {
            handler = new QueueRequestHandler(getInstance(), handler);
        }
        return proxy._setRequestHandler(handler);
    }

    @Override
    public BatchRequestQueue
    getBatchRequestQueue()
    {
        return _fixedConnection.getBatchRequestQueue();
    }

    @Override
    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
        {
            return true;
        }
        if(!(obj instanceof FixedReference))
        {
            return false;
        }
        FixedReference rhs = (FixedReference)obj;
        if(!super.equals(rhs))
        {
            return false;
        }
        return _fixedConnection.equals(rhs._fixedConnection);
    }

    @Override
    public int
    hashCode()
    {
        return super.hashCode();
    }

    private Ice.ConnectionI _fixedConnection;
    private static EndpointI[] _emptyEndpoints = new EndpointI[0];
}
