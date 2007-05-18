// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class RoutableReference extends Reference
{
    public final RouterInfo
    getRouterInfo()
    {
        return _routerInfo;
    }

    public final EndpointI[]
    getRoutedEndpoints()
    {
        if(_routerInfo != null)
        {
            //
            // If we route, we send everything to the router's client
            // proxy endpoints.
            //
            return _routerInfo.getClientEndpoints();
        }
        return new EndpointI[0];
    }

    public final boolean
    getSecure()
    {
        return _secure;
    }

    public final boolean
    getPreferSecure()
    {
        return _preferSecure;
    }

    public final boolean
    getCollocationOptimization()
    {
        return _collocationOptimization;
    }

    public final boolean
    getCacheConnection()
    {
        return _cacheConnection;
    }

    public final Ice.EndpointSelectionType
    getEndpointSelection()
    {
        return _endpointSelection;
    }

    public final boolean
    getThreadPerConnection()
    {
        return _threadPerConnection;
    }

    public Reference
    changeSecure(boolean newSecure)
    {
        if(newSecure == _secure)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._secure = newSecure;
        return r;
    }

    public Reference
    changePreferSecure(boolean newPreferSecure)
    {
        if(newPreferSecure == _preferSecure)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._preferSecure = newPreferSecure;
        return r;
    }

    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        RouterInfo newRouterInfo = getInstance().routerManager().get(newRouter);
        if(newRouterInfo != null && _routerInfo != null && newRouterInfo.equals(_routerInfo))
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._routerInfo = newRouterInfo;
        return r;
    }

    public Reference
    changeCollocationOptimization(boolean newCollocationOptimization)
    {
        if(newCollocationOptimization == _collocationOptimization)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._collocationOptimization = newCollocationOptimization;
        return r;
    }

    public Reference
    changeCompress(boolean newCompress)
    {
        if(_overrideCompress && _compress == newCompress)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._compress = newCompress;
        r._overrideCompress = true;
        return r;       
    }

    public Reference
    changeTimeout(int newTimeout)
    {
        if(_overrideTimeout && _timeout == newTimeout)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._timeout = newTimeout;
        r._overrideTimeout = true;
        return r;       
    }

    public Reference
    changeConnectionId(String id)
    {
        if(_connectionId.equals(id))
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._connectionId = id;
        return r;       
    }

    public final Reference
    changeCacheConnection(boolean newCache)
    {
        if(newCache == _cacheConnection)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._cacheConnection = newCache;
        return r;
    }

    public final Reference
    changeEndpointSelection(Ice.EndpointSelectionType newType)
    {
        if(newType == _endpointSelection)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._endpointSelection = newType;
        return r;
    }

    public Reference
    changeThreadPerConnection(boolean newTpc)
    {
        if(newTpc == _threadPerConnection)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._threadPerConnection = newTpc;
        return r;       
    }

    public synchronized int
    hashCode()
    {
        return super.hashCode();
    }

    public boolean
    equals(java.lang.Object obj)
    {
        //
        // Note: if(this == obj) and type test are performed by each non-abstract derived class.
        //

        if(!super.equals(obj))
        {
            return false;
        }
        RoutableReference rhs = (RoutableReference)obj; // Guaranteed to succeed.
        if(_secure != rhs._secure)
        {
            return false;
        }
        if(_preferSecure != rhs._preferSecure)
        {
            return false;
        }
        if(_collocationOptimization != rhs._collocationOptimization)
        {
            return false;
        }
        if(_cacheConnection != rhs._cacheConnection)
        {
            return false;
        }
        if(_endpointSelection != rhs._endpointSelection)
        {
            return false;
        }
        if(!_connectionId.equals(rhs._connectionId))
        {
           return false;
        }
        if(_overrideCompress != rhs._overrideCompress)
        {
           return false;
        }
        if(_overrideCompress && _compress != rhs._compress)
        {
            return false;
        }
        if(_overrideTimeout != rhs._overrideTimeout)
        {
           return false;
        }
        if(_overrideTimeout && _timeout != rhs._timeout)
        {
            return false;
        }
        if(_threadPerConnection != rhs._threadPerConnection)
        {
            return false;
        }
        return _routerInfo == null ? rhs._routerInfo == null : _routerInfo.equals(rhs._routerInfo);
    }

    protected
    RoutableReference(Instance inst,
                      Ice.Communicator com,
                      Ice.Identity ident,
                      java.util.Map ctx,
                      String fac,
                      int md,
                      boolean sec,
                      boolean prefSec,
                      RouterInfo rtrInfo,
                      boolean collocationOpt,
                      boolean cacheConnection,
                      Ice.EndpointSelectionType endpointSelection,
                      boolean threadPerConnection)
    {
        super(inst, com, ident, ctx, fac, md);
        _secure = sec;
        _preferSecure = prefSec;
        _routerInfo = rtrInfo;
        _collocationOptimization = collocationOpt;
        _cacheConnection = cacheConnection;
        _endpointSelection = endpointSelection;
        _overrideCompress = false;
        _compress = false;
        _overrideTimeout = false;
        _timeout = -1;
        _threadPerConnection = threadPerConnection;
    }

    protected void
    applyOverrides(EndpointI[] endpts)
    {
        //
        // Apply the endpoint overrides to each endpoint.
        //
        for(int i = 0; i < endpts.length; ++i)
        {
            endpts[i] = endpts[i].connectionId(_connectionId);
            if(_overrideCompress)
            {
                endpts[i] = endpts[i].compress(_compress);                  
            }
            if(_overrideTimeout)
            {
                endpts[i] = endpts[i].timeout(_timeout);
            }
        }
    }

    protected Ice.ConnectionI
    createConnection(EndpointI[] allEndpoints, Ice.BooleanHolder compress)
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
                java.util.Iterator i = endpoints.iterator();
                while(i.hasNext())
                {
                    EndpointI endpoint = (EndpointI)i.next();
                    if(endpoint.datagram())
                    {
                        i.remove();
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
                java.util.Iterator i = endpoints.iterator();
                while(i.hasNext())
                {
                    EndpointI endpoint = (EndpointI)i.next();
                    if(!endpoint.datagram())
                    {
                        i.remove();
                    }
                }
                break;
            }
        }

        //
        // Sort the endpoints according to the endpoint selection type.
        //
        switch(getEndpointSelection().value())
        {
            case Ice.EndpointSelectionType._Random:
            {
                java.util.Collections.shuffle(endpoints);
                break;
            }
            case Ice.EndpointSelectionType._Ordered:
            {
                // Nothing to do.
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }

        //
        // If a secure connection is requested or secure overrides is
        // set, remove all non-secure endpoints. Otherwise if preferSecure is set
        // make secure endpoints prefered. By default make non-secure
        // endpoints preferred over secure endpoints.
        //
        DefaultsAndOverrides overrides = getInstance().defaultsAndOverrides();
        if(overrides.overrideSecure ? overrides.overrideSecureValue : getSecure())
        {
            java.util.Iterator i = endpoints.iterator();
            while(i.hasNext())
            {
                EndpointI endpoint = (EndpointI)i.next();
                if(!endpoint.secure())
                {
                    i.remove();
                }
            }
        }
        else if(getPreferSecure())
        {
            java.util.Collections.sort(endpoints, _preferSecureEndpointComparator);
        }
        else
        {
            java.util.Collections.sort(endpoints, _preferNonSecureEndpointComparator);
        }

        if(endpoints.size() == 0)
        {
            Ice.NoEndpointException ex = new Ice.NoEndpointException();
            ex.proxy = toString();
            throw ex;
        }
        
        //
        // Finally, create the connection.
        //
        OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
        if(getCacheConnection() || endpoints.size() == 1)
        {
            //
            // Get an existing connection or create one if there's no
            // existing connection to one of the given endpoints.
            //
            return factory.create((EndpointI[])endpoints.toArray(
                new EndpointI[endpoints.size()]), false, _threadPerConnection, getEndpointSelection(), compress);
        }
        else
        {
            //
            // Go through the list of endpoints and try to create the
            // connection until it succeeds. This is different from just
            // calling create() with the given endpoints since this might
            // create a new connection even if there's an existing
            // connection for one of the endpoints.
            //
            
            Ice.LocalException exception = null;
            EndpointI[] endpoint = new EndpointI[1];
            
            java.util.Iterator i = endpoints.iterator();
            while(i.hasNext())
            {
                try
                {
                    endpoint[0] = (EndpointI)i.next();
                    return factory.create(endpoint, i.hasNext(), _threadPerConnection, getEndpointSelection(),
                                          compress);
                }
                catch(Ice.LocalException ex)
                {
                    exception = ex;
                }
            }

            assert(exception != null);
            throw exception;
        }
    }

    static class EndpointComparator implements java.util.Comparator
    {
        EndpointComparator(boolean preferSecure)
        {
            _preferSecure = preferSecure;
        }
        
        public int
        compare(java.lang.Object l, java.lang.Object r)
        {
            IceInternal.EndpointI le = (IceInternal.EndpointI)l;
            IceInternal.EndpointI re = (IceInternal.EndpointI)r;
            boolean ls = le.secure();
            boolean rs = re.secure();
            if((ls && rs) || (!ls && !rs))
            {
                return 0;
            }
            else if(!ls && rs)
            {
                if(_preferSecure)
                {
                    return 1;
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                if(_preferSecure)
                {
                    return -1;
                }
                else
                {
                    return 1;
                }
            }
        }

        private boolean _preferSecure;
    }
    
    private static EndpointComparator _preferNonSecureEndpointComparator = new EndpointComparator(false);
    private static EndpointComparator _preferSecureEndpointComparator = new EndpointComparator(true);

    private boolean _secure;
    private boolean _preferSecure;
    private RouterInfo _routerInfo; // Null if no router is used.
    private boolean _collocationOptimization;
    private boolean _cacheConnection;
    private Ice.EndpointSelectionType _endpointSelection;
    private String _connectionId = "";
    private boolean _overrideCompress;
    private boolean _compress; // Only used if _overrideCompress == true
    private boolean _overrideTimeout;
    private int _timeout; // Only used if _overrideTimeout == true
    private boolean _threadPerConnection;
}
