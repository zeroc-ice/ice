// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class ReferenceFactory
{
    public synchronized Reference
    create(Ice.Identity ident,
           String[] facet,
           int mode,
           boolean secure,
	   boolean compress,
	   String adapterId,
           Endpoint[] endpoints,
           RouterInfo routerInfo,
           LocatorInfo locatorInfo,
           Ice.ObjectAdapter reverseAdapter)
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
        // Create a new reference
        //
        Reference ref = new Reference(_instance, ident, facet, mode, secure, compress, adapterId,
				      endpoints, routerInfo, locatorInfo, reverseAdapter);

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

        beg = StringUtil.findFirstNotOf(s, delim, end);
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
        end = StringUtil.checkQuote(s, beg);
        if(end == -1)
        {
            Ice.ProxyParseException e = new Ice.ProxyParseException();
	    e.str = s;
	    throw e;
        }
        else if(end == 0)
        {
            end = StringUtil.findFirstOf(s, delim + ":@", beg);
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
            else if(StringUtil.findFirstNotOf(s, delim, end) != -1)
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

        java.util.ArrayList facet = new java.util.ArrayList();
        int mode = Reference.ModeTwoway;
        boolean secure = false;
        boolean compress = false;
	String adapter = "";

        while(true)
        {
            beg = StringUtil.findFirstNotOf(s, delim, end);
            if(beg == -1)
            {
                break;
            }

            if(s.charAt(beg) == ':' || s.charAt(beg) == '@')
            {
                break;
            }

            end = StringUtil.findFirstOf(s, delim + ":@", beg);
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
            int argumentBeg = StringUtil.findFirstNotOf(s, delim, end);
            if(argumentBeg != -1)
            {
                final char ch = s.charAt(argumentBeg);
                if(ch != '@' && ch != ':' && ch != '-')
                {
                    beg = argumentBeg;
                    end = StringUtil.checkQuote(s, beg);
                    if(end == -1)
                    {
                        Ice.ProxyParseException e = new Ice.ProxyParseException();
			e.str = s;
			throw e;
                    }
                    else if(end == 0)
                    {
                        end = StringUtil.findFirstOf(s, delim + ":@", beg);
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
            // IceInternal::Reference::toString() must be updated as well.
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

                    final int argLen = argument.length();
                    Ice.StringHolder token = new Ice.StringHolder();

                    int argBeg = 0;
                    while(argBeg < argLen)
                    {
                        //
                        // Skip slashes
                        //
                        argBeg = StringUtil.findFirstNotOf(argument, "/", argBeg);
                        if(argBeg == -1)
                        {
                            break;
                        }

                        //
                        // Find unescaped slash
                        //
                        int argEnd = argBeg;
                        while((argEnd = argument.indexOf('/', argEnd)) != -1)
                        {
                            if(argument.charAt(argEnd - 1) != '\\')
                            {
                                break;
                            }
                            argEnd++;
                        }

                        if(argEnd == -1)
                        {
                            argEnd = argLen;
                        }

                        if(!IceInternal.StringUtil.decodeString(argument, argBeg, argEnd, token))
                        {
                            Ice.ProxyParseException e = new Ice.ProxyParseException();
			    e.str = s;
			    throw e;
                        }
                        facet.add(token.value);
                        argBeg = argEnd + 1;
                    }

                    if(facet.size() == 0)
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

                case 'c':
                {
                    if(argument != null)
                    {
                        Ice.ProxyParseException e = new Ice.ProxyParseException();
			e.str = s;
			throw e;
                    }
                    compress = true;
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

        java.util.ArrayList endpoints = new java.util.ArrayList();
        if(beg != -1)
        {
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
	    }
	    else if(s.charAt(beg) == '@')
	    {
                beg = StringUtil.findFirstNotOf(s, delim, beg + 1);
                if(beg == -1)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
		    e.str = s;
		    throw e;
                }

                end = StringUtil.checkQuote(s, beg);
                if(end == -1)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
		    e.str = s;
		    throw e;
                }
                else if(end == 0)
                {
                    end = StringUtil.findFirstOf(s, delim, beg);
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
                if(!IceInternal.StringUtil.decodeString(s, beg, end, token) || token.value.length() == 0)
                {
                    Ice.ProxyParseException e = new Ice.ProxyParseException();
		    e.str = s;
		    throw e;
                }
                adapter = token.value;
	    }
	}

        Endpoint[] endp = new Endpoint[endpoints.size()];
        endpoints.toArray(endp);

        String[] fac = new String[facet.size()];
        facet.toArray(fac);

        RouterInfo routerInfo = _instance.routerManager().get(getDefaultRouter());
        LocatorInfo locatorInfo = _instance.locatorManager().get(getDefaultLocator());
        return create(ident, fac, mode, secure, compress, adapter, endp, routerInfo, locatorInfo, null);
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

        String[] facet = s.readStringSeq();

        int mode = (int)s.readByte();
        if(mode < 0 || mode > Reference.ModeLast)
        {
            throw new Ice.ProxyUnmarshalException();
        }

        boolean secure = s.readBool();

        boolean compress = s.readBool();

        Endpoint[] endpoints;
	String adapterId = "";

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
        return create(ident, facet, mode, secure, compress, adapterId, endpoints, routerInfo, locatorInfo, null);
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

    private Instance _instance;
    private Ice.RouterPrx _defaultRouter;
    private Ice.LocatorPrx _defaultLocator;
    private java.util.WeakHashMap _references = new java.util.WeakHashMap();
}
