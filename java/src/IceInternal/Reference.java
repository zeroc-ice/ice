// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

        if (r == null)
        {
            return false;
        }

        if (this == r)
        {
            return true;
        }

        if (!identity.category.equals(r.identity.category))
        {
            return false;
        }

        if (!identity.name.equals(r.identity.name))
        {
            return false;
        }

        if (facet != null && !facet.equals(r.facet))
        {
            return false;
        }

        if (mode != r.mode)
        {
            return false;
        }

        if (secure != r.secure)
        {
            return false;
        }

        if (!compare(origEndpoints, r.origEndpoints))
        {
            return false;
        }

        if (!compare(endpoints, r.endpoints))
        {
            return false;
        }

        if (routerInfo != r.routerInfo)
        {
            return false;
        }

        if (routerInfo != null && r.routerInfo != null &&
            !routerInfo.equals(r.routerInfo))
        {
            return false;
        }

        if (reverseAdapter != r.reverseAdapter)
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

        s.writeString(facet);

        s.writeByte((byte)mode);

        s.writeBool(secure);

        s.writeSize(origEndpoints.length);
        for (int i = 0; i < origEndpoints.length; i++)
        {
            origEndpoints[i].streamWrite(s);
        }

        if (endpointsEqual())
        {
            s.writeBool(true);
        }
        else
        {
            s.writeBool(false);
            s.writeSize(endpoints.length);
            for (int i = 0; i < endpoints.length; i++)
            {
                endpoints[i].streamWrite(s);
            }
        }
    }

    //
    // Convert the reference to its string form
    //
    public String
    toString()
    {
        StringBuffer s = new StringBuffer();
        s.append(Ice.Util.identityToString(identity));

        if (facet.length() > 0)
        {
            s.append(" -f ");
            s.append(facet);
        }

        switch (mode)
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

        if (secure)
        {
            s.append(" -s");
        }

        for (int i = 0; i < origEndpoints.length; i++)
        {
            s.append(':');
            s.append(origEndpoints[i].toString());
        }

        if (!endpointsEqual())
        {
            s.append(':');
            for (int i = 0; i < endpoints.length; i++)
            {
                s.append(':');
                s.append(endpoints[i].toString());
            }
        }

        return s.toString();
    }

    //
    // All members are treated as const, because References are immutable.
    //
    public Instance instance;
    public Ice.Identity identity;
    public String facet;
    public int mode;
    public boolean secure;
    public Endpoint[] origEndpoints; // Original endpoints.
    public Endpoint[] endpoints; // Actual endpoints, changed by a location forward.
    public RouterInfo routerInfo; // Null if no router is used.
    public Ice.ObjectAdapter reverseAdapter; // For reverse communications using the adapter's incoming connections.
    public int hashValue;

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    public Reference
    changeIdentity(Ice.Identity newIdentity)
    {
        if (newIdentity.equals(identity))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(newIdentity, facet, mode, secure,
                                                      origEndpoints, endpoints,
                                                      routerInfo, reverseAdapter);
        }
    }

    public Reference
    changeFacet(String newFacet)
    {
        if (newFacet.equals(facet))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, newFacet, mode, secure,
                                                      origEndpoints, endpoints,
                                                      routerInfo, reverseAdapter);
        }
    }

    public Reference
    changeTimeout(int timeout)
    {
        //
        // We change the timeout settings in all endpoints.
        //
        Endpoint[] newOrigEndpoints = new Endpoint[origEndpoints.length];
        for (int i = 0; i < origEndpoints.length; i++)
        {
            newOrigEndpoints[i] = origEndpoints[i].timeout(timeout);
        }

        Endpoint[] newEndpoints = new Endpoint[endpoints.length];
        for (int i = 0; i < endpoints.length; i++)
        {
            newEndpoints[i] = endpoints[i].timeout(timeout);
        }

        //
        // If we have a router, we also change the timeout settings on the
        // router and the router's client proxy.
        //
        RouterInfo newRouterInfo = null;
        if (routerInfo != null)
        {
            try
            {
                Ice.RouterPrx newRouter =
                    Ice.RouterPrxHelper.uncheckedCast(routerInfo.getRouter().ice_timeout(timeout));
                Ice.ObjectPrx newClientProxy = routerInfo.getClientProxy().ice_timeout(timeout);
                newRouterInfo = instance.routerManager().get(newRouter);
                newRouterInfo.setClientProxy(newClientProxy);
            }
            catch (Ice.NoEndpointException ex)
            {
                // Ignore non-existing client proxies.
            }
        }

        return instance.referenceFactory().create(identity, facet, mode, secure,
                                                  newOrigEndpoints, newEndpoints,
                                                  newRouterInfo, reverseAdapter);
    }

    public Reference
    changeMode(int newMode)
    {
        if (newMode == mode)
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, newMode, secure,
                                                      origEndpoints, endpoints,
                                                      routerInfo, reverseAdapter);
        }
    }

    public Reference
    changeSecure(boolean newSecure)
    {
        if (newSecure == secure)
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, newSecure,
                                                      origEndpoints, endpoints,
                                                      routerInfo, reverseAdapter);
        }
    }

    public Reference
    changeEndpoints(Endpoint[] newEndpoints)
    {
        if (compare(newEndpoints, endpoints))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, secure,
                                                      origEndpoints, newEndpoints,
                                                      routerInfo, reverseAdapter);
        }
    }

    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        RouterInfo newRouterInfo = instance.routerManager().get(newRouter);

        if ((routerInfo == newRouterInfo) ||
            (routerInfo != null && newRouterInfo != null && newRouterInfo.equals(routerInfo)))
        {
            return this;
        }
        else
        {
            return instance.referenceFactory().create(identity, facet, mode, secure,
                                                      origEndpoints, endpoints,
                                                      newRouterInfo, reverseAdapter);
        }
    }

    public Reference
    changeDefault()
    {
        return instance.referenceFactory().create(identity, "", ModeTwoway, false,
                                                  origEndpoints, origEndpoints,
                                                  null, null);
    }

    //
    // Only for use by ReferenceFactory
    //
    Reference(Instance inst,
              Ice.Identity ident,
              String fac,
              int md,
              boolean sec,
              Endpoint[] origEndpts,
              Endpoint[] endpts,
              RouterInfo rtrInfo,
              Ice.ObjectAdapter rvAdapter)
    {
        instance = inst;
        identity = ident;
        facet = fac;
        mode = md;
        secure = sec;
        origEndpoints = origEndpts;
        endpoints = endpts;
        routerInfo = rtrInfo;
        reverseAdapter = rvAdapter;
        hashValue = 0;

        calcHashValue();
    }

    private void
    calcHashValue()
    {
        int h = 0;

        int sz = identity.name.length();
        for (int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)identity.name.charAt(i);
        }

        sz = identity.category.length();
        for (int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)identity.category.charAt(i);
        }

        sz = facet.length();
        for (int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)facet.charAt(i);
        }

        h = 5 * h + mode;

        h = 5 * h + (secure ? 1 : 0);

        //
        // TODO: Should we also take the endpoints into account for hash
        // calculation? Perhaps not, the code above should be good enough
        // for a good hash value.
        //

        hashValue = h;
    }

    //
    // Check if origEndpoints == endpoints
    //
    private boolean
    endpointsEqual()
    {
        if (_checkEndpointsEqual)
        {
            _endpointsEqual = compare(origEndpoints, endpoints);
            _checkEndpointsEqual = false;
        }

        return _endpointsEqual;
    }

    private boolean
    compare(Endpoint[] arr1, Endpoint[] arr2)
    {
        if (arr1 == arr2)
        {
            return true;
        }

        if (arr1.length == arr2.length)
        {
            for (int i = 0; i < arr1.length; i++)
            {
                if (!arr1[i].equals(arr2[i]))
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    private boolean _endpointsEqual = false;
    private boolean _checkEndpointsEqual = true;
}
