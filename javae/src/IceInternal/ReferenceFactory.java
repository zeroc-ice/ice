// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ReferenceFactory
{
    public synchronized Reference
    create(Ice.Identity ident,
           java.util.Hashtable context,
           String facet,
           int mode,
	   boolean secure,
           Endpoint[] endpoints,
           RouterInfo routerInfo)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        if(ident.name.length() == 0 && ident.category.length() == 0)
        {
            return null;
        }

        //
        // Create new reference
        //
        return new DirectReference(_instance, _communicator, ident, context, facet, mode, secure, endpoints, 
                                   routerInfo);
    }

    public synchronized Reference
    create(Ice.Identity ident,
           java.util.Hashtable context,
           String facet,
           int mode,
	   boolean secure,
           String adapterId,
           RouterInfo routerInfo,
	   LocatorInfo locatorInfo)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        if(ident.name.length() == 0 && ident.category.length() == 0)
        {
            return null;
        }

        //
        // Create new reference
        //
        return new IndirectReference(_instance, _communicator, ident, context, facet, mode, secure, adapterId,
				     routerInfo, locatorInfo);
    }

    public synchronized Reference
    create(Ice.Identity ident,
           java.util.Hashtable context,
           String facet,
           int mode,
	   Ice.Connection[] fixedConnections)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        if(ident.name.length() == 0 && ident.category.length() == 0)
        {
            return null;
        }

        //
        // Create new reference
        //
        return new FixedReference(_instance, _communicator, ident, context, facet, mode, fixedConnections);
    }

    public synchronized Reference
    copy(Reference r)
    {
        if(_instance == null)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

	Ice.Identity ident = r.getIdentity();
	if(ident.name.length() == 0 && ident.category.length() == 0)
	{
	    return null;
	}
	return (Reference)r.ice_clone();
    }

    public Reference
    create(String s)
    {
        if(s == null || s.length() == 0)
        {
            return null;
        }

        final String delim = " \t\n\r";

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
        String idstr = null;
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
                end = s.length();
            }
            idstr = s.substring(beg, end);
        }
        else
        {
            beg++; // Skip leading quote
            idstr = s.substring(beg, end);
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
        Ice.Identity ident = _instance.stringToIdentity(idstr);

	IceUtil.Debug.Assert(ident.name != null);
        if(ident.name.length() == 0)
        {
            //
            // An identity with an empty name and a non-empty
            // category is illegal.
            //
            if(ident.category.length() > 0)
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

        String facet = "";
        int mode = Reference.ModeTwoway;
	boolean secure = false;
	String adapter = "";

        while(true)
        {
            beg = IceUtil.StringUtil.findFirstNotOf(s, delim, end);
            if(beg == -1)
            {
                break;
            }

            if(s.charAt(beg) == ':' || s.charAt(beg) == '@')
            {
                break;
            }

            end = IceUtil.StringUtil.findFirstOf(s, delim + ":@", beg);
            if(end == -1)
            {
                end = s.length();
            }

            if(beg == end)
            {
                break;
            }

            String option = s.substring(beg, end);
            if(option.length() != 2 || option.charAt(0) != '-')
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
            String argument = null;
            int argumentBeg = IceUtil.StringUtil.findFirstNotOf(s, delim, end);
            if(argumentBeg != -1)
            {
                final char ch = s.charAt(argumentBeg);
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
                            end = s.length();
                        }
                        argument = s.substring(beg, end);
                    }
                    else
                    {
                        beg++; // Skip leading quote
                        argument = s.substring(beg, end);
                        end++; // Skip trailing quote
                    }
                }
            }

            //
            // If any new options are added here,
            // IceInternal::Reference::toString() and its derived classes must be updated as well.
            //
            switch(option.charAt(1))
            {
                case 'f':
                {
                    if(argument == null)
                    {
                        Ice.ProxyParseException e = new Ice.ProxyParseException();
			e.str = s;
			throw e;
                    }

                    Ice.StringHolder facetH = new Ice.StringHolder();
                    if(!IceUtil.StringUtil.unescapeString(argument, 0, argument.length(), facetH))
                    {
                        Ice.ProxyParseException e = new Ice.ProxyParseException();
			e.str = s;
			throw e;
                    }

                    facet = facetH.value;
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

	RouterInfo routerInfo = _instance.routerManager().get(getDefaultRouter());
	LocatorInfo locatorInfo = _instance.locatorManager().get(getDefaultLocator());

	if(beg == -1)
	{
	    return create(ident, null, facet, mode, secure, "", routerInfo, locatorInfo);
	}

        java.util.Vector endpoints = new java.util.Vector();

	if(s.charAt(beg) == ':')
	{
	    java.util.Vector unknownEndpoints = new java.util.Vector();
	    end = beg;

	    while(end < s.length() && s.charAt(end) == ':')
	    {
		beg = end + 1;
		
		end = s.indexOf(':', beg);
		if(end == -1)
		{
		    end = s.length();
		}
		
		String es = s.substring(beg, end);
		Endpoint endp = _instance.endpointFactory().create(es);
		if(endp != null)
		{
		    endpoints.addElement(endp);
		}
		else
		{
		    unknownEndpoints.addElement(es);
		}
	    }
	    if(endpoints.size() == 0)
	    {
	        Ice.EndpointParseException e = new Ice.EndpointParseException();
		e.str = (String)unknownEndpoints.elementAt(0);
		throw e;
	    }
	    else if(unknownEndpoints.size() != 0 &&
	    	    _instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Endpoints", 1) > 0)
	    {
	        String msg = "Proxy contains unknown endpoints:";
		java.util.Enumeration e = unknownEndpoints.elements();
		while(e.hasMoreElements())
		{
		    msg += " `" + (String)e.nextElement() + "'";
		}
		_instance.initializationData().logger.warning(msg);
	    }

	    Endpoint[] endp = new Endpoint[endpoints.size()];
	    endpoints.copyInto(endp);
	    return create(ident, null, facet, mode, secure, endp, routerInfo);
	}
	else if(s.charAt(beg) == '@')
	{
	    beg = IceUtil.StringUtil.findFirstNotOf(s, delim, beg + 1);
	    if(beg == -1)
	    {
		Ice.ProxyParseException e = new Ice.ProxyParseException();
		e.str = s;
		throw e;
	    }

            String adapterstr = null;
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
		    end = s.length();
		}
                adapterstr = s.substring(beg, end);
	    }
	    else
	    {
		beg++; // Skip leading quote
                adapterstr = s.substring(beg, end);
                end++; // Skip trailing quote
	    }

            if(end != s.length() && IceUtil.StringUtil.findFirstNotOf(s, delim, end) != -1)
            {
                Ice.ProxyParseException e = new Ice.ProxyParseException();
                e.str = s;
                throw e;
            }

	    Ice.StringHolder token = new Ice.StringHolder();
            if(!IceUtil.StringUtil.unescapeString(adapterstr, 0, adapterstr.length(), token) ||
               token.value.length() == 0)
	    {
		Ice.ProxyParseException e = new Ice.ProxyParseException();
		e.str = s;
		throw e;
	    }
	    adapter = token.value;
	    return create(ident, null, facet, mode, secure, adapter, routerInfo, locatorInfo);
	}

	Ice.ProxyParseException ex = new Ice.ProxyParseException();
	ex.str = s;
	throw ex;
    }

    public Reference
    createFromProperties(String propertyPrefix)
    {
        Ice.Properties properties = _instance.initializationData().properties;

        Reference ref = create(properties.getProperty(propertyPrefix));
        if(ref == null)
        {
            return null;
        }

        String property = propertyPrefix + ".Locator";
        if(properties.getProperty(property).length() != 0)
        {
            ref = ref.changeLocator(Ice.LocatorPrxHelper.uncheckedCast(_communicator.propertyToProxy(property)));
            if(ref instanceof DirectReference)
            {
                String s = "`" + property + "=" + properties.getProperty(property) +
                           "': cannot set a locator on a direct reference; setting ignored";
                _instance.initializationData().logger.warning(s);
            }
        }

        property = propertyPrefix + ".Router";
        if(properties.getProperty(property).length() != 0)
        {
            if(propertyPrefix.endsWith(".Router"))
            {
                String s = "`" + property + "=" + properties.getProperty(property) +
                           "': cannot set a router on a router; setting ignored";
                _instance.initializationData().logger.warning(s);
            }
            else
            {
                ref = ref.changeRouter(Ice.RouterPrxHelper.uncheckedCast(_communicator.propertyToProxy(property)));
            }
        }

        return ref;
    }

    public Reference
    create(Ice.Identity ident, BasicStream s)
    {
        //
        // Don't read the identity here. Operations calling this
        // constructor read the identity, and pass it as a parameter.
        //

        if(ident.name.length() == 0 && ident.category.length() == 0)
        {
            return null;
        }

        //
        // For compatibility with the old FacetPath.
        //
        String[] facetPath = s.readStringSeq();
        String facet;
        if(facetPath.length > 0)
        {
	    if(facetPath.length > 1)
	    {
	        Ice.Util.throwProxyUnmarshalException();
	    }
            facet = facetPath[0];
        }
	else
        {
	    facet = "";
	}

        int mode = (int)s.readByte();
        if(mode < 0 || mode > Reference.ModeLast)
        {
            Ice.Util.throwProxyUnmarshalException();
        }

	boolean secure = s.readBool();
       
        Endpoint[] endpoints;
	String adapterId = "";

        RouterInfo routerInfo = _instance.routerManager().get(getDefaultRouter());
        LocatorInfo locatorInfo = _instance.locatorManager().get(getDefaultLocator());

        int sz = s.readSize();
	if(sz > 0)
	{
	    endpoints = new Endpoint[sz];
	    for(int i = 0; i < sz; i++)
	    {
		endpoints[i] = _instance.endpointFactory().read(s);
	    }
	    return create(ident, null, facet, mode, secure, endpoints, routerInfo);
	}
	else
	{
	    endpoints = new Endpoint[0];
	    adapterId = s.readString();
	    return create(ident, null, facet, mode, secure, adapterId, routerInfo, locatorInfo);
	}
    }

    public synchronized void
    setDefaultRouter(Ice.RouterPrx defaultRouter)
    {
        _defaultRouter = defaultRouter;
    }

    public synchronized Ice.RouterPrx
    getDefaultRouter()
    {
        return _defaultRouter;
    }

    public synchronized void
    setDefaultLocator(Ice.LocatorPrx defaultLocator)
    {
        _defaultLocator = defaultLocator;
    }

    public synchronized Ice.LocatorPrx
    getDefaultLocator()
    {
        return _defaultLocator;
    }

    //
    // Only for use by Instance
    //
    ReferenceFactory(Instance instance, Ice.Communicator communicator)
    {
        _instance = instance;
	_communicator = communicator;
    }

    synchronized void
    destroy()
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        _instance = null;
	_communicator = null;
        _defaultRouter = null;
        _defaultLocator = null;
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
    private Ice.RouterPrx _defaultRouter;
    private Ice.LocatorPrx _defaultLocator;
    private int _hashUpdateCounter = 0;
}
