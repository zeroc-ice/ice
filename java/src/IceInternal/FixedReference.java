// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        super(instance, communicator, identity, facet, mode, secure, Ice.Util.Protocol_1_0, encoding);
        _fixedConnection = connection;
    }

    public EndpointI[]
    getEndpoints()
    {
        return _emptyEndpoints;
    }

    public String
    getAdapterId()
    {
        return "";
    }

    public LocatorInfo
    getLocatorInfo()
    {
        return null;
    }

    public RouterInfo
    getRouterInfo()
    {
        return null;
    }

    public boolean
    getCollocationOptimized()
    {
        return false;
    }
    
    public final boolean
    getCacheConnection()
    {
        return true;
    }

    public boolean
    getPreferSecure()
    {
        return false;
    }

    public final Ice.EndpointSelectionType
    getEndpointSelection()
    {
        return Ice.EndpointSelectionType.Random;
    }

    public int
    getLocatorCacheTimeout()
    {
        return 0;
    }

    public String
    getConnectionId()
    {
        return "";
    }

    public Reference
    changeEndpoints(EndpointI[] newEndpoints)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeAdapterId(String newAdapterId)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeCollocationOptimized(boolean newCollocationOptimized)
    {
        throw new Ice.FixedProxyException();
    }

    public final Reference
    changeCacheConnection(boolean newCache)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changePreferSecure(boolean prefSec)
    {
        throw new Ice.FixedProxyException();
    }

    public final Reference
    changeEndpointSelection(Ice.EndpointSelectionType newType)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeLocatorCacheTimeout(int newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeTimeout(int newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeConnectionId(String connectionId)
    {
        throw new Ice.FixedProxyException();
    }

    public boolean
    isIndirect()
    {
        return false;
    }

    public boolean
    isWellKnown()
    {
        return false;
    }

    public void
    streamWrite(BasicStream s)
        throws Ice.MarshalException
    {
        throw new Ice.FixedProxyException();
    }

    public String
    toString()
        throws Ice.MarshalException
    {
        throw new Ice.FixedProxyException();
    }

    public java.util.Map<String, String>
    toProperty(String prefix)
    {
        throw new Ice.FixedProxyException();
    }

    public Ice.ConnectionI
    getConnection(Ice.BooleanHolder compress)
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

        if(defaultsAndOverrides.overrideCompress)
        {
            compress.value = defaultsAndOverrides.overrideCompressValue;
        }
        else if(_overrideCompress)
        {
            compress.value = _compress;
        }
        else
        {
            compress.value = _fixedConnection.endpoint().compress();
        }
        return _fixedConnection;
    }

    public void
    getConnection(GetConnectionCallback callback)
    {
        try
        {
            Ice.BooleanHolder compress = new Ice.BooleanHolder();
            Ice.ConnectionI connection = getConnection(compress);
            callback.setConnection(connection, compress.value);
        }
        catch(Ice.LocalException ex)
        {
            callback.setException(ex);
        }
    }

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

    public int
    hashCode()
    {
        return super.hashCode();
    }

    private Ice.ConnectionI _fixedConnection;
    private static EndpointI[] _emptyEndpoints = new EndpointI[0];
}
