// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class RoutableReference extends Reference
{
    @Override
    public final EndpointI[]
    getEndpoints()
    {
        return _endpoints;
    }

    @Override
    public final String
    getAdapterId()
    {
        return _adapterId;
    }

    @Override
    public final LocatorInfo
    getLocatorInfo()
    {
        return _locatorInfo;
    }

    @Override
    public final RouterInfo
    getRouterInfo()
    {
        return _routerInfo;
    }

    @Override
    public final boolean
    getCollocationOptimized()
    {
        return _collocationOptimized;
    }

    @Override
    public final boolean
    getCacheConnection()
    {
        return _cacheConnection;
    }

    @Override
    public final boolean
    getPreferSecure()
    {
        return _preferSecure;
    }

    @Override
    public final Ice.EndpointSelectionType
    getEndpointSelection()
    {
        return _endpointSelection;
    }

    @Override
    public final int
    getLocatorCacheTimeout()
    {
        return _locatorCacheTimeout;
    }

    @Override
    public final String
    getConnectionId()
    {
        return _connectionId;
    }

    @Override
    public Reference
    changeEncoding(Ice.EncodingVersion newEncoding)
    {
        RoutableReference r = (RoutableReference)super.changeEncoding(newEncoding);
        if(r != this)
        {
            LocatorInfo locInfo = r._locatorInfo;
            if(locInfo != null && !locInfo.getLocator().ice_getEncodingVersion().equals(newEncoding))
            {
                r._locatorInfo = getInstance().locatorManager().get(
                    (Ice.LocatorPrx)locInfo.getLocator().ice_encodingVersion(newEncoding));
            }
        }
        return r;
    }

    @Override
    public Reference
    changeCompress(boolean newCompress)
    {
        RoutableReference r = (RoutableReference)super.changeCompress(newCompress);
        if(r != this && _endpoints.length > 0) // Also override the compress flag on the endpoints if it was updated.
        {
            EndpointI[] newEndpoints = new EndpointI[_endpoints.length];
            for(int i = 0; i < _endpoints.length; i++)
            {
                newEndpoints[i] = _endpoints[i].compress(newCompress);
            }
            r._endpoints = newEndpoints;
        }
        return r;
    }

    @Override
    public Reference
    changeEndpoints(EndpointI[] newEndpoints)
    {
        if(java.util.Arrays.equals(newEndpoints, _endpoints))
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._endpoints = newEndpoints;
        r._adapterId = "";
        r.applyOverrides(r._endpoints);
        return r;
    }

    @Override
    public Reference
    changeAdapterId(String newAdapterId)
    {
        if(_adapterId.equals(newAdapterId))
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._adapterId = newAdapterId;
        r._endpoints = _emptyEndpoints;
        return r;
    }

    @Override
    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
        LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
        if(newLocatorInfo != null && _locatorInfo != null && newLocatorInfo.equals(_locatorInfo))
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._locatorInfo = newLocatorInfo;
        return r;
    }

    @Override
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

    @Override
    public Reference
    changeCollocationOptimized(boolean newCollocationOptimized)
    {
        if(newCollocationOptimized == _collocationOptimized)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._collocationOptimized = newCollocationOptimized;
        return r;
    }

    @Override
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

    @Override
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

    @Override
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

    @Override
    public Reference
    changeLocatorCacheTimeout(int newTimeout)
    {
        if(_locatorCacheTimeout == newTimeout)
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._locatorCacheTimeout = newTimeout;
        return r;
    }

    @Override
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
        if(_endpoints.length > 0)
        {
            EndpointI[] newEndpoints = new EndpointI[_endpoints.length];
            for(int i = 0; i < _endpoints.length; i++)
            {
                newEndpoints[i] = _endpoints[i].timeout(newTimeout);
            }
            r._endpoints = newEndpoints;
        }
        return r;
    }

    @Override
    public Reference
    changeConnectionId(String id)
    {
        if(_connectionId.equals(id))
        {
            return this;
        }
        RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
        r._connectionId = id;
        if(_endpoints.length > 0)
        {
            EndpointI[] newEndpoints = new EndpointI[_endpoints.length];
            for(int i = 0; i < _endpoints.length; i++)
            {
                newEndpoints[i] = _endpoints[i].connectionId(id);
            }
            r._endpoints = newEndpoints;
        }
        return r;
    }

    @Override
    public boolean
    isIndirect()
    {
        return _endpoints.length == 0;
    }

    @Override
    public boolean
    isWellKnown()
    {
        return _endpoints.length == 0 && _adapterId.length() == 0;
    }

    @Override
    public void
    streamWrite(Ice.OutputStream s)
        throws Ice.MarshalException
    {
        super.streamWrite(s);

        s.writeSize(_endpoints.length);
        if(_endpoints.length > 0)
        {
            assert(_adapterId.length() == 0);
            for(EndpointI endpoint : _endpoints)
            {
                s.writeShort(endpoint.type());
                endpoint.streamWrite(s);
            }
        }
        else
        {
            s.writeString(_adapterId); // Adapter id.
        }
    }

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
        s.append(super.toString());
        if(_endpoints.length > 0)
        {
            for(EndpointI endpoint : _endpoints)
            {
                String endp = endpoint.toString();
                if(endp != null && endp.length() > 0)
                {
                    s.append(':');
                    s.append(endp);
                }
            }
        }
        else if(_adapterId.length() > 0)
        {
            s.append(" @ ");

            //
            // If the encoded adapter id string contains characters which
            // the reference parser uses as separators, then we enclose
            // the adapter id string in quotes.
            //
            String a = IceUtilInternal.StringUtil.escapeString(_adapterId, null, getInstance().toStringMode());
            if(IceUtilInternal.StringUtil.findFirstOf(a, " :@") != -1)
            {
                s.append('"');
                s.append(a);
                s.append('"');
            }
            else
            {
                s.append(a);
            }
        }
        return s.toString();
    }

    @Override
    public java.util.Map<String, String> toProperty(String prefix)
    {
        java.util.Map<String, String> properties = new java.util.HashMap<String, String>();

        properties.put(prefix, toString());
        properties.put(prefix + ".CollocationOptimized", _collocationOptimized ? "1" : "0");
        properties.put(prefix + ".ConnectionCached", _cacheConnection ? "1" : "0");
        properties.put(prefix + ".PreferSecure", _preferSecure ? "1" : "0");
        properties.put(prefix + ".EndpointSelection",
                       _endpointSelection == Ice.EndpointSelectionType.Random ? "Random" : "Ordered");

        {
            StringBuffer s = new StringBuffer();
            s.append(getInvocationTimeout());
            properties.put(prefix + ".InvocationTimeout", s.toString());
        }
        {
            StringBuffer s = new StringBuffer();
            s.append(_locatorCacheTimeout);
            properties.put(prefix + ".LocatorCacheTimeout", s.toString());
        }

        if(_routerInfo != null)
        {
            Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)_routerInfo.getRouter();
            java.util.Map<String, String> routerProperties = h._getReference().toProperty(prefix + ".Router");
            for(java.util.Map.Entry<String, String> p : routerProperties.entrySet())
            {
                properties.put(p.getKey(), p.getValue());
            }
        }

        if(_locatorInfo != null)
        {
            Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)_locatorInfo.getLocator();
            java.util.Map<String, String> locatorProperties = h._getReference().toProperty(prefix + ".Locator");
            for(java.util.Map.Entry<String, String> p : locatorProperties.entrySet())
            {
                properties.put(p.getKey(), p.getValue());
            }
        }

        return properties;
    }

    @Override
    public synchronized int
    hashCode()
    {
        if(!_hashInitialized)
        {
            super.hashCode(); // Initializes _hashValue.
            _hashValue = IceInternal.HashUtil.hashAdd(_hashValue, _adapterId);
        }
        return _hashValue;
    }

    @Override
    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
        {
            return true;
        }
        if(!(obj instanceof RoutableReference))
        {
            return false;
        }

        if(!super.equals(obj))
        {
            return false;
        }
        RoutableReference rhs = (RoutableReference)obj; // Guaranteed to succeed.
        if(_locatorInfo == null ? rhs._locatorInfo != null : !_locatorInfo.equals(rhs._locatorInfo))
        {
            return false;
        }
        if(_routerInfo == null ? rhs._routerInfo != null : !_routerInfo.equals(rhs._routerInfo))
        {
            return false;
        }
        if(_collocationOptimized != rhs._collocationOptimized)
        {
            return false;
        }
        if(_cacheConnection != rhs._cacheConnection)
        {
            return false;
        }
        if(_preferSecure != rhs._preferSecure)
        {
            return false;
        }
        if(_endpointSelection != rhs._endpointSelection)
        {
            return false;
        }
        if(_locatorCacheTimeout != rhs._locatorCacheTimeout)
        {
            return false;
        }
        if(!_connectionId.equals(rhs._connectionId))
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
        if(!java.util.Arrays.equals(_endpoints, rhs._endpoints))
        {
            return false;
        }
        if(!_adapterId.equals(rhs._adapterId))
        {
           return false;
        }
        return true;
    }

    @Override
    public RequestHandler
    getRequestHandler(Ice.ObjectPrxHelperBase proxy)
    {
        return getInstance().requestHandlerFactory().getRequestHandler(this, proxy);
    }

    @Override
    public BatchRequestQueue
    getBatchRequestQueue()
    {
        return new BatchRequestQueue(getInstance(), getMode() == Reference.ModeBatchDatagram);
    }

    public void
    getConnection(final GetConnectionCallback callback)
    {
        if(_routerInfo != null)
        {
            //
            // If we route, we send everything to the router's client
            // proxy endpoints.
            //
            _routerInfo.getClientEndpoints(new RouterInfo.GetClientEndpointsCallback()
                {
                    @Override
                    public void
                    setEndpoints(EndpointI[] endpts)
                    {
                        if(endpts.length > 0)
                        {
                            applyOverrides(endpts);
                            createConnection(endpts, callback);
                        }
                        else
                        {
                            getConnectionNoRouterInfo(callback);
                        }
                    }

                    @Override
                    public void
                    setException(Ice.LocalException ex)
                    {
                        callback.setException(ex);
                    }
                });
        }
        else
        {
            getConnectionNoRouterInfo(callback);
        }
    }

    public void
    getConnectionNoRouterInfo(final GetConnectionCallback callback)
    {
        if(_endpoints.length > 0)
        {
            createConnection(_endpoints, callback);
            return;
        }

        final RoutableReference self = this;
        if(_locatorInfo != null)
        {
            _locatorInfo.getEndpoints(this, _locatorCacheTimeout, new LocatorInfo.GetEndpointsCallback()
            {
                @Override
                public void
                setEndpoints(EndpointI[] endpoints, final boolean cached)
                {
                    if(endpoints.length == 0)
                    {
                        callback.setException(new Ice.NoEndpointException(self.toString()));
                        return;
                    }

                    applyOverrides(endpoints);
                    createConnection(endpoints, new GetConnectionCallback()
                    {
                        @Override
                        public void
                        setConnection(Ice.ConnectionI connection, boolean compress)
                        {
                            callback.setConnection(connection, compress);
                        }

                        @Override
                        public void
                        setException(Ice.LocalException exc)
                        {
                            try
                            {
                                throw exc;
                            }
                            catch(Ice.NoEndpointException ex)
                            {
                                callback.setException(ex); // No need to retry if there's no endpoints.
                            }
                            catch(Ice.LocalException ex)
                            {
                                assert(_locatorInfo != null);
                                _locatorInfo.clearCache(self);
                                if(cached)
                                {
                                    TraceLevels traceLvls = getInstance().traceLevels();
                                    if(traceLvls.retry >= 2)
                                    {
                                        String s = "connection to cached endpoints failed\n" +
                                            "removing endpoints from cache and trying again\n" + ex;
                                        getInstance().initializationData().logger.trace(traceLvls.retryCat, s);
                                    }
                                    getConnectionNoRouterInfo(callback); // Retry.
                                    return;
                                }
                                callback.setException(ex);
                            }
                        }
                        });
                }

                @Override
                public void
                setException(Ice.LocalException ex)
                {
                    callback.setException(ex);
                }
            });
        }
        else
        {
            callback.setException(new Ice.NoEndpointException(toString()));
        }
    }

    protected
    RoutableReference(Instance instance,
                      Ice.Communicator communicator,
                      Ice.Identity identity,
                      String facet,
                      int mode,
                      boolean secure,
                      Ice.ProtocolVersion protocol,
                      Ice.EncodingVersion encoding,
                      EndpointI[] endpoints,
                      String adapterId,
                      LocatorInfo locatorInfo,
                      RouterInfo routerInfo,
                      boolean collocationOptimized,
                      boolean cacheConnection,
                      boolean prefereSecure,
                      Ice.EndpointSelectionType endpointSelection,
                      int locatorCacheTimeout,
                      int invocationTimeout,
                      java.util.Map<String, String> context)
    {
        super(instance, communicator, identity, facet, mode, secure, protocol, encoding, invocationTimeout, context);
        _endpoints = endpoints;
        _adapterId = adapterId;
        _locatorInfo = locatorInfo;
        _routerInfo = routerInfo;
        _collocationOptimized = collocationOptimized;
        _cacheConnection = cacheConnection;
        _preferSecure = prefereSecure;
        _endpointSelection = endpointSelection;
        _locatorCacheTimeout = locatorCacheTimeout;
        _overrideTimeout = false;
        _timeout = -1;

        if(_endpoints == null)
        {
            _endpoints = _emptyEndpoints;
        }
        if(_adapterId == null)
        {
            _adapterId = "";
        }
        assert(_adapterId.length() == 0 || _endpoints.length == 0);
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

    private EndpointI[]
    filterEndpoints(EndpointI[] allEndpoints)
    {
        java.util.List<EndpointI> endpoints = new java.util.ArrayList<EndpointI>();

        //
        // Filter out opaque endpoints.
        //
        for(EndpointI endpoint : allEndpoints)
        {
            if(!(endpoint instanceof IceInternal.OpaqueEndpointI))
            {
                endpoints.add(endpoint);
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
                java.util.Iterator<EndpointI> i = endpoints.iterator();
                while(i.hasNext())
                {
                    EndpointI endpoint = i.next();
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
                java.util.Iterator<EndpointI> i = endpoints.iterator();
                while(i.hasNext())
                {
                    EndpointI endpoint = i.next();
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
        switch(getEndpointSelection())
        {
            case Random:
            {
                java.util.Collections.shuffle(endpoints);
                break;
            }
            case Ordered:
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
            java.util.Iterator<EndpointI> i = endpoints.iterator();
            while(i.hasNext())
            {
                EndpointI endpoint = i.next();
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

        return endpoints.toArray(new EndpointI[endpoints.size()]);
    }

    protected void
    createConnection(EndpointI[] allEndpoints, final GetConnectionCallback callback)
    {
        final EndpointI[] endpoints = filterEndpoints(allEndpoints);
        if(endpoints.length == 0)
        {
            callback.setException(new Ice.NoEndpointException(toString()));
            return;
        }

        //
        // Finally, create the connection.
        //
        final OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
        if(getCacheConnection() || endpoints.length == 1)
        {
            //
            // Get an existing connection or create one if there's no
            // existing connection to one of the given endpoints.
            //
            factory.create(endpoints, false, getEndpointSelection(),
                           new OutgoingConnectionFactory.CreateConnectionCallback()
                           {
                               @Override
                            public void
                               setConnection(Ice.ConnectionI connection, boolean compress)
                               {
                                   //
                                   // If we have a router, set the object adapter for this router
                                   // (if any) to the new connection, so that callbacks from the
                                   // router can be received over this new connection.
                                   //
                                   if(_routerInfo != null && _routerInfo.getAdapter() != null)
                                   {
                                       connection.setAdapter(_routerInfo.getAdapter());
                                   }
                                   callback.setConnection(connection, compress);
                               }

                               @Override
                            public void
                               setException(Ice.LocalException ex)
                               {
                                   callback.setException(ex);
                               }
                           });
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

            factory.create(new EndpointI[]{ endpoints[0] }, true, getEndpointSelection(),
                           new OutgoingConnectionFactory.CreateConnectionCallback()
                           {
                               @Override
                               public void
                               setConnection(Ice.ConnectionI connection, boolean compress)
                               {
                                   //
                                   // If we have a router, set the object adapter for this router
                                   // (if any) to the new connection, so that callbacks from the
                                   // router can be received over this new connection.
                                   //
                                   if(_routerInfo != null && _routerInfo.getAdapter() != null)
                                   {
                                       connection.setAdapter(_routerInfo.getAdapter());
                                   }
                                   callback.setConnection(connection, compress);
                               }

                               @Override
                               public void
                               setException(final Ice.LocalException ex)
                               {
                                   if(_exception == null)
                                   {
                                       _exception = ex;
                                   }

                                   if(++_i == endpoints.length)
                                   {
                                       callback.setException(_exception);
                                       return;
                                   }

                                   final boolean more = _i != endpoints.length - 1;
                                   final EndpointI[] endpoint = new EndpointI[]{ endpoints[_i] };
                                   factory.create(endpoint, more, getEndpointSelection(), this);
                               }

                               private int _i = 0;
                               private Ice.LocalException _exception = null;
                           });
        }
    }

    static class EndpointComparator implements java.util.Comparator<EndpointI>
    {
        EndpointComparator(boolean preferSecure)
        {
            _preferSecure = preferSecure;
        }

        @Override
        public int
        compare(EndpointI le, EndpointI re)
        {
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
    private static EndpointI[] _emptyEndpoints = new EndpointI[0];

    private EndpointI[] _endpoints;
    private String _adapterId;
    private LocatorInfo _locatorInfo; // Null if no router is used.
    private RouterInfo _routerInfo; // Null if no router is used.
    private boolean _collocationOptimized;
    private boolean _cacheConnection;
    private boolean _preferSecure;
    private Ice.EndpointSelectionType _endpointSelection;
    private int _locatorCacheTimeout;

    private boolean _overrideTimeout;
    private int _timeout; // Only used if _overrideTimeout == true
    private String _connectionId = "";
}
