// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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

    public final Endpoint[]
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
	return new Endpoint[0];
    }

    public final boolean
    getCollocationOptimization()
    {
        return _collocationOptimization;
    }

    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        RouterInfo newRouterInfo = getInstance().routerManager().get(newRouter);
	if(newRouterInfo == _routerInfo)
	{
	    return this;
	}
	RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
	r._routerInfo = newRouterInfo;
	return r;
    }

    public Reference
    changeDefault()
    {
	RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
	r._routerInfo = getInstance().routerManager().get(getInstance().referenceFactory().getDefaultRouter());
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
	if(_collocationOptimization != rhs._collocationOptimization)
	{
	    return false;
	}
	return _routerInfo == null ? rhs._routerInfo == null : _routerInfo.equals(rhs._routerInfo);
    }

    protected
    RoutableReference(Instance inst,
		      Ice.Identity ident,
		      java.util.Map ctx,
		      String fac,
		      int md,
		      boolean sec,
		      RouterInfo rtrInfo,
		      boolean collocationOpt)
    {
        super(inst, ident, ctx, fac, md, sec);
	_routerInfo = rtrInfo;
	_collocationOptimization = collocationOpt;
    }

    private RouterInfo _routerInfo; // Null if no router is used.
    private boolean _collocationOptimization;
}
