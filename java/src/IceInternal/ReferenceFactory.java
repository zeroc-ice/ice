// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ReferenceFactory
{
    public synchronized Reference
    create(Ice.Identity ident,
           java.util.Map context,
           String facet,
           int mode,
           boolean secure,
           Endpoint[] endpoints,
           RouterInfo routerInfo,
	   boolean collocationOptimization)
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
        DirectReference ref = new DirectReference(_instance, ident, context, facet, mode, secure,
						  endpoints, routerInfo, collocationOptimization);
	return updateCache(ref);
    }

    public synchronized Reference
    create(Ice.Identity ident,
           java.util.Map context,
           String facet,
           int mode,
           boolean secure,
           String adapterId,
           RouterInfo routerInfo,
	   LocatorInfo locatorInfo,
	   boolean collocationOptimization)
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
        IndirectReference ref = new IndirectReference(_instance, ident, context, facet, mode, secure,
						      adapterId, routerInfo, locatorInfo, collocationOptimization);
	return updateCache(ref);
    }

    public synchronized Reference
    create(Ice.Identity ident,
           java.util.Map context,
           String facet,
           int mode,
	   Ice.ConnectionI[] fixedConnections)
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
        FixedReference ref = new FixedReference(_instance, ident, context, facet, mode, fixedConnections);
	return updateCache(ref);
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
	return (Reference)r.clone();
    }

    public Reference
    create(String s)
    {
        if(s.length() == 0)
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
        Ice.Identity ident = Ice.Util.stringToIdentity(idstr);

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
	    return create(ident, new java.util.HashMap(), facet, mode, secure, "", routerInfo, locatorInfo, true);
	}

        java.util.ArrayList endpoints = new java.util.ArrayList();

	if(s.charAt(beg) == ':')
	{
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
		Endpoint endp = _instance.endpointFactoryManager().create(es);
		endpoints.add(endp);
	    }
	    Endpoint[] endp = new Endpoint[endpoints.size()];
	    endpoints.toArray(endp);
	    return create(ident, new java.util.HashMap(), facet, mode, secure, endp, routerInfo, true);
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
	    }
	    else
	    {
		beg++; // Skip leading quote
	    }

	    Ice.StringHolder token = new Ice.StringHolder();
	    if(!IceUtil.StringUtil.unescapeString(s, beg, end, token) || token.value.length() == 0)
	    {
		Ice.ProxyParseException e = new Ice.ProxyParseException();
		e.str = s;
		throw e;
	    }
	    adapter = token.value;
	    return create(ident, new java.util.HashMap(), facet, mode, secure, adapter, routerInfo, locatorInfo, true);
	}

	Ice.ProxyParseException ex = new Ice.ProxyParseException();
	ex.str = s;
	throw ex;
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
	        throw new Ice.ProxyUnmarshalException();
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
            throw new Ice.ProxyUnmarshalException();
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
		endpoints[i] = _instance.endpointFactoryManager().read(s);
	    }
	    return create(ident, new java.util.HashMap(), facet, mode, secure, endpoints, routerInfo, true);
	}
	else
	{
	    endpoints = new Endpoint[0];
	    adapterId = s.readString();
	    return create(ident, new java.util.HashMap(), facet, mode, secure,
	                  adapterId, routerInfo, locatorInfo, true);
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
    ReferenceFactory(Instance instance)
    {
        _instance = instance;
    }

    synchronized void
    destroy()
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        _instance = null;
        _defaultRouter = null;
        _defaultLocator = null;
        _references.clear();
    }

    private Reference
    updateCache(Reference ref)
    {
        //
        // If we already have an equivalent reference, use such equivalent
        // reference. Otherwise add the new reference to the reference
        // set.
        //
        // Java implementation note: A WeakHashMap is used to hold References,
        // allowing References to be garbage collected automatically. A
        // Reference serves as both key and value in the map. The
        // WeakHashMap class internally creates a weak reference for the
        // key, and we use a weak reference for the value as well.
        //
        java.lang.ref.WeakReference w = (java.lang.ref.WeakReference)_references.get(ref);
        if(w != null)
        {
            Reference r = (Reference)w.get();
            if(r != null)
            {
                ref = r;
            }
            else
            {
                _references.put(ref, new java.lang.ref.WeakReference(ref));
            }
        }
        else
        {
            _references.put(ref, new java.lang.ref.WeakReference(ref));
        }

        return ref;
    }

    private Instance _instance;
    private Ice.RouterPrx _defaultRouter;
    private Ice.LocatorPrx _defaultLocator;
    private java.util.WeakHashMap _references = new java.util.WeakHashMap();
}
