// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class FixedReference extends Reference
{
    public
    FixedReference(Instance instance,
                   com.zeroc.Ice.Communicator communicator,
                   com.zeroc.Ice.Identity identity,
                   String facet,
                   int mode,
                   boolean secure,
                   com.zeroc.Ice.EncodingVersion encoding,
                   com.zeroc.Ice.ConnectionI connection)
    {
        super(instance, communicator, identity, facet, mode, secure, com.zeroc.Ice.Util.Protocol_1_0, encoding, -1,
              null);
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
    public final com.zeroc.Ice.EndpointSelectionType
    getEndpointSelection()
    {
        return com.zeroc.Ice.EndpointSelectionType.Random;
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
    public Reference
    changeEndpoints(EndpointI[] newEndpoints)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeAdapterId(String newAdapterId)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeLocator(com.zeroc.Ice.LocatorPrx newLocator)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeRouter(com.zeroc.Ice.RouterPrx newRouter)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeCollocationOptimized(boolean newCollocationOptimized)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public final Reference
    changeCacheConnection(boolean newCache)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public Reference
    changePreferSecure(boolean prefSec)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public final Reference
    changeEndpointSelection(com.zeroc.Ice.EndpointSelectionType newType)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeLocatorCacheTimeout(int newTimeout)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeTimeout(int newTimeout)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public Reference
    changeConnectionId(String connectionId)
    {
        throw new com.zeroc.Ice.FixedProxyException();
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
    streamWrite(com.zeroc.Ice.OutputStream s)
        throws com.zeroc.Ice.MarshalException
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public java.util.Map<String, String>
    toProperty(String prefix)
    {
        throw new com.zeroc.Ice.FixedProxyException();
    }

    @Override
    public RequestHandler
    getRequestHandler(com.zeroc.Ice._ObjectPrxI proxy)
    {
        switch(getMode())
        {
        case Reference.ModeTwoway:
        case Reference.ModeOneway:
        case Reference.ModeBatchOneway:
        {
            if(_fixedConnection.endpoint().datagram())
            {
                throw new com.zeroc.Ice.NoEndpointException("");
            }
            break;
        }

        case Reference.ModeDatagram:
        case Reference.ModeBatchDatagram:
        {
            if(!_fixedConnection.endpoint().datagram())
            {
                throw new com.zeroc.Ice.NoEndpointException("");
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
            throw new com.zeroc.Ice.NoEndpointException("");
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

    private com.zeroc.Ice.ConnectionI _fixedConnection;
    private static EndpointI[] _emptyEndpoints = new EndpointI[0];
}
