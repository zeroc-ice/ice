// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class IndirectReference extends RoutableReference
{
    public
    IndirectReference(Instance inst,
		      Ice.Communicator com,
    		      Ice.Identity ident,
		      java.util.Map ctx,
		      String fs,
		      int md,
		      boolean sec,
		      String adptid,
		      RouterInfo rtrInfo,
		      LocatorInfo locInfo,
		      boolean collocationOpt,
		      int locatorCacheTimeout)
    {
    	super(inst, com, ident, ctx, fs, md, sec, rtrInfo, collocationOpt);
        _adapterId = adptid;
	_locatorInfo = locInfo;
	_locatorCacheTimeout = locatorCacheTimeout;
    }

    public final LocatorInfo
    getLocatorInfo()
    {
        return _locatorInfo;
    }

    public final String
    getAdapterId()
    {
        return _adapterId;
    }

    public EndpointI[]
    getEndpoints()
    {
        return new EndpointI[0];
    }

    public int
    getLocatorCacheTimeout()
    {
	return _locatorCacheTimeout;
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
	LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
	if(_locatorInfo != null && newLocatorInfo != null && newLocatorInfo.equals(_locatorInfo))
	{
	    return this;
	}
	IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	r._locatorInfo = newLocatorInfo;
	return r;
    }

    public Reference
    changeCompress(boolean newCompress)
    {
	if(_overrideCompress && _compress == newCompress)
	{
	    return this;
	}
        IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
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
        IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
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
        IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	r._connectionId = id;
	return r;	
    }

    public Reference
    changeAdapterId(String newAdapterId)
    {
	if(_adapterId.equals(newAdapterId))
	{
	    return this;
	}
        IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	r._adapterId = newAdapterId;
	return r;	
    }

    public Reference
    changeEndpoints(EndpointI[] newEndpoints)
    {
	if(newEndpoints == null || newEndpoints.length == 0)
	{
	    return this;
	}
	return getInstance().referenceFactory().create(getIdentity(), getContext(), getFacet(), getMode(),
						       getSecure(), newEndpoints, getRouterInfo(),
						       getCollocationOptimization());	
    }

    public Reference
    changeLocatorCacheTimeout(int newTimeout)
    {
	if(_locatorCacheTimeout == newTimeout)
	{
	    return this;
	}
        IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	r._locatorCacheTimeout = newTimeout;
	return r;	
    }

    public void
    streamWrite(BasicStream s)
	throws Ice.MarshalException
    {
        super.streamWrite(s);

	s.writeSize(0);
	s.writeString(_adapterId);
    }

    public String
    toString()
    {
        String result = super.toString();

	if(_adapterId.length() == 0)
	{
	    return result;
	}

	StringBuffer s = new StringBuffer();
	s.append(result);
	s.append(" @ ");

	//
	// If the encoded adapter id string contains characters which
	// the reference parser uses as separators, then we enclose
	// the adapter id string in quotes.
	//
	String a = IceUtil.StringUtil.escapeString(_adapterId, null);
	if(IceUtil.StringUtil.findFirstOf(a, " \t\n\r") != -1)
	{
	    s.append('"');
	    s.append(a);
	    s.append('"');
	}
	else
	{
	    s.append(a);
	}
	return s.toString();
    }

    public Ice.ConnectionI
    getConnection(Ice.BooleanHolder comp)
    {
	Ice.ConnectionI connection;

	while(true)
	{
	    EndpointI[] endpts = super.getRoutedEndpoints();
	    Ice.BooleanHolder cached = new Ice.BooleanHolder(false);
	    if(endpts.length == 0 && _locatorInfo != null)
	    {
	        endpts = _locatorInfo.getEndpoints(this, _locatorCacheTimeout, cached);
	    }

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

	    try
	    {
		connection = createConnection(endpts, comp);
		assert(connection != null);
	    }
	    catch(Ice.NoEndpointException ex)
	    {
		throw ex; // No need to retry if there's no endpoints.
	    }
	    catch(Ice.LocalException ex)
	    {
		if(getRouterInfo() == null)
		{
		    assert(_locatorInfo != null);
		    _locatorInfo.clearCache(this);

		    if(cached.value)
		    {
			TraceLevels traceLevels = getInstance().traceLevels();
			Ice.Logger logger = getInstance().logger();
			
			if(traceLevels.retry >= 2)
			{
			    String s = "connection to cached endpoints failed\n" +
				       "removing endpoints from cache and trying one more time\n" + ex;
			    logger.trace(traceLevels.retryCat, s);
			}
			
			continue;
		    }
		}

		throw ex;
	    }

	    break;
	}

	//
	// If we have a router, set the object adapter for this router
	// (if any) to the new connection, so that callbacks from the
	// router can be received over this new connection.
	//
	if(getRouterInfo() != null)
	{
	    connection.setAdapter(getRouterInfo().getAdapter());
	}

	assert(connection != null);
	return connection;
    }

    public synchronized int
    hashCode()
    {
        if(_hashInitialized)
	{
	    return _hashValue;
	}
	super.hashCode();             // Initializes _hashValue.
	int sz = _adapterId.length(); // Add hash of adapter ID to base hash.
	for(int i = 0; i < sz; i++)
	{   
	    _hashValue = 5 * _hashValue + (int)_adapterId.charAt(i);
	}
	return _hashValue;
    }

    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
	{
	    return true;
	}
	if(!(obj instanceof IndirectReference))
	{
	    return false;
	}
        IndirectReference rhs = (IndirectReference)obj;
        if(!super.equals(rhs))
        {
            return false;
        }
	if(!_adapterId.equals(rhs._adapterId))
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
	if(_locatorInfo == null ? rhs._locatorInfo != null : !_locatorInfo.equals(rhs._locatorInfo))
	{
	    return false;
	}
	return _locatorCacheTimeout == rhs._locatorCacheTimeout;
    }

    private String _adapterId;
    private String _connectionId = "";
    private LocatorInfo _locatorInfo;
    private boolean _overrideCompress;
    private boolean _compress; // Only used if _overrideCompress == true
    private boolean _overrideTimeout;
    private int _timeout; // Only used if _overrideTimeout == true
    private int _locatorCacheTimeout;
}
