// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
    create(String str)
    {
        String s = str.trim();
        if(s.length() == 0)
        {
            throw new Ice.ProxyParseException();
        }

        int colon = s.indexOf(':');
	if(colon == -1)
	    colon = s.indexOf('@');

        String init;
        if(colon == -1)
        {
            init = s;
        }
        else
        {
            init = s.substring(0, colon);
        }

        String[] arr = init.split("[ \t\n\r]+");
        Ice.Identity ident = Ice.Util.stringToIdentity(arr[0]);
        String[] facet = new String[0];
        int mode = Reference.ModeTwoway;
        boolean secure = false;
        boolean compress = false;
	String adapter = "";

        int i = 1;
        while(i < arr.length)
        {
            String option = arr[i++];
            if(option.length() != 2 || option.charAt(0) != '-')
            {
                throw new Ice.ProxyParseException();
            }

            String argument = null;
            if(i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
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
                        throw new Ice.EndpointParseException();
                    }

		    // TODO: For Mark.
                    //facet = argument;
                    break;
                }

                case 't':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = Reference.ModeTwoway;
                    break;
                }

                case 'o':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = Reference.ModeOneway;
                    break;
                }

                case 'O':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = Reference.ModeBatchOneway;
                    break;
                }

                case 'd':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = Reference.ModeDatagram;
                    break;
                }

                case 'D':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = Reference.ModeBatchDatagram;
                    break;
                }

                case 's':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    secure = true;
                    break;
                }

                case 'c':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    compress = true;
                    break;
                }

                default:
                {
                    throw new Ice.ProxyParseException();
                }
            }
        }

        java.util.ArrayList endpoints = new java.util.ArrayList();

	if(colon != -1)
	{
	    if(s.charAt(colon) == ':')
	    {
		final int len = s.length();
		int end = colon;
		while(end < len && s.charAt(end) == ':')
		{
		    int beg = end + 1;
		    
		    end = s.indexOf(':', beg);
		    if(end == -1)
		    {
			end = len;
		    }
		    
		    String es = s.substring(beg, end);
		    Endpoint endp = _instance.endpointFactoryManager().create(es);
		    endpoints.add(endp);
		}
	    }
	    else if(s.charAt(colon) == '@')
	    {
		init = s.substring(colon + 1, s.length()).trim();
		arr = init.split("[ \t\n\r]+");
		adapter = arr[0];
	    }
	}

        Endpoint[] endp = new Endpoint[endpoints.size()];
        endpoints.toArray(endp);

        RouterInfo routerInfo = _instance.routerManager().get(getDefaultRouter());
        LocatorInfo locatorInfo = _instance.locatorManager().get(getDefaultLocator());
        return create(ident, facet, mode, secure, compress, adapter, endp, routerInfo, locatorInfo, null);
    }

    public Reference
    create(Ice.Identity ident, BasicStream s)
    {
        //
        // Don't read the identity here. Operations calling this
        // constructor read the identity, and pass it as a parameter.
        //

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
