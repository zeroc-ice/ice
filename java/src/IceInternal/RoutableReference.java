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
    changeDefault()
    {
	RoutableReference r = (RoutableReference)super.changeDefault();
	r._secure = false;
	r._routerInfo = getInstance().routerManager().get(getInstance().referenceFactory().getDefaultRouter());
	r._collocationOptimization = getInstance().defaultsAndOverrides().defaultCollocationOptimization;
	return r;
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
	if((newRouterInfo == _routerInfo) ||
		(newRouterInfo != null && _routerInfo != null && newRouterInfo.equals(_routerInfo)))
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

    private boolean _secure;
    private RouterInfo _routerInfo; // Null if no router is used.
    private boolean _collocationOptimization;
}
