// **********************************************************************
//
// Copyright (c) 2002
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

package IceInternal;

public final class Reference
{
    public final static int ModeTwoway = 0;
    public final static int ModeOneway = 1;
    public final static int ModeBatchOneway = 2;
    public final static int ModeDatagram = 3;
    public final static int ModeBatchDatagram = 4;
    public final static int ModeLast = ModeBatchDatagram;

    public int
    hashCode()
    {
        return hashValue;
    }

    public boolean
    equals(java.lang.Object obj)
    {
        Reference r = (Reference)obj;

        if(r == null)
        {
            return false;
        }

        if(this == r)
        {
            return true;
        }

        if(!identity.category.equals(r.identity.category))
        {
            return false;
        }

        if(!identity.name.equals(r.identity.name))
        {
            return false;
        }

        if(!java.util.Arrays.equals(facet, r.facet))
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

        if(compress != r.compress)
        {
            return false;
        }
	
	if(!adapterId.equals(r.adapterId))
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

        if(routerInfo != null && r.routerInfo != null &&
	   !routerInfo.equals(r.routerInfo))
        {
            return false;
        }

        if(locatorInfo != r.locatorInfo)
        {
            return false;
        }

        if(locatorInfo != null && r.locatorInfo != null &&
	   !locatorInfo.equals(r.locatorInfo))
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
    public void
    streamWrite(BasicStream s)
    {
        //
        // Don't write the identity here. Operations calling streamWrite
        // write the identity.
        //

        s.writeStringSeq(facet);

        s.writeByte((byte)mode);

        s.writeBool(secure);

        s.writeBool(compress);

	s.writeSize(endpoints.length);

	if(endpoints.length > 0)
	{
	    assert(adapterId.equals(""));
	    
            for(int i = 0; i < endpoints.length; i++)
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
    public String
    toString()
    {
        StringBuffer s = new StringBuffer();

        //
        // If the encoded identity string contains characters which
        // the reference parser uses as separators, then we enclose
        // the identity string in quotes.
        //
        String id = Ice.Util.identityToString(identity);
        if(StringUtil.findFirstOf(id, " \t\n\r:@") != -1)
        {
            s.append('"');
            s.append(id);
            s.append('"');
        }
        else
        {
            s.append(id);
        }

        if(facet.length > 0)
        {
            StringBuffer f = new StringBuffer();
            for(int i = 0; i < facet.length ; i++)
            {
                f.append(StringUtil.encodeString(facet[i], "/"));
                if(i < facet.length - 1)
                {
                    f.append('/');
                }
            }

            //
            // If the encoded facet string contains characters which
            // the reference parser uses as separators, then we enclose
            // the facet string in quotes.
            //
            s.append(" -f ");
            String fs = f.toString();
            if(StringUtil.findFirstOf(fs, " \t\n\r:@") != -1)
            {
                s.append('"');
                s.append(fs);
                s.append('"');
            }
            else
            {
                s.append(fs);
            }
        }

        switch(mode)
        {
            case ModeTwoway:
            {
                s.append(" -t");
                break;
            }

            case ModeOneway:
            {
                s.append(" -o");
                break;
            }

            case ModeBatchOneway:
            {
                s.append(" -O");
                break;
            }

            case ModeDatagram:
            {
                s.append(" -d");
                break;
            }

            case ModeBatchDatagram:
            {
                s.append(" -D");
                break;
            }
        }

        if(secure)
        {
            s.append(" -s");
        }

        if(compress)
        {
            s.append(" -c");
        }

	if(endpoints.length > 0)
	{
	    assert(adapterId.equals(""));

	    for(int i = 0; i < endpoints.length; i++)
	    {
		s.append(':');
		s.append(endpoints[i].toString());
	    }
	}
        else
        {
            String a = StringUtil.encodeString(adapterId, null);
            //
            // If the encoded adapter id string contains characters which
            // the reference parser uses as separators, then we enclose
            // the adapter id string in quotes.
            //
            s.append(" @ ");
            if(StringUtil.findFirstOf(a, " \t\n\r") != -1)
            {
                s.append('"');
                s.append(a);
                s.append('"');
            }
            else
            {
                s.append(a);
            }
        }

        return s.toString();
    }

    //
    // All members are treated as const, because References are immutable.
    //
    final public Instance instance;
    final public Ice.Identity identity;
    final public String[] facet;
    final public int mode;
    final public boolean secure;
    final public boolean compress;
    final public String adapterId;
    final public Endpoint[] endpoints; // Actual endpoints, changed by a location forward.
    final public RouterInfo routerInfo; // Null if no router is used.
    final public LocatorInfo locatorInfo; // Null if no locator is used.
    final public Ice.ObjectAdapter reverseAdapter; // For reverse comm. using the adapter's incoming connections.
    final public boolean collocationOptimization;
    final public int hashValue;

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    public Reference
    changeIdentity(Ice.Identity newIdentity)
    {
        if(newIdentity.equals(identity))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(newIdentity, facet, mode, secure, compress, adapterId,
 						      endpoints, routerInfo, locatorInfo, reverseAdapter,
						      collocationOptimization);
        }
    }

    public Reference
    changeFacet(String[] newFacet)
    {
        if(java.util.Arrays.equals(facet, newFacet))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, newFacet, mode, secure, compress, adapterId,
						      endpoints, routerInfo, locatorInfo, reverseAdapter,
						      collocationOptimization);
        }
    }

