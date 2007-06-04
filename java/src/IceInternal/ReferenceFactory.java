// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
           boolean preferSecure,
           EndpointI[] endpoints,
           RouterInfo routerInfo,
           boolean collocationOptimization,
           boolean cacheConnection,
           Ice.EndpointSelectionType endpointSelection,
           boolean threadPerConnection)
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
        DirectReference ref = new DirectReference(_instance, _communicator, ident, context, facet, mode, secure,
                                                  preferSecure, endpoints, routerInfo, collocationOptimization,
                                                  cacheConnection, endpointSelection, threadPerConnection);
        return updateCache(ref);
    }

    public synchronized Reference
    create(Ice.Identity ident,
           java.util.Map context,
           String facet,
           int mode,
           boolean secure,
           boolean preferSecure,
           String adapterId,
           RouterInfo routerInfo,
           LocatorInfo locatorInfo,
           boolean collocationOptimization,
           boolean cacheConnection,
           Ice.EndpointSelectionType endpointSelection,
           boolean threadPerConnection,
           int locatorCacheTimeout)
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
        IndirectReference ref = new IndirectReference(_instance, _communicator, ident, context, facet, mode, secure,
                                                      preferSecure, adapterId, routerInfo, locatorInfo,
                                                      collocationOptimization, cacheConnection, endpointSelection,
                                                      threadPerConnection, locatorCacheTimeout);
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
        FixedReference ref = new FixedReference(_instance, _communicator, ident, context, facet, mode,
                                                fixedConnections);
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
            return create(ident, _instance.getDefaultContext(), facet, mode, secure,
                          _instance.defaultsAndOverrides().defaultPreferSecure, "", routerInfo,
                          locatorInfo, _instance.defaultsAndOverrides().defaultCollocationOptimization, true,
                          _instance.defaultsAndOverrides().defaultEndpointSelection, _instance.threadPerConnection(),
                          _instance.defaultsAndOverrides().defaultLocatorCacheTimeout);
        }

        java.util.ArrayList endpoints = new java.util.ArrayList();

        if(s.charAt(beg) == ':')
        {
            java.util.ArrayList unknownEndpoints = new java.util.ArrayList();
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
                EndpointI endp = _instance.endpointFactoryManager().create(es, false);
                if(endp != null)
                {
                    java.util.ArrayList endps = endp.expand();
                    endpoints.addAll(endps);
                }
                else
                {
                    unknownEndpoints.add(es);
                }
            }
            if(endpoints.size() == 0)
            {
                Ice.EndpointParseException e = new Ice.EndpointParseException();
                e.str = (String)unknownEndpoints.get(0);
                throw e;
            }
            else if(unknownEndpoints.size() != 0 &&
                   _instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Endpoints", 1) > 0)
            {
                String msg = "Proxy contains unknown endpoints:";
                java.util.Iterator iter = unknownEndpoints.iterator();
                while(iter.hasNext())
                {
                    msg += " `" + (String)iter.next() + "'";
                }
                _instance.initializationData().logger.warning(msg);
            }

            EndpointI[] endp = new EndpointI[endpoints.size()];
            endpoints.toArray(endp);
            return create(ident, _instance.getDefaultContext(), facet, mode, secure, 
                          _instance.defaultsAndOverrides().defaultPreferSecure, endp, routerInfo,
                          _instance.defaultsAndOverrides().defaultCollocationOptimization, true,
                          _instance.defaultsAndOverrides().defaultEndpointSelection, _instance.threadPerConnection());
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
            return create(ident, _instance.getDefaultContext(), facet, mode, secure,
                          _instance.defaultsAndOverrides().defaultPreferSecure, adapter,
                          routerInfo, locatorInfo, _instance.defaultsAndOverrides().defaultCollocationOptimization,
                          true, _instance.defaultsAndOverrides().defaultEndpointSelection,
                          _instance.threadPerConnection(), _instance.defaultsAndOverrides().defaultLocatorCacheTimeout);
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

        //
        // Warn about unknown properties.
        //
        if(properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
        {
            checkForUnknownProperties(propertyPrefix);
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

        property = propertyPrefix + ".LocatorCacheTimeout";
        if(properties.getProperty(property).length() != 0)
        {
            ref = ref.changeLocatorCacheTimeout(properties.getPropertyAsInt(property));
            if(ref instanceof DirectReference)
            {
                String s = "`" + property + "=" + properties.getProperty(property) +
                           "': cannot set a locator cache timeout on a direct reference; setting ignored";
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

        property = propertyPrefix + ".PreferSecure";
        if(properties.getProperty(property).length() != 0)
        {
            ref = ref.changePreferSecure(properties.getPropertyAsInt(property) > 0);
        }

        property = propertyPrefix + ".ConnectionCached";
        if(properties.getProperty(property).length() != 0)
        {
            ref = ref.changeCacheConnection(properties.getPropertyAsInt(property) > 0);
        }

        property = propertyPrefix + ".EndpointSelection";
        if(properties.getProperty(property).length() != 0)
        {
            String type = properties.getProperty(property);
            if(type.equals("Random"))
            {
                ref = ref.changeEndpointSelection(Ice.EndpointSelectionType.Random);
            }
            else if(type.equals("Ordered"))
            {
                ref = ref.changeEndpointSelection(Ice.EndpointSelectionType.Ordered);
            }
            else
            {
                Ice.EndpointSelectionTypeParseException ex = new Ice.EndpointSelectionTypeParseException();
                ex.str = type;
                throw ex;
            }
        }

        property = propertyPrefix + ".CollocationOptimization";
        if(properties.getProperty(property).length() != 0)
        {
            ref = ref.changeCollocationOptimization(properties.getPropertyAsInt(property) > 0);
        }

        property = propertyPrefix + ".ThreadPerConnection";
        if(properties.getProperty(property).length() != 0)
        {
            ref = ref.changeThreadPerConnection(properties.getPropertyAsInt(property) > 0);
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

        EndpointI[] endpoints;
        String adapterId = "";

        RouterInfo routerInfo = _instance.routerManager().get(getDefaultRouter());
        LocatorInfo locatorInfo = _instance.locatorManager().get(getDefaultLocator());

        int sz = s.readSize();
        if(sz > 0)
        {
            endpoints = new EndpointI[sz];
            for(int i = 0; i < sz; i++)
            {
                endpoints[i] = _instance.endpointFactoryManager().read(s);
            }
            return create(ident, _instance.getDefaultContext(), facet, mode, secure, 
                          _instance.defaultsAndOverrides().defaultPreferSecure, endpoints,
                          routerInfo, _instance.defaultsAndOverrides().defaultCollocationOptimization, true,
                          _instance.defaultsAndOverrides().defaultEndpointSelection, _instance.threadPerConnection());
        }
        else
        {
            endpoints = new EndpointI[0];
            adapterId = s.readString();
            return create(ident, _instance.getDefaultContext(), facet, mode, secure,
                          _instance.defaultsAndOverrides().defaultPreferSecure, adapterId, routerInfo, locatorInfo,
                          _instance.defaultsAndOverrides().defaultCollocationOptimization, true,
                          _instance.defaultsAndOverrides().defaultEndpointSelection, _instance.threadPerConnection(),
                          _instance.defaultsAndOverrides().defaultLocatorCacheTimeout);
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

    static private String[] _suffixes =
    {
        "EndpointSelection",
        "ConnectionCached",
        "PreferSecure",
        "LocatorCacheTimeout",
        "Locator",
        "Router",
        "CollocationOptimization",
        "ThreadPerConnection"
    };

    private void
    checkForUnknownProperties(String prefix)
    {
        //
        // Do not warn about unknown properties if Ice prefix, ie Ice, Glacier2, etc
        //      
        for(int i = 0; IceInternal.PropertyNames.clPropNames[i] != null; ++i)
        {
            if(prefix.startsWith(IceInternal.PropertyNames.clPropNames[i] + "."))
            {
                return;
            }
        }

        java.util.ArrayList unknownProps = new java.util.ArrayList();
        java.util.Map props = _instance.initializationData().properties.getPropertiesForPrefix(prefix + ".");
        java.util.Iterator p = props.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String prop = (String)entry.getKey();

            boolean valid = false;
            for(int i = 0; i < _suffixes.length; ++i)
            {
                if(prop.equals(prefix + "." + _suffixes[i]))
                {
                    valid = true;
                    break;
                }
            }

            if(!valid)
            {
                unknownProps.add(prop);
            }
        }

        if(unknownProps.size() != 0)
        {
            String message = "found unknown properties for proxy '" + prefix + "':";
            p = unknownProps.iterator();
            while(p.hasNext())
            {
                message += "\n    " + (String)p.next();
            }
            _instance.initializationData().logger.warning(message);
        }
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
    private Ice.RouterPrx _defaultRouter;
    private Ice.LocatorPrx _defaultLocator;
    private java.util.WeakHashMap _references = new java.util.WeakHashMap();
}
