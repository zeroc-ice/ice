// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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

    public final Endpoint[]
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
	return new Endpoint[0];
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
	return _routerInfo == null ? rhs._routerInfo == null : _routerInfo.equals(rhs._routerInfo);
    }

    protected
    RoutableReference()
    {
	//
	// Required for cloning operations.
	//
    }

    protected void
    shallowCopy(RoutableReference ref)
    {
	super.shallowCopy(ref);
	ref._routerInfo = _routerInfo;
    }

    protected
    RoutableReference(Instance inst,
		      Ice.Communicator com,
		      Ice.Identity ident,
                      java.util.Hashtable context,
		      String fac,
		      int md,
		      boolean secure,
		      RouterInfo rtrInfo)
    {
        super(inst, com, ident, context, fac, md, secure);
	_routerInfo = rtrInfo;
    }

    private RouterInfo _routerInfo; // Null if no router is used.
}