    public Reference
    changeTimeout(int timeout)
    {
        //
        // We change the timeout settings in all endpoints.
        //
        Endpoint[] newEndpoints = new Endpoint[endpoints.length];
        for(int i = 0; i < endpoints.length; i++)
        {
            newEndpoints[i] = endpoints[i].timeout(timeout);
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
                Ice.RouterPrx newRouter =
		    Ice.RouterPrxHelper.uncheckedCast(routerInfo.getRouter().ice_timeout(timeout));
                Ice.ObjectPrx newClientProxy = routerInfo.getClientProxy().ice_timeout(timeout);
                newRouterInfo = instance.routerManager().get(newRouter);
                newRouterInfo.setClientProxy(newClientProxy);
            }
            catch(Ice.NoEndpointException ex)
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
	    Ice.LocatorPrx newLocator = 
		Ice.LocatorPrxHelper.uncheckedCast(locatorInfo.getLocator().ice_timeout(timeout));
	    newLocatorInfo = instance.locatorManager().get(newLocator);
	}

        return instance.referenceFactory().create(identity, facet, mode, secure, compress, adapterId, 
						  newEndpoints, newRouterInfo, newLocatorInfo, reverseAdapter,
						  collocationOptimization);
    }

    public Reference
    changeMode(int newMode)
    {
        if(newMode == mode)
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, newMode, secure, compress, adapterId,
						      endpoints, routerInfo, locatorInfo, reverseAdapter,
						      collocationOptimization);
        }
    }

    public Reference
    changeSecure(boolean newSecure)
    {
        if(newSecure == secure)
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, newSecure, compress, adapterId,
						      endpoints, routerInfo, locatorInfo, reverseAdapter,
						      collocationOptimization);
        }
    }

    public Reference
    changeCompress(boolean newCompress)
    {
        if(newCompress == compress)
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, secure, newCompress, adapterId,
						      endpoints, routerInfo, locatorInfo, reverseAdapter,
						      collocationOptimization);
        }
    }

    public Reference
    changeAdapterId(String newAdapterId)
    {
	if(adapterId.equals(newAdapterId))
	{
	    return this;
	}
	else
	{
	    return instance.referenceFactory().create(identity, facet, mode, secure, compress, newAdapterId,
						      endpoints, routerInfo, locatorInfo, reverseAdapter,
						      collocationOptimization);
	}
    }

    public Reference
    changeEndpoints(Endpoint[] newEndpoints)
    {
        if(compare(newEndpoints, endpoints))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, secure, compress, adapterId,
						      newEndpoints, routerInfo, locatorInfo, reverseAdapter,
						      collocationOptimization);
        }
    }

    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        RouterInfo newRouterInfo = instance.routerManager().get(newRouter);

        if((routerInfo == newRouterInfo) ||
            (routerInfo != null && newRouterInfo != null && newRouterInfo.equals(routerInfo)))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, secure, compress, adapterId,
						      endpoints, newRouterInfo, locatorInfo, reverseAdapter,
						      collocationOptimization);
        }
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
        LocatorInfo newLocatorInfo = instance.locatorManager().get(newLocator);

        if((locatorInfo == newLocatorInfo) ||
            (locatorInfo != null && newLocatorInfo != null && newLocatorInfo.equals(locatorInfo)))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, secure, compress, adapterId,
						      endpoints, routerInfo, newLocatorInfo, reverseAdapter,
						      collocationOptimization);
        }
    }

    public Reference
    changeCollocationOptimization(boolean newCollocationOptimization)
    {
        if(newCollocationOptimization == collocationOptimization)
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, secure, compress, adapterId,
						      endpoints, routerInfo, locatorInfo, reverseAdapter,
						      newCollocationOptimization);
        }
    }

    public Reference
    changeDefault()
    {
        RouterInfo routerInfo = instance.routerManager().get(instance.referenceFactory().getDefaultRouter());
        LocatorInfo locatorInfo = instance.locatorManager().get(instance.referenceFactory().getDefaultLocator());

        return instance.referenceFactory().create(identity, new String[0], ModeTwoway, false, false,
						  adapterId, endpoints,
                                                  routerInfo, locatorInfo, null, true);
    }

    //
    // Only for use by ReferenceFactory
    //
    Reference(Instance inst,
              Ice.Identity ident,
              String[] fac,
              int md,
              boolean sec,
              boolean com,
	      String adptId,
              Endpoint[] endpts,
              RouterInfo rtrInfo,
	      LocatorInfo locInfo,
              Ice.ObjectAdapter rvAdapter,
	      boolean collocationOpt)
    {
	//
	// It's either adapter id or endpoints, it can't be both.
	//
	assert(!(!adptId.equals("") && !(endpts.length == 0)));

        instance = inst;
        identity = ident;
        facet = fac;
        mode = md;
        secure = sec;
        compress = com;
	adapterId = adptId;
        endpoints = endpts;
        routerInfo = rtrInfo;
        locatorInfo = locInfo;
        reverseAdapter = rvAdapter;
	collocationOptimization = collocationOpt;

        int h = 0;

        int sz = identity.name.length();
        for(int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)identity.name.charAt(i);
        }

        sz = identity.category.length();
        for(int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)identity.category.charAt(i);
        }

        for(int i = 0; i < facet.length; i++)
        {
	    sz = facet[i].length();
	    for(int j = 0; j < sz; j++)
	    {
		h = 5 * h + (int)facet[i].charAt(j);
	    }
        }

        h = 5 * h + mode;

        h = 5 * h + (secure ? 1 : 0);

        h = 5 * h + (compress ? 1 : 0);

        //
        // TODO: Should we also take the endpoints into account for hash
        // calculation? Perhaps not, the code above should be good enough
        // for a good hash value.
        //

	hashValue = h;
    }

    private boolean
    compare(Endpoint[] arr1, Endpoint[] arr2)
    {
        if(arr1 == arr2)
        {
            return true;
        }

        if(arr1.length == arr2.length)
        {
            for(int i = 0; i < arr1.length; i++)
            {
                if(!arr1[i].equals(arr2[i]))
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }
}
