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

    public sealed class ReferenceFactory
    {
	public Reference
	create(Ice.Identity ident,
	       Ice.Context context,
	       Ice.FacetPath facet,
	       int mode,
	       bool secure,
	       string adapterId,
	       Endpoint[] endpoints,
	       RouterInfo routerInfo,
	       LocatorInfo locatorInfo,
	       Ice.ObjectAdapter reverseAdapter,
	       bool collocationOptimization)
	{
	    lock(this)
	    {
		if(_instance == null)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		if(ident.name.Length == 0 && ident.category.Length == 0)
		{
		    return null;
		}
		
		//
		// Create a new reference
		//
		return new Reference(_instance, ident, context, facet, mode, secure, adapterId,
					     endpoints, routerInfo, locatorInfo,
					     reverseAdapter, collocationOptimization);
	    }
	}
	
	public Reference create(string s)
	{
	    if(s.Length == 0)
	    {
		return null;
	    }
	    
	    string delim = " \t\n\r";
	    
	    int beg;
	    int end = 0;
	    
	    beg = StringUtil.findFirstNotOf(s, delim, end);
	    if(beg == - 1)
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
	    end = StringUtil.checkQuote(s, beg);
	    if(end == - 1)
	    {
		Ice.ProxyParseException e = new Ice.ProxyParseException();
		e.str = s;
		throw e;
	    }
	    else if(end == 0)
	    {
		end = StringUtil.findFirstOf(s, delim + ":@", beg);
		if(end == - 1)
		{
		    end = s.Length;
		}
		idstr = s.Substring(beg, (end) - (beg));
	    }
	    else
	    {
		beg++; // Skip leading quote
		idstr = s.Substring(beg, (end) - (beg));
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
	    Ice.Identity ident = Ice.Util.stringToIdentity(idstr);
	    
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
		else if(StringUtil.findFirstNotOf(s, delim, end) != - 1)
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
	    
	    Ice.FacetPath facet = new Ice.FacetPath();
	    int mode = Reference.ModeTwoway;
	    bool secure = false;
	    string adapter = "";
	    
	    while(true)
	    {
		beg = StringUtil.findFirstNotOf(s, delim, end);
		if(beg == - 1)
		{
		    break;
		}
		
		if(s[beg] == ':' || s[beg] == '@')
		{
		    break;
		}
		
		end = StringUtil.findFirstOf(s, delim + ":@", beg);
		if(end == - 1)
		{
		    end = s.Length;
		}
		
		if(beg == end)
		{
		    break;
		}
		
		string option = s.Substring(beg, (end) - (beg));
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
		int argumentBeg = StringUtil.findFirstNotOf(s, delim, end);
		if(argumentBeg != - 1)
		{
		    char ch = s[argumentBeg];
		    if(ch != '@' && ch != ':' && ch != '-')
		    {
			beg = argumentBeg;
			end = StringUtil.checkQuote(s, beg);
			if(end == - 1)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			else if(end == 0)
			{
			    end = StringUtil.findFirstOf(s, delim + ":@", beg);
			    if(end == - 1)
			    {
					end = s.Length;
			    }
			    argument = s.Substring(beg, (end) - (beg));
			}
			else
			{
			    beg++; // Skip leading quote
			    argument = s.Substring(beg, (end) - (beg));
			    end++; // Skip trailing quote
			}
		    }
		}
		
		//
		// If any new options are added here,
		// IceInternal::Reference::toString() must be updated as well.
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
			
			int argLen = argument.Length;
			string token;
			
			int argBeg = 0;
			while(argBeg < argLen)
			{
			    //
			    // Skip slashes
			    //
			    argBeg = StringUtil.findFirstNotOf(argument, "/", argBeg);
			    if(argBeg == - 1)
			    {
				break;
			    }
			    
			    //
			    // Find unescaped slash
			    //
			    int argEnd = argBeg;
			    //UPGRADE_WARNING: Method 'java.lang.String.indexOf' was converted to 'string.IndexOf' which may throw an exception. 'ms-help://MS.VSCC.2003/commoner/redir/redirect.htm?keyword="jlca1101"'
			    while((argEnd = argument.IndexOf((System.Char) '/', argEnd)) != - 1)
			    {
				if(argument[argEnd - 1] != '\\')
				{
				    break;
				}
				argEnd++;
			    }
			    
			    if(argEnd == - 1)
			    {
				argEnd = argLen;
			    }
			    
			    if(!IceInternal.StringUtil.decodeString(argument, argBeg, argEnd, out token))
			    {
				Ice.ProxyParseException e = new Ice.ProxyParseException();
				e.str = s;
				throw e;
			    }
			    facet.Add(token);
			    argBeg = argEnd + 1;
			}
			
			if(facet.Count == 0)
			{
			    Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
			}
			
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
			mode = Reference.ModeTwoway;
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
			mode = Reference.ModeOneway;
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
			mode = Reference.ModeBatchOneway;
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
			mode = Reference.ModeDatagram;
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
			mode = Reference.ModeBatchDatagram;
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
	    
	    ArrayList endpoints = new ArrayList();
	    if(beg != - 1)
	    {
		if(s[beg] == ':')
		{
		    end = beg;
		    
		    while(end < s.Length && s[end] == ':')
		    {
			beg = end + 1;
			end = s.IndexOf(':', beg);
			if(end == -1)
			{
			    end = s.Length;
			}
			
			string es = s.Substring(beg, (end) - (beg));
			Endpoint endp = _instance.endpointFactoryManager().create(es);
			endpoints.Add(endp);
		    }
		}
		else if(s[beg] == '@')
		{
		    beg = StringUtil.findFirstNotOf(s, delim, beg + 1);
		    if(beg == - 1)
		    {
			Ice.ProxyParseException e = new Ice.ProxyParseException();
			e.str = s;
			throw e;
		    }
		    
		    end = StringUtil.checkQuote(s, beg);
		    if(end == - 1)
		    {
			Ice.ProxyParseException e = new Ice.ProxyParseException();
			e.str = s;
			throw e;
		    }
		    else if(end == 0)
		    {
			end = StringUtil.findFirstOf(s, delim, beg);
			if(end == - 1)
			{
			    end = s.Length;
			}
		    }
		    else
		    {
			beg++; // Skip leading quote
		    }
		    
		    string token;
		    if(!IceInternal.StringUtil.decodeString(s, beg, end, out token) || token.Length == 0)
		    {
			Ice.ProxyParseException e = new Ice.ProxyParseException();
			e.str = s;
			throw e;
		    }
		    adapter = token;
		}
	    }
	    
	    Endpoint[] endp2 = new Endpoint[endpoints.Count];
	    endpoints.CopyTo(endp2, 0);
	    
	    RouterInfo routerInfo = _instance.routerManager().get(getDefaultRouter());
	    LocatorInfo locatorInfo = _instance.locatorManager().get(getDefaultLocator());
	    return create(ident, new Ice.Context(), facet, mode, secure, adapter, endp2, routerInfo, locatorInfo, null, true);
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
	    
	    Ice.FacetPath facet = s.readFacetPath();
	    
	    int mode = (int) s.readByte();
	    if(mode < 0 || mode > Reference.ModeLast)
	    {
		throw new Ice.ProxyUnmarshalException();
	    }
	    
	    bool secure = s.readBool();
	    
	    Endpoint[] endpoints;
	    string adapterId = "";
	    
	    int sz = s.readSize();
	    if(sz > 0)
	    {
		endpoints = new Endpoint[sz];
		for(int i = 0; i < sz; i++)
		{
		    endpoints[i] = _instance.endpointFactoryManager().read(s);
		}
	    }
	    else
	    {
		endpoints = new Endpoint[0];
		adapterId = s.readString();
	    }
	    
	    RouterInfo routerInfo = _instance.routerManager().get(getDefaultRouter());
	    LocatorInfo locatorInfo = _instance.locatorManager().get(getDefaultLocator());
	    return create(ident, new Ice.Context(), facet, mode, secure, adapterId, endpoints, routerInfo, locatorInfo, null, true);
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
	internal ReferenceFactory(Instance instance)
	{
	    _instance = instance;
	}
	
	internal void destroy()
	{
	    lock(this)
	    {
		if(_instance == null)
		{
		    throw new Ice.CommunicatorDestroyedException();
		}
		
		_instance = null;
		_defaultRouter = null;
		_defaultLocator = null;
	    }
	}
	
	private Instance _instance;
	private Ice.RouterPrx _defaultRouter;
	private Ice.LocatorPrx _defaultLocator;
    }

}
