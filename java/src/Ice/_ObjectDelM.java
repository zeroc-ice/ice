// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package Ice;

public class _ObjectDelM implements _ObjectDel
{
    public boolean
    ice_isA(String __id, java.util.Map __context)
        throws IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out = getOutgoing("ice_isA", OperationMode.Nonmutating, __context);
        try
        {
            IceInternal.BasicStream __is = __out.is();
            IceInternal.BasicStream __os = __out.os();
            __os.writeString(__id);
            if(!__out.invoke())
            {
                throw new UnknownUserException();
            }
            try
            {
                return __is.readBool();
            }
            catch(LocalException __ex)
            {
                throw new IceInternal.NonRepeatable(__ex);
            }
        }
        finally
        {
            reclaimOutgoing(__out);
        }
    }

    public void
    ice_ping(java.util.Map __context)
        throws IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out = getOutgoing("ice_ping", OperationMode.Nonmutating, __context);
        try
        {
            if(!__out.invoke())
            {
                throw new UnknownUserException();
            }
        }
        finally
        {
            reclaimOutgoing(__out);
        }
    }

    public String[]
    ice_ids(java.util.Map __context)
        throws IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out = getOutgoing("ice_ids", OperationMode.Nonmutating, __context);
        try
        {
            IceInternal.BasicStream __is = __out.is();
            if(!__out.invoke())
            {
                throw new UnknownUserException();
            }
            try
            {
                return __is.readStringSeq();
            }
            catch(LocalException __ex)
            {
                throw new IceInternal.NonRepeatable(__ex);
            }
        }
        finally
        {
            reclaimOutgoing(__out);
        }
    }

    public String
    ice_id(java.util.Map __context)
        throws IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out = getOutgoing("ice_id", OperationMode.Nonmutating, __context);
        try
        {
            IceInternal.BasicStream __is = __out.is();
            if(!__out.invoke())
            {
                throw new UnknownUserException();
            }
            try
            {
                return __is.readString();
            }
            catch(LocalException __ex)
            {
                throw new IceInternal.NonRepeatable(__ex);
            }
        }
        finally
        {
            reclaimOutgoing(__out);
        }
    }

    public String[]
    ice_facets(java.util.Map __context)
        throws IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out = getOutgoing("ice_facets", OperationMode.Nonmutating, __context);
        try
        {
            IceInternal.BasicStream __is = __out.is();
            if(!__out.invoke())
            {
                throw new UnknownUserException();
            }
            try
            {
                return __is.readStringSeq();
            }
            catch(LocalException __ex)
            {
                throw new IceInternal.NonRepeatable(__ex);
            }
        }
        finally
        {
            reclaimOutgoing(__out);
        }
    }

    public boolean
    ice_invoke(String operation, OperationMode mode, byte[] inParams, ByteSeqHolder outParams, java.util.Map __context)
        throws IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out = getOutgoing(operation, mode, __context);
        try
        {
            IceInternal.BasicStream __os = __out.os();
            __os.writeBlob(inParams);
            boolean ok = __out.invoke();
            if(__reference.mode == IceInternal.Reference.ModeTwoway)
            {
                try
                {
                    IceInternal.BasicStream __is = __out.is();
                    int sz = __is.getReadEncapsSize();
                    outParams.value = __is.readBlob(sz);
                }
                catch(LocalException __ex)
                {
                    throw new IceInternal.NonRepeatable(__ex);
                }
            }
            return ok;
        }
        finally
        {
            reclaimOutgoing(__out);
        }
    }

    public void
    ice_invoke_async(AMI_Object_ice_invoke cb, String operation, Ice.OperationMode mode, byte[] inParams,
		     java.util.Map context)
    {
	cb.__setup(__connection, __reference, operation, mode, context);
	IceInternal.BasicStream __os = cb.__os();
	__os.writeBlob(inParams);
	cb.__invoke();
    }

    //
    // Only for use by ObjectPrx
    //
    final void
    __copyFrom(_ObjectDelM from)
    {
        //
        // No need to synchronize "from", as the delegate is immutable
        // after creation.
        //

        //
        // No need to synchronize, as this operation is only called
        // upon initialization.
        //

	assert(__reference == null);
	assert(__connection == null);

	if(from.__connection != null)
	{
	    from.__connection.incProxyCount();
	}
	
// Can not happen, __connection must be null.
/*
	if(__connection != null)
	{
	    __connection.decProxyCount();
	}
*/

        __reference = from.__reference;
        __connection = from.__connection;
    }

    protected IceInternal.Reference __reference;
    protected IceInternal.Connection __connection;

    public void
    setup(IceInternal.Reference ref)
    {
        //
        // No need to synchronize, as this operation is only called
        // upon initialization.
        //

	assert(__reference == null);
	assert(__connection == null);

        __reference = ref;

        if(__reference.reverseAdapter != null)
        {
            //
            // If we have a reverse object adapter, we use the incoming
            // connections from such object adapter.
            //
            ObjectAdapterI adapter = (ObjectAdapterI)__reference.reverseAdapter;
            IceInternal.Connection[] connections = adapter.getIncomingConnections();

            IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[connections.length];
            for(int i = 0; i < connections.length; i++)
            {
                endpoints[i] = connections[i].endpoint();
            }
            endpoints = filterEndpoints(endpoints);

            if(endpoints.length == 0)
            {
                NoEndpointException e = new NoEndpointException();
		e.proxy = __reference.toString();
		throw e;
            }

            int j;
            for(j = 0; j < connections.length; j++)
            {
                if(connections[j].endpoint().equals(endpoints[0]))
                {
                    break;
                }
            }
            assert(j < connections.length);
            __connection = connections[j];
	    __connection.incProxyCount();
        }
        else
        {
	    while(true)
	    {
		IceInternal.Endpoint[] endpoints = null;
		BooleanHolder cached = new BooleanHolder();
		cached.value = false;

		if(__reference.routerInfo != null)
		{
		    //
		    // If we route, we send everything to the router's client
		    // proxy endpoints.
		    //
		    ObjectPrx proxy = __reference.routerInfo.getClientProxy();
		    endpoints = ((ObjectPrxHelper)proxy).__reference().endpoints;
		}
		else if(__reference.endpoints.length > 0)
		{
		    endpoints = __reference.endpoints;
		}
		else if(__reference.locatorInfo != null)
		{
		    endpoints = __reference.locatorInfo.getEndpoints(__reference, cached);
		}

		IceInternal.Endpoint[] filteredEndpoints = null;
		if(endpoints != null)
		{
		    filteredEndpoints = filterEndpoints(endpoints);
		}
		if(filteredEndpoints == null || filteredEndpoints.length == 0)
		{
		    NoEndpointException e = new NoEndpointException();
		    e.proxy = __reference.toString();
		    throw e;
		}

		try
		{
		    IceInternal.OutgoingConnectionFactory factory = __reference.instance.outgoingConnectionFactory();
		    __connection = factory.create(filteredEndpoints);
		    assert(__connection != null);
		    __connection.incProxyCount();
		}
		catch(LocalException ex)
		{
		    if(__reference.routerInfo == null && __reference.endpoints.length == 0)
		    {
			assert(__reference.locatorInfo != null);
			__reference.locatorInfo.clearCache(__reference);
			
			if(cached.value)
			{
			    IceInternal.TraceLevels traceLevels = __reference.instance.traceLevels();
			    Logger logger = __reference.instance.logger();
			    
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
            // If we have a router, set the object adapter for this
            // router (if any) to the new connection, so that
            // callbacks from the router can be received over this new
            // connection.
            //
            if(__reference.routerInfo != null)
            {
                __connection.setAdapter(__reference.routerInfo.getAdapter());
            }
        }
    }

    private IceInternal.Endpoint[]
    filterEndpoints(IceInternal.Endpoint[] allEndpoints)
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

        switch(__reference.mode)
        {
            case IceInternal.Reference.ModeTwoway:
            case IceInternal.Reference.ModeOneway:
            case IceInternal.Reference.ModeBatchOneway:
            {
                //
                // Filter out datagram endpoints.
                //
                java.util.Iterator i = endpoints.iterator();
                while(i.hasNext())
                {
                    IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.next();
                    if(endpoint.datagram())
                    {
                        i.remove();
                    }
                }
                break;
            }

            case IceInternal.Reference.ModeDatagram:
            case IceInternal.Reference.ModeBatchDatagram:
            {
                //
                // Filter out non-datagram endpoints.
                //
                java.util.Iterator i = endpoints.iterator();
                while(i.hasNext())
                {
                    IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.next();
                    if(!endpoint.datagram())
                    {
                        i.remove();
                    }
                }
                break;
            }
        }

        //
        // Randomize the order of endpoints.
        //
        java.util.Collections.shuffle(endpoints);

        //
        // If a secure connection is requested, remove all non-secure
        // endpoints. Otherwise make non-secure endpoints preferred over
        // secure endpoints by partitioning the endpoint vector, so that
        // non-secure endpoints come first.
        //
        if(__reference.secure)
        {
            java.util.Iterator i = endpoints.iterator();
            while(i.hasNext())
            {
                IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.next();
                if(!endpoint.secure())
                {
                    i.remove();
                }
            }
        }
        else
        {
            java.util.Collections.sort(endpoints, __comparator);
        }

        IceInternal.Endpoint[] arr = new IceInternal.Endpoint[endpoints.size()];
        endpoints.toArray(arr);
        return arr;
    }

    private static class EndpointComparator implements java.util.Comparator
    {
        public int
        compare(java.lang.Object l, java.lang.Object r)
        {
            IceInternal.Endpoint le = (IceInternal.Endpoint)l;
            IceInternal.Endpoint re = (IceInternal.Endpoint)r;
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
    private static EndpointComparator __comparator = new EndpointComparator();

    protected IceInternal.Outgoing
    getOutgoing(String operation, OperationMode mode, java.util.Map context)
    {
        IceInternal.Outgoing out;

        synchronized(__outgoingMutex)
        {
            if(__outgoingCache == null)
            {
                out = new IceInternal.Outgoing(__connection, __reference, operation, mode, context);
            }
            else
            {
                out = __outgoingCache;
                __outgoingCache = __outgoingCache.next;
                out.reset(operation, mode, context);
            }
        }

        return out;
    }

    protected void
    reclaimOutgoing(IceInternal.Outgoing out)
    {
	//
	// TODO: Is this code necessary? Shouldn't __outgoingCache be
	// empty, i.e., shouldn't this be assert(__outgoingCache ==
	// null), just like for _incomingCache in
	// IceInternal::Connection?
	//
        synchronized(__outgoingMutex)
        {
            out.next = __outgoingCache;
            __outgoingCache = out;
        }
    }

    protected void
    finalize()
        throws Throwable
    {
	if(__connection != null)
	{
	    __connection.decProxyCount();
	}
	
        while(__outgoingCache != null)
        {
            IceInternal.Outgoing next = __outgoingCache.next;
            __outgoingCache.destroy();
            __outgoingCache.next = null;
            __outgoingCache = next;
        }
    }

    private IceInternal.Outgoing __outgoingCache;
    private java.lang.Object __outgoingMutex = new java.lang.Object();
}
