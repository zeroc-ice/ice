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
	    Reference r = (Reference)obj; // TODO argument exception
	    
	    if(r == null)
	    {
		return false;
	    }
	    
	    if(this == r)
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
	    
	    if(reverseAdapter != r.reverseAdapter)
	    {
		return false;
	    }
	    
	    if(collocationOptimization != r.collocationOptimization)
	    {
		return false;
	    }
	    
	    return true;
	}
	
	//
	// Marshal the reference
	//
	public void streamWrite(BasicStream s)
	{
	    //
	    // Don't write the identity here. Operations calling streamWrite
	    // write the identity.
	    //
	    
	    s.writeStringSeq(facet);
	    
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
	// Convert the reference to its string form
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
	    if(StringUtil.findFirstOf(id, " \t\n\r:@") != -1)
	    {
		s.Append('"');
		s.Append(id);
		s.Append('"');
	    }
	    else
	    {
		s.Append(id);
	    }
	    
	    if(facet.Count > 0)
	    {
		System.Text.StringBuilder f = new System.Text.StringBuilder();
		for(int i = 0; i < facet.Count; i++)
		{
		    f.Append(StringUtil.encodeString(facet[i], "/"));
		    if(i < facet.Count - 1)
		    {
			f.Append('/');
		    }
		}
		
		//
		// If the encoded facet string contains characters which
		// the reference parser uses as separators, then we enclose
		// the facet string in quotes.
		//
		s.Append(" -f ");
		string fs = f.ToString();
		if(StringUtil.findFirstOf(fs, " \t\n\r:@") != -1)
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
		string a = StringUtil.encodeString(adapterId, null);
		//
		// If the encoded adapter id string contains characters which
		// the reference parser uses as separators, then we enclose
		// the adapter id string in quotes.
		//
		s.Append(" @ ");
		if(StringUtil.findFirstOf(a, " \t\n\r") != -1)
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
	public readonly Ice.FacetPath facet;
	public readonly int mode;
	public readonly bool secure;
	public readonly string adapterId;
	public readonly Endpoint[] endpoints; // Actual endpoints, changed by a location forward.
	public readonly RouterInfo routerInfo; // Null if no router is used.
	public readonly LocatorInfo locatorInfo; // Null if no locator is used.
	public readonly Ice.ObjectAdapter reverseAdapter; // For reverse comm. using the adapter's incoming connections.
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
							  reverseAdapter, collocationOptimization);
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
							  reverseAdapter, collocationOptimization);
	    }
	}
	
	public Reference changeFacet(Ice.FacetPath newFacet)
	{
	    if(newFacet.Equals(facet))
	    {
		return this;
	    }
	    else
	    {
		return instance.referenceFactory().create(identity, context, newFacet, mode, secure, adapterId,
							  endpoints, routerInfo, locatorInfo,
							  reverseAdapter, collocationOptimization);
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
	    
	    //
	    // If we have a router, we also change the timeout settings on the
	    // router and the router's client proxy.
	    //
	    RouterInfo newRouterInfo = null;
	    if(routerInfo != null)
	    {
		try
		{
		    Ice.RouterPrx newRouter = Ice.RouterPrxHelper.uncheckedCast(routerInfo.getRouter().ice_timeout(newTimeout));
		    Ice.ObjectPrx newClientProxy = routerInfo.getClientProxy().ice_timeout(newTimeout);
		    newRouterInfo = instance.routerManager().get(newRouter);
		    newRouterInfo.setClientProxy(newClientProxy);
		}
		catch(Ice.NoEndpointException)
		{
		    // Ignore non-existing client proxies.
		}
	    }
	    
	    //
	    // If we have a locator, we also change the timeout settings
	    // on the locator.
	    //
	    LocatorInfo newLocatorInfo = null;
	    if(locatorInfo != null)
	    {
		Ice.LocatorPrx newLocator = Ice.LocatorPrxHelper.uncheckedCast(locatorInfo.getLocator().ice_timeout(newTimeout));
		newLocatorInfo = instance.locatorManager().get(newLocator);
	    }
	    
	    return instance.referenceFactory().create(identity, context, facet, mode, secure, adapterId,
						      newEndpoints, newRouterInfo, newLocatorInfo,
						      reverseAdapter, collocationOptimization);
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
							  reverseAdapter, collocationOptimization);
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
							  reverseAdapter, collocationOptimization);
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
	    
	    //
	    // If we have a router, we also change the compress settings on the
	    // router and the router's client proxy.
	    //
	    RouterInfo newRouterInfo = null;
	    if(routerInfo != null)
	    {
		try
		{
		    Ice.RouterPrx newRouter = Ice.RouterPrxHelper.uncheckedCast(routerInfo.getRouter().ice_compress(newCompress));
		    Ice.ObjectPrx newClientProxy = routerInfo.getClientProxy().ice_compress(newCompress);
		    newRouterInfo = instance.routerManager().get(newRouter);
		    newRouterInfo.setClientProxy(newClientProxy);
		}
		catch(Ice.NoEndpointException)
		{
		    // Ignore non-existing client proxies.
		}
	    }
	    
	    //
	    // If we have a locator, we also change the compress settings
	    // on the locator.
	    //
	    LocatorInfo newLocatorInfo = null;
	    if(locatorInfo != null)
	    {
		Ice.LocatorPrx newLocator = Ice.LocatorPrxHelper.uncheckedCast(
						locatorInfo.getLocator().ice_compress(newCompress));
		newLocatorInfo = instance.locatorManager().get(newLocator);
	    }
	    
	    return instance.referenceFactory().create(identity, context, facet, mode, secure, adapterId,
						      newEndpoints, newRouterInfo, newLocatorInfo,
						      reverseAdapter, collocationOptimization);
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
							  reverseAdapter, collocationOptimization);
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
							  reverseAdapter, collocationOptimization);
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
							  reverseAdapter, collocationOptimization);
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
							  reverseAdapter, collocationOptimization);
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
							  reverseAdapter, newCollocationOptimization);
	    }
	}
	
	public Reference changeDefault()
	{
	    RouterInfo routerInfo = instance.routerManager().get(instance.referenceFactory().getDefaultRouter());
	    LocatorInfo locatorInfo = instance.locatorManager().get(instance.referenceFactory().getDefaultLocator());
	    
	    return instance.referenceFactory().create(identity, context, new Ice.FacetPath(), ModeTwoway, false,
						      adapterId, endpoints, routerInfo, locatorInfo, null, true);
	}
	
	//
	// Only for use by ReferenceFactory
	//
	internal Reference(Instance inst, Ice.Identity ident, Ice.Context ctx, Ice.FacetPath fac, int md,
		           bool sec, string adptId, Endpoint[] endpts, RouterInfo rtrInfo, LocatorInfo locInfo,
		           Ice.ObjectAdapter rvAdapter, bool collocationOpt)
	{
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
	    reverseAdapter = rvAdapter;
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
	
	private bool compare(Endpoint[] arr1, Endpoint[] arr2)
	{
	    if(arr1 == arr2)
	    {
		return true;
	    }
	    
	    if(arr1.Length == arr2.Length)
	    {
		for(int i = 0; i < arr1.Length; i++)
		{
		    if(!arr1[i].Equals(arr2[i]))
		    {
			return false;
		    }
		}
		
		return true;
	    }
	    
	    return false;
	}
    }

}
