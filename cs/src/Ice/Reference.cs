// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;

    public sealed class Reference
    {
	public const int ModeTwoway = 0;
	public const int ModeOneway = 1;
	public const int ModeBatchOneway = 2;
	public const int ModeDatagram = 3;
	public const int ModeBatchDatagram = 4;
	public static readonly int ModeLast = ModeBatchDatagram;
	
	public override int GetHashCode()
	{
	    return hashValue;
	}
	
	public override bool Equals(object obj)
	{
            if(obj == null)
            {
                return false;
            }

	    if(!(obj is Reference))
	    {
		throw new System.ArgumentException("expected argument of type `Reference'", "obj");
	    }

            Reference r = (Reference)obj;
	    
	    if(ReferenceEquals(this, r))
	    {
		return true;
	    }
	    
	    if(!identity.category.Equals(r.identity.category))
	    {
		return false;
	    }
	    
	    if(!identity.name.Equals(r.identity.name))
	    {
		return false;
	    }
	    
	    if(!context.Equals(r.context))
	    {
		return false;
	    }
	    
	    if(!facet.Equals(r.facet))
	    {
		return false;
	    }
	    
	    if(mode != r.mode)
	    {
		return false;
	    }
	    
	    if(secure != r.secure)
	    {
		return false;
	    }
	    
	    if(!adapterId.Equals(r.adapterId))
	    {
		return false;
	    }
	    
	    if(!compare(endpoints, r.endpoints))
	    {
		return false;
	    }
	    
	    if(routerInfo != r.routerInfo)
	    {
		return false;
	    }
	    
	    if(routerInfo != null && r.routerInfo != null && !routerInfo.Equals(r.routerInfo))
	    {
		return false;
	    }
	    
	    if(locatorInfo != r.locatorInfo)
	    {
		return false;
	    }
	    
	    if(locatorInfo != null && r.locatorInfo != null && !locatorInfo.Equals(r.locatorInfo))
	    {
		return false;
	    }
	    
	    if(!compare(fixedConnections, r.fixedConnections))
	    {
		return false;
	    }
	    
	    return true;
	}
	
	//
	// Marshal the reference.
	//
	public void streamWrite(BasicStream s)
	{
	    //
	    // Don't write the identity here. Operations calling streamWrite
	    // write the identity.
	    //

	    //
	    // For compatibility with the old FacetPath.
	    //
	    if(facet.Length == 0)
	    {
	        s.writeStringSeq(null);
	    }
	    else
	    {
	        string[] facetPath = { facet };
		s.writeStringSeq(facetPath);
	    }
	    
	    s.writeByte((byte)mode);
	    
	    s.writeBool(secure);
	    
	    s.writeSize(endpoints.Length);
	    
	    if(endpoints.Length > 0)
	    {
		Debug.Assert(adapterId.Equals(""));
		
		for(int i = 0; i < endpoints.Length; i++)
		{
		    endpoints[i].streamWrite(s);
		}
	    }
	    else
	    {
		s.writeString(adapterId);
	    }
	}
	
	//
	// Convert the reference to its string form.
	//
	public override string ToString()
	{
	    System.Text.StringBuilder s = new System.Text.StringBuilder();
	    
	    //
	    // If the encoded identity string contains characters which
	    // the reference parser uses as separators, then we enclose
	    // the identity string in quotes.
	    //
	    string id = Ice.Util.identityToString(identity);
	    if(IceUtil.StringUtil.findFirstOf(id, " \t\n\r:@") != -1)
	    {
		s.Append('"');
		s.Append(id);
		s.Append('"');
	    }
	    else
	    {
		s.Append(id);
	    }
	    
	    if(facet.Length > 0)
	    {
		//
		// If the encoded facet string contains characters which
		// the reference parser uses as separators, then we enclose
		// the facet string in quotes.
		//
		s.Append(" -f ");
		string fs = IceUtil.StringUtil.escapeString(facet, "");
		if(IceUtil.StringUtil.findFirstOf(fs, " \t\n\r:@") != -1)
		{
		    s.Append('"');
		    s.Append(fs);
		    s.Append('"');
		}
		else
		{
		    s.Append(fs);
		}
	    }
	    
	    switch(mode)
	    {
		case ModeTwoway: 
		{
		    s.Append(" -t");
		    break;
		}
		
		case ModeOneway: 
		{
		    s.Append(" -o");
		    break;
		}
		
		case ModeBatchOneway: 
		{
		    s.Append(" -O");
		    break;
		}
		
		case ModeDatagram: 
		{
		    s.Append(" -d");
		    break;
		}
		
		case ModeBatchDatagram: 
		{
		    s.Append(" -D");
		    break;
		}
	    }
	    
	    if(secure)
	    {
		s.Append(" -s");
	    }
	    
	    if(endpoints.Length > 0)
	    {
		Debug.Assert(adapterId.Equals(""));
		
		for(int i = 0; i < endpoints.Length; i++)
		{
		    string endp = endpoints[i].ToString();
		    if(endp != null && endp.Length > 0)
		    {
			s.Append(':');
			s.Append(endp);
		    }
		}
	    }
	    else
	    {
		string a = IceUtil.StringUtil.escapeString(adapterId, null);
		//
		// If the encoded adapter id string contains characters which
		// the reference parser uses as separators, then we enclose
		// the adapter id string in quotes.
		//
		s.Append(" @ ");
		if(IceUtil.StringUtil.findFirstOf(a, " \t\n\r") != -1)
		{
		    s.Append('"');
		    s.Append(a);
		    s.Append('"');
		}
		else
		{
		    s.Append(a);
		}
	    }
	    
	    return s.ToString();
	}
	
	//
	// All members are readonly, because References are immutable.
	//
	public readonly Instance instance;
	public readonly Ice.Identity identity;
	public readonly Ice.Context context;
	public readonly string facet;
	public readonly int mode;
	public readonly bool secure;
	public readonly string adapterId;
	public readonly Endpoint[] endpoints;
	public readonly RouterInfo routerInfo; // Null if no router is used.
	public readonly LocatorInfo locatorInfo; // Null if no locator is used.
	public readonly Ice.ConnectionI[] fixedConnections; // // For using fixed connections, otherwise empty.
	public readonly bool collocationOptimization;
	public readonly int hashValue;
	
	//
	// Get a new reference, based on the existing one, overwriting
	// certain values.
	//
	public Reference changeIdentity(Ice.Identity newIdentity)
	{
	    if(newIdentity.Equals(identity))
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(newIdentity, context, facet, mode, secure,
							  adapterId, endpoints, routerInfo, locatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeContext(Ice.Context newContext)
	{
	    if(newContext.Equals(context))
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, newContext, facet, mode, secure, adapterId,
							  endpoints, routerInfo, locatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeFacet(string newFacet)
	{
	    if(newFacet.Equals(facet))
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, newFacet, mode, secure, adapterId,
							  endpoints, routerInfo, locatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeTimeout(int newTimeout)
	{
	    //
	    // We change the timeout settings in all endpoints.
	    //
	    Endpoint[] newEndpoints = new Endpoint[endpoints.Length];
	    for(int i = 0; i < endpoints.Length; i++)
	    {
		newEndpoints[i] = endpoints[i].timeout(newTimeout);
	    }
	    
	    return instance.referenceFactory().create(identity, context, facet, mode, secure, adapterId,
						      newEndpoints, routerInfo, locatorInfo,
						      fixedConnections, collocationOptimization);
	}
	
	public Reference changeMode(int newMode)
	{
	    if(newMode == mode)
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, facet, newMode, secure, adapterId,
							  endpoints, routerInfo, locatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeSecure(bool newSecure)
	{
	    if(newSecure == secure)
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, facet, mode, newSecure, adapterId,
							  endpoints, routerInfo, locatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeCompress(bool newCompress)
	{
	    //
	    // We change the compress settings in all endpoints.
	    //
	    Endpoint[] newEndpoints = new Endpoint[endpoints.Length];
	    for(int i = 0; i < endpoints.Length; i++)
	    {
		newEndpoints[i] = endpoints[i].compress(newCompress);
	    }
	    
	    return instance.referenceFactory().create(identity, context, facet, mode, secure, adapterId,
						      newEndpoints, routerInfo, locatorInfo,
						      fixedConnections, collocationOptimization);
	}
	
	public Reference changeAdapterId(string newAdapterId)
	{
	    if(adapterId.Equals(newAdapterId))
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, facet, mode, secure, newAdapterId,
							  endpoints, routerInfo, locatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeEndpoints(Endpoint[] newEndpoints)
	{
	    if(compare(newEndpoints, endpoints))
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, facet, mode, secure, adapterId,
							  newEndpoints, routerInfo, locatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeRouter(Ice.RouterPrx newRouter)
	{
	    RouterInfo newRouterInfo = instance.routerManager().get(newRouter);
	    
	    if((routerInfo == newRouterInfo) ||
		(routerInfo != null && newRouterInfo != null && newRouterInfo.Equals(routerInfo)))
	    {
		    return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, facet, mode, secure, adapterId,
							  endpoints, newRouterInfo, locatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeLocator(Ice.LocatorPrx newLocator)
	{
	    LocatorInfo newLocatorInfo = instance.locatorManager().get(newLocator);
	    
	    if((locatorInfo == newLocatorInfo) ||
		(locatorInfo != null && newLocatorInfo != null && newLocatorInfo.Equals(locatorInfo)))
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, facet, mode, secure, adapterId,
							  endpoints, routerInfo, newLocatorInfo,
							  fixedConnections, collocationOptimization);
	    }
	}
	
	public Reference changeCollocationOptimization(bool newCollocationOptimization)
	{
	    if(newCollocationOptimization == collocationOptimization)
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, facet, mode, secure, adapterId,
							  endpoints, routerInfo, locatorInfo,
							  fixedConnections, newCollocationOptimization);
	    }
	}
	
	public Reference changeDefault()
	{
	    RouterInfo routerInfo = instance.routerManager().get(instance.referenceFactory().getDefaultRouter());
	    LocatorInfo locatorInfo = instance.locatorManager().get(instance.referenceFactory().getDefaultLocator());
	    
	    return instance.referenceFactory().create(identity, context, "", ModeTwoway, false,
						      adapterId, endpoints, routerInfo, locatorInfo, null, true);
	}
	
	//
	// Get a suitable connection for this reference.
	//
	public Ice.ConnectionI getConnection(out bool compress)
	{
	    Ice.ConnectionI connection;

	    //
	    // If we have a fixed connection, we return such fixed connection.
	    //
	    if(fixedConnections.Length > 0)
	    {
		Ice.ConnectionI[] filteredConns = filterConnections(fixedConnections);
		if(filteredConns.Length == 0)
		{
		    Ice.NoEndpointException e = new Ice.NoEndpointException();
		    e.proxy = ToString();
		    throw e;
		}

		connection = filteredConns[0];
		compress = connection.endpoint().compress();
	    }
	    else
	    {
		while(true)
		{
		    bool cached = false;
		    Endpoint[] endpts = null;

		    if(routerInfo != null)
		    {
			//
			// If we route, we send everything to the router's client
			// proxy endpoints.
			//
			Ice.ObjectPrx proxy = routerInfo.getClientProxy();
			endpts = ((Ice.ObjectPrxHelperBase)proxy).__reference().endpoints;
		    }
		    else if(endpoints.Length > 0)
		    {
			endpts = endpoints;
		    }
		    else if(locatorInfo != null)
		    {
			endpts = locatorInfo.getEndpoints(this, out cached);
		    }

		    Endpoint[] filteredEndpts = filterEndpoints(endpts);
		    if(filteredEndpts == null || filteredEndpts.Length == 0)
		    {
			Ice.NoEndpointException e = new Ice.NoEndpointException();
			e.proxy = ToString();
			throw e;
		    }

		    try
		    {
			OutgoingConnectionFactory factory = instance.outgoingConnectionFactory();
			connection = factory.create(filteredEndpts, out compress);
			Debug.Assert(connection != null);
		    }
		    catch(Ice.LocalException ex)
		    {
			if(routerInfo == null && endpoints.Length == 0)
			{
			    Debug.Assert(locatorInfo != null);
			    locatorInfo.clearCache(this);
			    
			    if(cached)
			    {
				TraceLevels traceLevels = instance.traceLevels();
				Ice.Logger logger = instance.logger();
				
				if(traceLevels.retry >= 2)
				{
				    string s = "connection to cached endpoints failed\n" +
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
		if(routerInfo != null)
		{
		    connection.setAdapter(routerInfo.getAdapter());
		}
	    }

	    Debug.Assert(connection != null);
	    return connection;
	}

	//
	// Filter endpoints based on criteria from this reference.
	//
	public Endpoint[]
	filterEndpoints(Endpoint[] allEndpoints)
	{
	    ArrayList endpoints = new ArrayList();

	    //
	    // Filter out unknown endpoints.
	    //
	    for(int i = 0; i < allEndpoints.Length; i++)
	    {
		if(!allEndpoints[i].unknown())
		{
		    endpoints.Add(allEndpoints[i]);
		}
	    }

	    switch(mode)
	    {
		case Reference.ModeTwoway:
		case Reference.ModeOneway:
		case Reference.ModeBatchOneway:
		{
		    //
		    // Filter out datagram endpoints.
		    //
		    ArrayList tmp = new ArrayList();
		    foreach(Endpoint endpoint in endpoints)
		    {
			if(!endpoint.datagram())
			{
			    tmp.Add(endpoint);
			}
		    }
		    endpoints = tmp;
		    break;
		}

		case Reference.ModeDatagram:
		case Reference.ModeBatchDatagram:
		{
		    //
		    // Filter out non-datagram endpoints.
		    //
		    ArrayList tmp = new ArrayList();
		    foreach(Endpoint endpoint in endpoints)
		    {
			if(endpoint.datagram())
			{
			    tmp.Add(endpoint);
			}
		    }
		    endpoints = tmp;
		    break;
		}
	    }

	    //
	    // Randomize the order of endpoints.
	    //
	    for(int i = 0; i < endpoints.Count - 2; ++i)
	    {
		int r = _rand.Next(endpoints.Count - i) + i;
		Debug.Assert(r >= i && r < endpoints.Count);
		if(r != i)
		{
		    object tmp = endpoints[i];
		    endpoints[i] = endpoints[r];
		    endpoints[r] = tmp;
		}
	    }

	    //
	    // If a secure connection is requested, remove all non-secure
	    // endpoints. Otherwise make non-secure endpoints preferred over
	    // secure endpoints by partitioning the endpoint vector, so that
	    // non-secure endpoints come first.
	    //
	    if(secure)
	    {
		ArrayList tmp = new ArrayList();
		foreach(Endpoint endpoint in endpoints)
		{
		    if(endpoint.secure())
		    {
			tmp.Add(endpoint);
		    }
		}
		endpoints = tmp;
	    }
	    else
	    {
		endpoints.Sort(_endpointComparator);
	    }
	    
	    Endpoint[] arr = new Endpoint[endpoints.Count];
	    if(arr.Length != 0)
	    {
		endpoints.CopyTo(arr);
	    }
	    return arr;
	}

	private class EndpointComparator : IComparer
	{
	    public int Compare(object l, object r)
	    {
		IceInternal.Endpoint le = (IceInternal.Endpoint)l;
		IceInternal.Endpoint re = (IceInternal.Endpoint)r;
		bool ls = le.secure();
		bool rs = re.secure();
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

	//
	// Filter connections based on criteria from this reference.
	//
	public Ice.ConnectionI[]
	filterConnections(Ice.ConnectionI[] allConnections)
	{
	    ArrayList connections = new ArrayList();

	    switch(mode)
	    {
		case Reference.ModeTwoway:
		case Reference.ModeOneway:
		case Reference.ModeBatchOneway:
		{
		    //
		    // Filter out datagram endpoints.
		    //
		    for(int i = 0; i < allConnections.Length; ++i)
		    {
			if(!allConnections[i].endpoint().datagram())
			{
			    connections.Add(allConnections[i]);
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
		    for(int i = 0; i < allConnections.Length; ++i)
		    {
			if(allConnections[i].endpoint().datagram())
			{
			    connections.Add(allConnections[i]);
			}
		    }

		    break;
		}
	    }

	    //
	    // Randomize the order of connections.
	    //
	    for(int i = 0; i < connections.Count - 2; ++i)
	    {
		int r = _rand.Next(connections.Count - i) + i;
		Debug.Assert(r >= i && r < connections.Count);
		if(r != i)
		{
		    object tmp = connections[i];
		    connections[i] = connections[r];
		    connections[r] = tmp;
		}
	    }

	    //
	    // If a secure connection is requested, remove all non-secure
	    // endpoints. Otherwise make non-secure endpoints preferred over
	    // secure endpoints by partitioning the endpoint vector, so that
	    // non-secure endpoints come first.
	    //
	    if(secure)
	    {
		ArrayList tmp = new ArrayList();
		foreach(Ice.ConnectionI connection in connections)
		{
		    if(connection.endpoint().secure())
		    {
			tmp.Add(connection);
		    }
		}
		connections = tmp;
	    }
	    else
	    {
		connections.Sort(_connectionComparator);
	    }
	    
	    Ice.ConnectionI[] arr = new Ice.ConnectionI[connections.Count];
	    if(arr.Length != 0)
	    {
		connections.CopyTo(arr);
	    }
	    return arr;
	}

	private class ConnectionComparator : IComparer
	{
	    public int Compare(object l, object r)
	    {
		Ice.ConnectionI lc = (Ice.ConnectionI)l;
		Ice.ConnectionI rc = (Ice.ConnectionI)r;
		bool ls = lc.endpoint().secure();
		bool rs = rc.endpoint().secure();
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
	
	private static ConnectionComparator _connectionComparator = new ConnectionComparator();

	private static System.Random _rand = new System.Random(unchecked((int)System.DateTime.Now.Ticks));

	//
	// Only for use by ReferenceFactory
	//
	internal Reference(Instance inst, Ice.Identity ident, Ice.Context ctx, string fac, int md,
		           bool sec, string adptId, Endpoint[] endpts, RouterInfo rtrInfo, LocatorInfo locInfo,
		           Ice.ConnectionI[] fixedConns, bool collocationOpt)
	{
	    //
	    // Validate string arguments.
	    //
	    Debug.Assert(ident.name != null);
	    Debug.Assert(ident.category != null);
	    Debug.Assert(fac != null);
	    Debug.Assert(adptId != null);

	    //
	    // It's either adapter id or endpoints, it can't be both.
	    //
	    Debug.Assert(!(!adptId.Equals("") && !(endpts.Length == 0)));
	    
	    instance = inst;
	    identity = ident;
	    context = ctx;
	    facet = fac;
	    mode = md;
	    secure = sec;
	    adapterId = adptId;
	    endpoints = endpts;
	    routerInfo = rtrInfo;
	    locatorInfo = locInfo;
	    fixedConnections = fixedConns;
	    collocationOptimization = collocationOpt;
	    
	    int h = 0;
	    
	    h = 5 * h + identity.GetHashCode();
	    h = 5 * h + context.GetHashCode();
	    h = 5 * h + facet.GetHashCode();
	    h = 5 * h + mode.GetHashCode();
	    h = 5 * h + secure.GetHashCode();
	    
	    //
	    // TODO: Should we also take the endpoints into account for hash
	    // calculation? Perhaps not, the code above should be good enough
	    // for a good hash value.
	    //
	    
	    hashValue = h;
	}

	private bool compare(object[] arr1, object[] arr2)
	{
	    if(arr1 == null)
	    {
		return arr2 == null;
	    }
	    if(arr2 == null)
	    {
	        return false;
	    }
	    if(arr1.Length != arr2.Length)
	    {
	        return false;
	    }
	    for(int i = 0; i < arr1.Length; i++)
	    {
		if(!arr1[i].Equals(arr2[i]))
		{
		    return false;
		}
	    }
	    return true;
	}	
    }

}
