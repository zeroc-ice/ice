// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;

namespace IceInternal
{

    public class ReferenceFactory
    {
	public Reference
	create(Ice.Identity ident,
	       Ice.Context context,
	       string facet,
	       Reference.Mode mode,
	       bool secure,
	       EndpointI[] endpoints,
	       RouterInfo routerInfo,
	       bool collocationOptimization)
	{
	    lock(this)
	    {

		if(instance_ == null)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}

		if(ident.name.Length == 0 && ident.category.Length == 0)
		{
		    return null;
		}

		//
		// Create new reference
		//
		DirectReference @ref = new DirectReference(instance_, _communicator, ident, context, facet, mode,
							   secure, endpoints, routerInfo, collocationOptimization);
		return updateCache(@ref);
	    }
	}

	public Reference create(Ice.Identity ident,
	                        Ice.Context context,
	                        string facet,
	                        Reference.Mode mode,
	                        bool secure,
	                        string adapterId,
	                        RouterInfo routerInfo,
	                        LocatorInfo locatorInfo,
	                        bool collocationOptimization,
				int locatorCacheTimeout)
	{
	    lock(this)
	    {
		if(instance_ == null)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}

		if(ident.name.Length == 0 && ident.category.Length == 0)
		{
		    return null;
		}

		//
		// Create new reference
		//
		IndirectReference @ref = new IndirectReference(instance_,  _communicator, ident, context, facet, mode,
							       secure, adapterId, routerInfo, locatorInfo,
							       collocationOptimization, locatorCacheTimeout);
		return updateCache(@ref);
	    }
	}

	public Reference create(Ice.Identity ident,
	                        Ice.Context context,
	                        string facet,
	                        Reference.Mode mode,
	                        Ice.ConnectionI[] fixedConnections)
	{
	    lock(this)
	    {
		if(instance_ == null)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}

		if(ident.name.Length == 0 && ident.category.Length == 0)
		{
		    return null;
		}

		//
		// Create new reference
		//
		FixedReference @ref = new FixedReference(instance_, _communicator, ident, context, facet, mode, fixedConnections);
		return updateCache(@ref);
	    }
	}

	public Reference copy(Reference r)
	{
	    lock(this)
	    {
		if(instance_ == null)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}

		Ice.Identity ident = r.getIdentity();
		if(ident.name.Length == 0 && ident.category.Length == 0)
		{
		    return null;
		}
		return (Reference)r.Clone();
	    }
	}

	public Reference create(string s)
	{
	    if(s.Length == 0)
	    {
		return null;
	    }

	    const string delim = " \t\n\r";

	    int beg;
	    int end = 0;

	    beg = IceUtil.StringUtil.findFirstNotOf(s, delim, end);
	    if(beg == -1)
	    {
		Ice.ProxyParseException e = new Ice.ProxyParseException();
		e.str = s;
		throw e;
	    }

	    //
	    // Extract the identity, which may be enclosed in single
	    // or double quotation marks.
	    //
	    string idstr = null;
	    end = IceUtil.StringUtil.checkQuote(s, beg);
	    if(end == -1)
	    {
		Ice.ProxyParseException e = new Ice.ProxyParseException();
		e.str = s;
		throw e;
	    }
	    else if(end == 0)
	    {
		end = IceUtil.StringUtil.findFirstOf(s, delim + ":@", beg);
		if(end == -1)
		{
		    end = s.Length;
		}
		idstr = s.Substring(beg, end - beg);
	    }
	    else
	    {
		beg++; // Skip leading quote
		idstr = s.Substring(beg, end - beg);
		end++; // Skip trailing quote
	    }

	    if(beg == end)
	    {
		Ice.ProxyParseException e = new Ice.ProxyParseException();
		e.str = s;
		throw e;
	    }

	    //
	    // Parsing the identity may raise IdentityParseException.
	    //
	    Ice.Identity ident = instance_.stringToIdentity(idstr);

	    if(ident.name.Length == 0)
	    {
		//
		// An identity with an empty name and a non-empty
		// category is illegal.
		//
		if(ident.category.Length > 0)
		{
		    Ice.IllegalIdentityException e = new Ice.IllegalIdentityException();
		    e.id = ident;
		    throw e;
		}
		//
		// Treat a stringified proxy containing two double
		// quotes ("") the same as an empty string, i.e.,
		// a null proxy, but only if nothing follows the
		// quotes.
		//
		else if(IceUtil.StringUtil.findFirstNotOf(s, delim, end) != -1)
		{
		    Ice.ProxyParseException e = new Ice.ProxyParseException();
		    e.str = s;
		    throw e;
		}
		else
		{
		    return null;
		}
	    }

	    string facet = "";
	    Reference.Mode mode = Reference.Mode.ModeTwoway;
	    bool secure = false;
	    string adapter = "";

	    while(true)
	    {
		beg = IceUtil.StringUtil.findFirstNotOf(s, delim, end);
		if(beg == -1)
		{
		    break;
		}

		if(s[beg] == ':' || s[beg] == '@')
		{
		    break;
		}

		end = IceUtil.StringUtil.findFirstOf(s, delim + ":@", beg);
		if(end == -1)
		{
		    end = s.Length;
		}

		if(beg == end)
		{
		    break;
		}

		string option = s.Substring(beg, end - beg);
		if(option.Length != 2 || option[0] != '-')
		{
		    Ice.ProxyParseException e = new Ice.ProxyParseException();
		    e.str = s;
		    throw e;
		}

		//
		// Check for the presence of an option argument. The
		// argument may be enclosed in single or double
		// quotation marks.
		//
		string argument = null;
		int argumentBeg = IceUtil.StringUtil.findFirstNotOf(s, delim, end);
		if(argumentBeg != -1)
		{
		    char ch = s[argumentBeg];
		    if(ch != '@' && ch != ':' && ch != '-')
		    {
			beg = argumentBeg;
			end = IceUtil.StringUtil.checkQuote(s, beg);
			if(end == -1)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			else if(end == 0)
			{
			    end = IceUtil.StringUtil.findFirstOf(s, delim + ":@", beg);
			    if(end == -1)
			    {
				end = s.Length;
			    }
			    argument = s.Substring(beg, end - beg);
			}
			else
			{
			    beg++; // Skip leading quote
			    argument = s.Substring(beg, end - beg);
			    end++; // Skip trailing quote
			}
		    }
		}

		//
		// If any new options are added here,
		// IceInternal::Reference::toString() and its derived classes must be updated as well.
		//
		switch(option[1])
		{
		    case 'f':
		    {
			if(argument == null)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}

			string token;
			if(!IceUtil.StringUtil.unescapeString(argument, 0, argument.Length, out token))
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
                        facet = token;
			break;
		    }

		    case 't':
		    {
			if(argument != null)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			mode = Reference.Mode.ModeTwoway;
			break;
		    }

		    case 'o':
		    {
			if(argument != null)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			mode = Reference.Mode.ModeOneway;
			break;
		    }

		    case 'O':
		    {
			if(argument != null)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			mode = Reference.Mode.ModeBatchOneway;
			break;
		    }

		    case 'd':
		    {
			if(argument != null)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			mode = Reference.Mode.ModeDatagram;
			break;
		    }

		    case 'D':
		    {
			if(argument != null)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			mode = Reference.Mode.ModeBatchDatagram;
			break;
		    }

		    case 's':
		    {
			if(argument != null)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			secure = true;
			break;
		    }

		    default:
		    {
			Ice.ProxyParseException e = new Ice.ProxyParseException();
			e.str = s;
			throw e;
		    }
		}
	    }

	    RouterInfo routerInfo = instance_.routerManager().get(getDefaultRouter());
	    LocatorInfo locatorInfo = instance_.locatorManager().get(getDefaultLocator());

	    if(beg == -1)
	    {
		return create(ident, instance_.initializationData().defaultContext, facet, mode, secure, "",
			      routerInfo, locatorInfo, instance_.defaultsAndOverrides().defaultCollocationOptimization,
			      instance_.defaultsAndOverrides().defaultLocatorCacheTimeout);
	    }

	    ArrayList endpoints = new ArrayList();

	    if(s[beg] == ':')
	    {
	        ArrayList unknownEndpoints = new ArrayList();
		end = beg;

		while(end < s.Length && s[end] == ':')
		{
		    beg = end + 1;
		    
		    end = s.IndexOf(':', beg);
		    if(end == -1)
		    {
			end = s.Length;
		    }
		    
		    string es = s.Substring(beg, end - beg);
		    EndpointI endp = instance_.endpointFactoryManager().create(es);
		    if(endp != null)
		    {
		        ArrayList endps = endp.expand(false);
			endpoints.AddRange(endps);
		    }
		    else
		    {
		        unknownEndpoints.Add(es);
		    }
		}
		if(endpoints.Count == 0)
		{
		    Ice.EndpointParseException e2 = new Ice.EndpointParseException();
		    e2.str = s;
		    throw e2;
		}
		else if(unknownEndpoints.Count != 0 &&
		        instance_.initializationData().properties.getPropertyAsIntWithDefault(
										"Ice.Warn.Endpoints", 1) > 0)
		{
		    string msg = "Proxy contains unknown endpoints:";
		    int sz = unknownEndpoints.Count;
		    for(int idx = 0; idx < sz; ++idx)
		    {
		        msg += " `" + (string)unknownEndpoints[idx] + "'";
		    }
		    instance_.initializationData().logger.warning(msg);
		}

		EndpointI[] ep = (EndpointI[])endpoints.ToArray(typeof(EndpointI));
		return create(ident, instance_.initializationData().defaultContext, facet, mode, secure, ep, routerInfo,
			      instance_.defaultsAndOverrides().defaultCollocationOptimization);
	    }
	    else if(s[beg] == '@')
	    {
		beg = IceUtil.StringUtil.findFirstNotOf(s, delim, beg + 1);
		if(beg == -1)
		{
		    Ice.ProxyParseException e = new Ice.ProxyParseException();
		    e.str = s;
		    throw e;
		}

		end = IceUtil.StringUtil.checkQuote(s, beg);
		if(end == -1)
		{
		    Ice.ProxyParseException e = new Ice.ProxyParseException();
		    e.str = s;
		    throw e;
		}
		else if(end == 0)
		{
		    end = IceUtil.StringUtil.findFirstOf(s, delim, beg);
		    if(end == -1)
		    {
			end = s.Length;
		    }
		}
		else
		{
		    beg++; // Skip leading quote
		}

		if(!IceUtil.StringUtil.unescapeString(s, beg, end, out adapter) || adapter.Length == 0)
		{
		    Ice.ProxyParseException e = new Ice.ProxyParseException();
		    e.str = s;
		    throw e;
		}
		return create(ident, instance_.initializationData().defaultContext, facet, mode, secure, adapter,
			      routerInfo, locatorInfo, instance_.defaultsAndOverrides().defaultCollocationOptimization,
			      instance_.defaultsAndOverrides().defaultLocatorCacheTimeout);
	    }

	    Ice.ProxyParseException ex = new Ice.ProxyParseException();
	    ex.str = s;
	    throw ex;
	}

	public Reference create(Ice.Identity ident, BasicStream s)
	{
	    //
	    // Don't read the identity here. Operations calling this
	    // constructor read the identity, and pass it as a parameter.
	    //

	    if(ident.name.Length == 0 && ident.category.Length == 0)
	    {
		return null;
	    }

	    //
	    // For compatibility with the old FacetPath.
	    //
	    string[] facetPath = s.readStringSeq();
	    string facet;
	    if(facetPath.Length > 0)
	    {
		if(facetPath.Length > 1)
		{
		    throw new Ice.ProxyUnmarshalException();
		}
		facet = facetPath[0];
	    }
	    else
	    {
		facet = "";
	    }

	    int mode = (int)s.readByte();
	    if(mode < 0 || mode > (int)Reference.Mode.ModeLast)
	    {
		throw new Ice.ProxyUnmarshalException();
	    }

	    bool secure = s.readBool();

	    EndpointI[] endpoints;
	    string adapterId = "";

	    RouterInfo routerInfo = instance_.routerManager().get(getDefaultRouter());
	    LocatorInfo locatorInfo = instance_.locatorManager().get(getDefaultLocator());

	    int sz = s.readSize();
	    if(sz > 0)
	    {
		endpoints = new EndpointI[sz];
		for(int i = 0; i < sz; i++)
		{
		    endpoints[i] = instance_.endpointFactoryManager().read(s);
		}
		return create(ident, instance_.initializationData().defaultContext, facet, (Reference.Mode)mode,
			      secure, endpoints, routerInfo,
			      instance_.defaultsAndOverrides().defaultCollocationOptimization);
	    }
	    else
	    {
		endpoints = new EndpointI[0];
		adapterId = s.readString();
		return create(ident, instance_.initializationData().defaultContext, facet, (Reference.Mode)mode,
			      secure, adapterId, routerInfo, locatorInfo,
			      instance_.defaultsAndOverrides().defaultCollocationOptimization,
			      instance_.defaultsAndOverrides().defaultLocatorCacheTimeout);
	    }
	}

	public void setDefaultRouter(Ice.RouterPrx defaultRouter)
	{
	    lock(this)
	    {
		_defaultRouter = defaultRouter;
	    }
	}

	public Ice.RouterPrx getDefaultRouter()
	{
	    lock(this)
	    {
		return _defaultRouter;
	    }
	}

	public void setDefaultLocator(Ice.LocatorPrx defaultLocator)
	{
	    lock(this)
	    {
		_defaultLocator = defaultLocator;
	    }
	}

	public Ice.LocatorPrx getDefaultLocator()
	{
	    lock(this)
	    {
		return _defaultLocator;
	    }
	}

	//
	// Only for use by Instance
	//
	internal ReferenceFactory(Instance instance, Ice.Communicator communicator)
	{
	    instance_ = instance;
	    _communicator = communicator;
	}

	internal void destroy()
	{
	    lock(this)
	    {
		if(instance_ == null)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}

		instance_ = null;
		_defaultRouter = null;
		_defaultLocator = null;
		_references.Clear();
	    }
	}

	private Reference updateCache(Reference @ref)
	{
	    //
	    // If we already have an equivalent reference, use such equivalent
	    // reference. Otherwise add the new reference to the reference
	    // set.
	    //
	    WeakReference w = new WeakReference(@ref);
	    WeakReference val = (WeakReference)_references[w];
	    if(val != null)
	    {
		Reference r = (Reference)val.Target;
		if(r != null && r.Equals(@ref))
		{
		    return r;
		}
	    }
	    _references[w] = w;

	    return @ref;
	}

	private Instance instance_;
	private Ice.Communicator _communicator;
	private Ice.RouterPrx _defaultRouter;
	private Ice.LocatorPrx _defaultLocator;
	private Hashtable _references = new Hashtable();
    }

}
