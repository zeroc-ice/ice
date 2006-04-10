// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;

    public sealed class RouterInfo
    {

	internal RouterInfo(Ice.RouterPrx router)
	{
	    _router = router;
	    
	    Debug.Assert(_router != null);
	}
	
	public void destroy()
	{
	    lock(this)
	    {
		_clientProxy = null;
		_serverProxy = null;
		_adapter = null;
		_identities.Clear();
	    }
	}
	
	public override bool Equals(System.Object obj)
	{
	    if(object.ReferenceEquals(this, obj))
	    {
		return true;
	    }

	    RouterInfo rhs = obj as RouterInfo;
	    return rhs == null ? false : _router.Equals(rhs._router);
	}

	public override int GetHashCode()
	{
	    return _router.GetHashCode();
	}
	
	public Ice.RouterPrx getRouter()
	{
	    //
	    // No mutex lock necessary, _router is immutable.
	    //
	    return _router;
	}

	public Ice.ObjectPrx getClientProxy()
	{
	    lock(this)
	    {
		if(_clientProxy == null) // Lazy initialization.
		{
		    _clientProxy = _router.getClientProxy();
		    if(_clientProxy == null)
		    {
			throw new Ice.NoEndpointException();
		    }

		    _clientProxy = _clientProxy.ice_router(null); // The client proxy cannot be routed.
		    
		    //
		    // In order to avoid creating a new connection to
		    // the router, we must use the same timeout as the
		    // already existing connection.
		    //
		    try
		    {
		        _clientProxy = _clientProxy.ice_timeout(_router.ice_connection().timeout());
		    }
		    catch(Ice.CollocationOptimizationException)
		    {
		        // Ignore - collocated router.
		    }
		}
		
		return _clientProxy;
	    }
	}

	public void setClientProxy(Ice.ObjectPrx clientProxy)
	{
	    lock(this)
	    {
		_clientProxy = clientProxy.ice_router(null); // The client proxy cannot be routed.

		//
		// In order to avoid creating a new connection to the
		// router, we must use the same timeout as the already
		// existing connection.
		//
		try
		{
		    _clientProxy = _clientProxy.ice_timeout(_router.ice_connection().timeout());
		}
		catch(Ice.CollocationOptimizationException)
		{
		    // Ignore - collocated router.
		}
	    }
	}

	public Ice.ObjectPrx getServerProxy()
	{
	    if(_serverProxy == null) // Lazy initialization.
	    {
		_serverProxy = _router.getServerProxy();
		if(_serverProxy == null)
		{
		    throw new Ice.NoEndpointException();
		}

		_serverProxy = _serverProxy.ice_router(null); // The server proxy cannot be routed.
	    }
	    
	    return _serverProxy;
	}

	public void setServerProxy(Ice.ObjectPrx serverProxy)
	{
	    _serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
	}

	public void addProxy(Ice.ObjectPrx proxy)
	{
	    Debug.Assert(proxy != null);

	    lock(this)
	    {
	        if(!_identities.Contains(proxy.ice_getIdentity()))
		{
		    //
		    // Only add the proxy to the router if it's not already in our local map.
		    //
		    Ice.ObjectPrx[] proxies = new Ice.ObjectPrx[1];
		    proxies[0] = proxy;
		    Ice.ObjectPrx[] evictedProxies = _router.addProxies(proxies);

		    //
		    // If we successfully added the proxy to the router, we add it to our local map.
		    //
		    _identities.Add(proxy.ice_getIdentity());

		    //
		    // We also must remove whatever proxies the router evicted.
		    //
		    for(int i = 0; i < evictedProxies.Length; ++i)
		    {
		        _identities.Remove(evictedProxies[i].ice_getIdentity());
		    }
		}
	    }
	}

	public void setAdapter(Ice.ObjectAdapter adapter)
	{
	    lock(this)
	    {
		_adapter = adapter;
	    }
	}

	public Ice.ObjectAdapter getAdapter()
	{
	    lock(this)
	    {
		return _adapter;
	    }
	}
	
	private readonly Ice.RouterPrx _router;
	private Ice.ObjectPrx _clientProxy;
	private Ice.ObjectPrx _serverProxy;
	private IceUtil.Set _identities = new IceUtil.Set();
	private Ice.ObjectAdapter _adapter;
    }

    public sealed class RouterManager
    {
        internal RouterManager()
        {
            _table = new Hashtable();
        }
	
        internal void destroy()
        {
            lock(this)
            {
                foreach(RouterInfo i in _table.Values)
                {
                    i.destroy();
                }
                _table.Clear();
            }
        }
	
        //
        // Returns router info for a given router. Automatically creates
        // the router info if it doesn't exist yet.
        //
        public RouterInfo get(Ice.RouterPrx rtr)
        {
            if(rtr == null)
            {
                return null;
            }
	    
            Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null)); // The router cannot be routed.
	    
            lock(this)
            {
                RouterInfo info = (RouterInfo)_table[router];
                if(info == null)
                {
                    info = new RouterInfo(router);
                    _table[router] = info;
                }
		
                return info;
            }
        }

        //
        // Returns router info for a given router. Automatically creates
        // the router info if it doesn't exist yet.
        //
        public RouterInfo erase(Ice.RouterPrx rtr)
        {
	    RouterInfo info = null;
            if(rtr == null)
            {
		Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null)); // The router cannot be routed.
		lock(this)
		{
		    info = (RouterInfo)_table[router];
		    if(info != null)
		    {
			_table.Remove(router);
		    }
		}
	    }
	    return info;
        }
	
        private Hashtable _table;
    }

}
