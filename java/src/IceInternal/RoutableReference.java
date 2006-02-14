// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
	    Ice.ObjectPrx clientProxy = _routerInfo.getClientProxy();
	    return ((Ice.ObjectPrxHelperBase)clientProxy).__reference().getEndpoints();
	}
	return new EndpointI[0];
    }

    public final boolean
    getSecure()
    {
	return _secure;
    }

    public final boolean
    getCollocationOptimization()
    {
        return _collocationOptimization;
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
	if(_collocationOptimization != rhs._collocationOptimization)
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
		      RouterInfo rtrInfo,
		      boolean collocationOpt)
    {
        super(inst, com, ident, ctx, fac, md);
	_secure = sec;
	_routerInfo = rtrInfo;
	_collocationOptimization = collocationOpt;
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
	    java.util.Collections.shuffle(endpoints);
	    break;
	case Ice.EndpointSelectionType._Ordered:
	    // Nothing to do.
	    break;
	default:
	    assert(false);
	    break;
	}

        //
        // If a secure connection is requested, remove all non-secure
        // endpoints. Otherwise make non-secure endpoints preferred over
        // secure endpoints by partitioning the endpoint vector, so that
        // non-secure endpoints come first.
        //
        if(getSecure())
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
        else
        {
            java.util.Collections.sort(endpoints, _endpointComparator);
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
	    return factory.create((EndpointI[])endpoints.toArray(new EndpointI[endpoints.size()]), false, compress);
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
		    return factory.create(endpoint, i.hasNext(), compress);
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
		return -1;
	    }
	    else
	    {
		return 1;
	    }
	}
    }
    
    private static EndpointComparator _endpointComparator = new EndpointComparator();

    private boolean _secure;
    private RouterInfo _routerInfo; // Null if no router is used.
    private boolean _collocationOptimization;
}
