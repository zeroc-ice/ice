// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class DirectReference extends RoutableReference
{
    public
    DirectReference(Instance inst,
    		    Ice.Identity ident,
		    java.util.Hashtable ctx,
		    String fs,
		    int md,
		    boolean sec,
		    Endpoint[] endpts,
		    RouterInfo rtrInfo)
    {
    	super(inst, ident, ctx, fs, md, sec, rtrInfo);
        _endpoints = endpts;
    }

    public Endpoint[] getEndpoints()
    {
        return _endpoints;
    }

    public final Reference
    changeEndpoints(Endpoint[] newEndpoints)
    {
	if(compare(newEndpoints, _endpoints))
	{
	    return this;
	}
        DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
	r._endpoints = newEndpoints;
	return r;
    }

    public Reference
    changeDefault()
    {
	//
	// Return an indirect reference if a default locator is set.
	//
	Ice.LocatorPrx loc = getInstance().referenceFactory().getDefaultLocator();
	if(loc != null)
	{
	    LocatorInfo newLocatorInfo = getInstance().locatorManager().get(loc);
	    return getInstance().referenceFactory().create(getIdentity(), null, "", ModeTwoway, false, "", null,
							   newLocatorInfo);
	}
	else
	{
	    return super.changeDefault();
	}
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
	if(newLocator != null)
	{
	    LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
	    return getInstance().referenceFactory().create(getIdentity(), getContext(), getFacet(), getMode(),
							   getSecure(), "", null, newLocatorInfo);
	}
	else
	{
	    return this;
	}
    }

    public Reference
    changeTimeout(int newTimeout)
    {
        DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
        Endpoint[] newEndpoints = new Endpoint[_endpoints.length];
        for(int i = 0; i < _endpoints.length; i++)
        {
            newEndpoints[i] = _endpoints[i].timeout(newTimeout);
        }
	r._endpoints = newEndpoints;
	return r;
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

    public Ice.Connection
    getConnection()
    {
        Endpoint[] endpts = super.getRoutedEndpoints();
	if(endpts.length == 0)
	{
	    endpts = _endpoints;
	}
	Endpoint[] filteredEndpoints = filterEndpoints(endpts);
	if(filteredEndpoints.length == 0)
	{
	    Ice.NoEndpointException ex = new Ice.NoEndpointException();
	    ex.proxy = toString();
	    throw ex;
	}

	OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
	Ice.Connection connection = factory.create(filteredEndpoints);

	//
	// If we have a router, set the object adapter for this router
	// (if any) to the new connection, so that callbacks from the
	// router can be received over this new connection.
	//
	if(getRouterInfo() != null)
	{
	    connection.setAdapter(getRouterInfo().getAdapter());
	}

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(connection != null);
	}
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
	return compare(_endpoints, rhs._endpoints);
    }

    protected
    DirectReference()
    {
    }

    protected void
    shallowCopy(DirectReference ref)
    {
	super.shallowCopy(ref);
	ref._endpoints = _endpoints;
    }

    public java.lang.Object
    ice_clone()
    {
	DirectReference result = new DirectReference();
        shallowCopy(result);
	return result;
    }

    private Endpoint[] _endpoints;
}
