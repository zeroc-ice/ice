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
namespace Ice
{

    using System.Collections;
    using System.Diagnostics;
    using IceUtil;
	    
    public class Object_DelM : Object_Del
    {
	public Object_DelM()
	{
	    __outgoingMutex = new System.Object();
	}

	public virtual bool ice_isA(string __id, Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_isA", OperationMode.Nonmutating, __context);
	    try
	    {
		IceInternal.BasicStream __is = __out.istr();
		IceInternal.BasicStream __os = __out.ostr();
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
	
	public virtual void ice_ping(Ice.Context __context)
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
	
	public virtual StringSeq ice_ids(Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_ids", OperationMode.Nonmutating, __context);
	    try
	    {
		IceInternal.BasicStream __is = __out.istr();
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
	
	public virtual string ice_id(Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_id", OperationMode.Nonmutating, __context);
	    try
	    {
		IceInternal.BasicStream __is = __out.istr();
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
	
	public virtual FacetPath ice_facets(Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_facets", OperationMode.Nonmutating, __context);
	    try
	    {
		IceInternal.BasicStream __is = __out.istr();
		if(!__out.invoke())
		{
		    throw new UnknownUserException();
		}
		try
		{
		    return __is.readFacetPath();
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
	
	public virtual bool ice_invoke(string operation, OperationMode mode, ByteSeq inParams, out ByteSeq outParams,
	                               Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing(operation, mode, __context);
	    try
	    {
		IceInternal.BasicStream __os = __out.ostr();
		__os.writeBlob(inParams);
		bool ok = __out.invoke();
		outParams = null;
		if(__reference.mode == IceInternal.Reference.ModeTwoway)
		{
		    try
		    {
			IceInternal.BasicStream __is = __out.istr();
			int sz = __is.getReadEncapsSize();
			outParams = __is.readBlob(sz);
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
	
	public virtual void ice_invoke_async(AMI_Object_ice_invoke cb, string operation, Ice.OperationMode mode,
	                                     Ice.ByteSeq inParams, Ice.Context context)
	{
	    cb.__setup(__connection, __reference, operation, mode, context);
	    IceInternal.BasicStream __os = cb.__os();
	    __os.writeBlob(inParams);
	    cb.__invoke();
	}
	
	//
	// Only for use by ObjectPrx
	//
	internal void __copyFrom(Object_DelM from)
	{
	    //
	    // No need to synchronize "from", as the delegate is immutable
	    // after creation.
	    //
	    
	    //
	    // No need to synchronize, as this operation is only called
	    // upon initialization.
	    //
	    
	    Debug.Assert(__reference == null);
	    Debug.Assert(__connection == null);
	    
	    if(from.__connection != null)
	    {
		from.__connection.incProxyCount();
	    }
	    
	    // Cannot happen, __connection must be null.
	    /*
	    if(__connection != null)
	    {
		__connection.decProxyCount();
	    }*/
	    
	    __reference = from.__reference;
	    __connection = from.__connection;
	}
	
	protected internal IceInternal.Reference __reference;
	protected internal IceInternal.Connection __connection;
	
	public virtual void setup(IceInternal.Reference rf)
	{
	    //
	    // No need to synchronize, as this operation is only called
	    // upon initialization.
	    //
	    
	    Debug.Assert(__reference == null);
	    Debug.Assert(__connection == null);
	    
	    __reference = rf;
	    
	    if(__reference.reverseAdapter != null)
	    {
		//
		// If we have a reverse object adapter, we use the incoming
		// connections from such object adapter.
		//
		ObjectAdapterI adapter = (ObjectAdapterI)__reference.reverseAdapter;
		IceInternal.Connection[] connections = adapter.getIncomingConnections();
		
		IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[connections.Length];
		for (int i = 0; i < connections.Length; i++)
		{
		    endpoints[i] = connections[i].endpoint();
		}
		endpoints = filterEndpoints(endpoints);
		
		if(endpoints.Length == 0)
		{
		    NoEndpointException e = new NoEndpointException();
		    e.proxy = __reference.ToString();
		    throw e;
		}
		
		int j;
		for (j = 0; j < connections.Length; j++)
		{
		    if(connections[j].endpoint().Equals(endpoints[0]))
		    {
			break;
		    }
		}
		Debug.Assert(j < connections.Length);
		__connection = connections[j];
		__connection.incProxyCount();
	    }
	    else
	    {
		while(true)
		{
		    IceInternal.Endpoint[] endpoints = null;
		    bool cached = false;
		    
		    if(__reference.routerInfo != null)
		    {
			//
			// If we route, we send everything to the router's client
			// proxy endpoints.
			//
			ObjectPrx proxy = __reference.routerInfo.getClientProxy();
			endpoints = ((ObjectPrxHelper)proxy).__reference().endpoints;
		    }
		    else if(__reference.endpoints.Length > 0)
		    {
			endpoints = __reference.endpoints;
		    }
		    else if(__reference.locatorInfo != null)
		    {
			endpoints = __reference.locatorInfo.getEndpoints(__reference, out cached);
		    }
		    
		    IceInternal.Endpoint[] filteredEndpoints = null;
		    if(endpoints != null)
		    {
			filteredEndpoints = filterEndpoints(endpoints);
		    }
		    if(filteredEndpoints == null || filteredEndpoints.Length == 0)
		    {
			NoEndpointException e = new NoEndpointException();
			e.proxy = __reference.ToString();
			throw e;
		    }
		    
		    try
		    {
			IceInternal.OutgoingConnectionFactory factory = __reference.instance.outgoingConnectionFactory();
			__connection = factory.create(filteredEndpoints);
			Debug.Assert(__connection != null);
			__connection.incProxyCount();
		    }
		    catch(LocalException ex)
		    {
			if(__reference.routerInfo == null && __reference.endpoints.Length == 0)
			{
			    Debug.Assert(__reference.locatorInfo != null);
			    __reference.locatorInfo.clearCache(__reference);
			    
			    if(cached)
			    {
				IceInternal.TraceLevels traceLevels = __reference.instance.traceLevels();
				Logger logger = __reference.instance.logger();
				
				if(traceLevels.retry >= 2)
				{
				    string s = "connection to cached endpoints failed\n"
					       + "removing endpoints from cache and trying one more time\n" + ex;
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
	
	private IceInternal.Endpoint[] filterEndpoints(IceInternal.Endpoint[] allEndpoints)
	{
	    LinkedList endpoints = new LinkedList();
	    
	    //
	    // Filter out unknown endpoints.
	    //
	    foreach(IceInternal.Endpoint endpoint in allEndpoints)
	    {
		if(!endpoint.unknown())
		{
		    endpoints.Add(endpoint);
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
		    LinkedList.Enumerator i = (LinkedList.Enumerator)endpoints.GetEnumerator();
		    while(i.MoveNext())
		    {
			IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.Current;
			if(endpoint.datagram())
			{
			    i.Remove();
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
		    LinkedList.Enumerator i = (LinkedList.Enumerator)endpoints.GetEnumerator();
		    while(i.MoveNext())
		    {
			IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.Current;
			if(!endpoint.datagram())
			{
			    i.Remove();
			}
		    }
		    break;
		}
	    }
	    
	    // TODO: Why first shuffle and then sort?

	    //
	    // Randomize the order of endpoints.
	    //
	    //UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
	    //java.util.Collections.shuffle(endpoints); // TODO
	    
	    //
	    // If a secure connection is requested, remove all non-secure
	    // endpoints. Otherwise make non-secure endpoints preferred over
	    // secure endpoints by partitioning the endpoint vector, so that
	    // non-secure endpoints come first.
	    //
	    if(__reference.secure)
	    {
		LinkedList.Enumerator i = (LinkedList.Enumerator)endpoints.GetEnumerator();
		while(i.MoveNext())
		{
		    IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.Current;
		    if(!endpoint.secure())
		    {
			i.Remove();
		    }
		}
	    }
	    else
	    {
		//UPGRADE_TODO: Field java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
		// java.util.Collections.sort(endpoints, __comparator); // TODO
	    }
	    
	    IceInternal.Endpoint[] arr = new IceInternal.Endpoint[endpoints.Count];
	    endpoints.CopyTo(arr, 0);
	    return arr;
	}
	
    /*	// TODO
	//UPGRADE_TODO: Interface java.util was not converted. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1095"'
	private class EndpointComparator : java.util.Comparator
	{
	    public virtual int compare(System.Object l, System.Object r)
	    {
		IceInternal.Endpoint le = (IceInternal.Endpoint) l;
		IceInternal.Endpoint re = (IceInternal.Endpoint) r;
		bool ls = le.secure();
		bool rs = re.secure();
		if((ls && rs) || (!ls && !rs))
		{
		    return 0;
		}
		else if(!ls && rs)
		{
		    return - 1;
		}
		else
		{
		    return 1;
		}
	    }
	}
    */

	//UPGRADE_NOTE: The initialization of  '__comparator' was moved to static method 'Ice._ObjectDelM'. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1005"'
	// private static EndpointComparator __comparator;
	
	protected internal virtual IceInternal.Outgoing getOutgoing(string operation, OperationMode mode,
	                                                            Ice.Context context)
	{
	    IceInternal.Outgoing outg;
	    
	    lock(__outgoingMutex)
	    {
		if(__outgoingCache == null)
		{
		    outg = new IceInternal.Outgoing(__connection, __reference, operation, mode, context);
		}
		else
		{
		    outg = __outgoingCache;
		    __outgoingCache = __outgoingCache.next;
		    outg.reset(operation, mode, context);
		}
	    }
	    
	    return outg;
	}
	
	protected internal virtual void reclaimOutgoing(IceInternal.Outgoing outg)
	{
	    //
	    // TODO: Is this code necessary? Shouldn't __outgoingCache be
	    // empty, i.e., shouldn't this be Debug.Assert(__outgoingCache ==
	    // null), just like for _incomingCache in
	    // IceInternal::Connection?
	    //
	    lock(__outgoingMutex)
	    {
		outg.next = __outgoingCache;
		__outgoingCache = outg;
	    }
	}
	
	~Object_DelM()
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
	private System.Object __outgoingMutex;
	static Object_DelM()
	{
	    // __comparator = new EndpointComparator(); // TODO
	}
    }

}
