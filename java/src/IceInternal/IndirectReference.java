// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
		      boolean collocationOpt)
    {
    	super(inst, com, ident, ctx, fs, md, sec, rtrInfo, collocationOpt);
        _adapterId = adptid;
	_locatorInfo = locInfo;
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

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
	//
	// Return a direct reference if a null locator is given.
	//
	if(newLocator == null)
	{
	    return getInstance().referenceFactory().create(getIdentity(), getContext(), getFacet(), getMode(),
							   getSecure(), new EndpointI[0], getRouterInfo(),
							   getCollocationOptimization());
	}
	else
	{
	    LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
	    if((newLocatorInfo == _locatorInfo) ||
		(_locatorInfo != null && newLocatorInfo != null && newLocatorInfo.equals(_locatorInfo)))
	    {
		return this;
	    }
	    IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	    r._locatorInfo = newLocatorInfo;
	    return this;
	}
    }

    public Reference
    changeCompress(boolean newCompress)
    {
        IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	if(_locatorInfo != null)
	{
	    Ice.LocatorPrx newLocator = Ice.LocatorPrxHelper.uncheckedCast(
	    					_locatorInfo.getLocator().ice_compress(newCompress));
	    r._locatorInfo = getInstance().locatorManager().get(newLocator);
	}
	return r;
    }

    public Reference
    changeTimeout(int newTimeout)
    {
        IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	if(_locatorInfo != null)
	{
	    Ice.LocatorPrx newLocator = Ice.LocatorPrxHelper.uncheckedCast(
	    					_locatorInfo.getLocator().ice_timeout(newTimeout));
	    r._locatorInfo = getInstance().locatorManager().get(newLocator);
	}
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
	        endpts = _locatorInfo.getEndpoints(this, cached);
	    }
	    for(int i = 0; i < endpts.length; ++i)
	    {
	        endpts[i] = endpts[i].connectionId(_connectionId);
	    }
	    EndpointI[] filteredEndpoints = filterEndpoints(endpts);
	    if(filteredEndpoints.length == 0)
	    {
	        Ice.NoEndpointException ex = new Ice.NoEndpointException();
		ex.proxy = toString();
		throw ex;
	    }

	    try
	    {
		OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
		connection = factory.create(filteredEndpoints, comp);
		assert(connection != null);
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
	return _locatorInfo == null ? rhs._locatorInfo == null : _locatorInfo.equals(rhs._locatorInfo);
    }

    private String _adapterId;
    private String _connectionId = "";
    private LocatorInfo _locatorInfo;
}
