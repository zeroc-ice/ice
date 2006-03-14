// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class DirectReference extends RoutableReference
{
    public
    DirectReference(Instance inst,
		    Ice.Communicator com,
    		    Ice.Identity ident,
		    java.util.Map ctx,
		    String fs,
		    int md,
		    boolean sec,
		    EndpointI[] endpts,
		    RouterInfo rtrInfo,
		    boolean collocationOpt)
    {
    	super(inst, com, ident, ctx, fs, md, sec, rtrInfo, collocationOpt);
        _endpoints = endpts;
    }

    public String
    getAdapterId()
    {
	return "";
    }

    public EndpointI[] 
    getEndpoints()
    {
        return _endpoints;
    }

    public int
    getLocatorCacheTimeout()
    {
	return 0;
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
	if(newLocator != null)
	{
	    LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
	    return getInstance().referenceFactory().create(getIdentity(), getContext(), getFacet(), getMode(),
							   getSecure(), "", null, newLocatorInfo,
							   getCollocationOptimization(),
							   getLocatorCacheTimeout());
	}
	else
	{
	    return this;
	}
    }

    public Reference
    changeCompress(boolean newCompress)
    {
        DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
        EndpointI[] newEndpoints = new EndpointI[_endpoints.length];
        for(int i = 0; i < _endpoints.length; i++)
        {
            newEndpoints[i] = _endpoints[i].compress(newCompress);
        }
	r._endpoints = newEndpoints;
	return r;
    }

    public Reference
    changeTimeout(int newTimeout)
    {
        DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
        EndpointI[] newEndpoints = new EndpointI[_endpoints.length];
        for(int i = 0; i < _endpoints.length; i++)
        {
            newEndpoints[i] = _endpoints[i].timeout(newTimeout);
        }
	r._endpoints = newEndpoints;
	return r;
    }

    public Reference
    changeConnectionId(String connectionId)
    {
        DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
        EndpointI[] newEndpoints = new EndpointI[_endpoints.length];
        for(int i = 0; i < _endpoints.length; i++)
        {
            newEndpoints[i] = _endpoints[i].connectionId(connectionId);
        }
	r._endpoints = newEndpoints;
	return r;
    }

    public Reference
    changeAdapterId(String newAdapterId)
    {
	if(newAdapterId == null || newAdapterId.length() == 0)
	{
	    return this;
	}
	LocatorInfo locatorInfo = 
	    getInstance().locatorManager().get(getInstance().referenceFactory().getDefaultLocator());
	return getInstance().referenceFactory().create(getIdentity(), getContext(), getFacet(), getMode(),
						       getSecure(), newAdapterId, getRouterInfo(), locatorInfo,
						       getCollocationOptimization(), getLocatorCacheTimeout());
    }

    public Reference
    changeEndpoints(EndpointI[] newEndpoints)
    {
	if(java.util.Arrays.equals(newEndpoints, _endpoints))
	{
	    return this;
	}

        DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
	r._endpoints = newEndpoints;
	return r;
    }

    public Reference
    changeLocatorCacheTimeout(int newTimeout)
    {
        return this;
    }

    public void
    streamWrite(BasicStream s)
	throws Ice.MarshalException
    {
        super.streamWrite(s);

	s.writeSize(_endpoints.length);
	if(_endpoints.length > 0)
	{
	    for(int i = 0; i < _endpoints.length; i++)
	    {
		_endpoints[i].streamWrite(s);
	    }
	}
	else
	{
	    s.writeString(""); // Adapter id.
	}
    }

    public String
    toString()
    {
	StringBuffer s = new StringBuffer();
	s.append(super.toString());

	for(int i = 0; i < _endpoints.length; i++)
	{
	    String endp = _endpoints[i].toString();
	    if(endp != null && endp.length() > 0)
	    {
		s.append(':');
		s.append(endp);
	    }
	}
	return s.toString();
    }

    public Ice.ConnectionI
    getConnection(Ice.BooleanHolder comp)
    {
        EndpointI[] endpts = super.getRoutedEndpoints();
	if(endpts.length == 0)
	{
	    endpts = _endpoints;
	}

	Ice.ConnectionI connection = createConnection(endpts, comp);

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
	if(!(obj instanceof DirectReference))
	{
	    return false;
	}
        DirectReference rhs = (DirectReference)obj;
        if(!super.equals(rhs))
        {
            return false;
        }
	return java.util.Arrays.equals(_endpoints, rhs._endpoints);
    }

    private EndpointI[] _endpoints;
}
