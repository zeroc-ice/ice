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
    public final static int ModeBatchLast = ModeBatchDatagram;

    public
    Reference(Instance inst, String ident, String fac, int md, boolean sec,
              Endpoint[] origEndpts, Endpoint[] endpts)
    {
        instance = inst;
        identity = ident;
        facet = fac;
        mode = md;
        secure = sec;
        origEndpoints = origEndpts;
        endpoints = endpts;
        hashValue = 0;

        calcHashValue();
    }

    public
    Reference(Instance inst, String str)
    {
        instance = inst;
        mode = ModeTwoway;
        secure = false;
        hashValue = 0;

        String s = str.trim();
        if (s.length() == 0)
        {
            throw new Ice.ReferenceParseException();
        }

        int colon = s.indexOf(':');
        String init;
        if (colon == -1)
        {
            init = s;
        }
        else
        {
            init = s.substring(0, colon);
        }

        String[] arr = init.split("[ \t\n\r]+");
        String identity = arr[0];

        int i = 1;
        while (i < arr.length)
        {
            String option = arr[i++];
            if (option.length() != 2 || option.charAt(0) != '-')
            {
                throw new Ice.ReferenceParseException();
            }

            String argument = null;
            if (i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
            }

            switch (option.charAt(1))
            {
                case 'f':
                {
                    if (argument == null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    facet = argument;
                    break;
                }

                case 't':
                {
                    if (argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = ModeTwoway;
                    break;
                }

                case 'o':
                {
                    if (argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = ModeOneway;
                    break;
                }

                case 'O':
                {
                    if (argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = ModeBatchOneway;
                    break;
                }

                case 'd':
                {
                    if (argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = ModeDatagram;
                    break;
                }

                case 'D':
                {
                    if (argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    mode = ModeBatchDatagram;
                    break;
                }

                case 's':
                {
                    if (argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    secure = true;
                    break;
                }

                default:
                {
                    if (argument != null)
                    {
                        throw new Ice.EndpointParseException();
                    }

                    throw new Ice.ReferenceParseException();
                }
            }
        }

        java.util.LinkedList origEndpointList = new java.util.LinkedList();
        java.util.LinkedList endpointList = new java.util.LinkedList();
        boolean orig = true;
        final int len = s.length();
        int end = colon;
        while (end < len && s.charAt(end) == ':')
        {
            int beg = end + 1;

            end = s.indexOf(':', beg);
            if (end == -1)
            {
                end = len;
            }

            if (beg == end) // "::"
            {
                if (!orig)
                {
                    throw new Ice.ReferenceParseException();
                }

                orig = false;
                continue;
            }

            String es = s.substring(beg, end);
            Endpoint endp = Endpoint.endpointFromString(es);

            if (orig)
            {
                origEndpointList.add(endp);
            }
            else
            {
                endpointList.add(endp);
            }
        }

        origEndpoints = new Endpoint[origEndpointList.size()];
        origEndpointList.toArray(origEndpoints);

        if (orig)
        {
            endpoints = origEndpoints;
        }
        else
        {
            endpoints = new Endpoint[endpointList.size()];
            endpointList.toArray(endpoints);
        }

        if (origEndpoints.length == 0 || endpoints.length == 0)
        {
            throw new Ice.ReferenceParseException();
        }

        calcHashValue();
    }

    public
    Reference(String ident, Ice.Stream s)
    {
        instance = s.instance();
        identity = ident;
        mode = ModeTwoway;
        secure = false;
        hashValue = 0;

        //
        // Don't read the identity here. Operations calling this
        // constructor read the identity, and pass it as a parameter.
        //

        facet = s.readString();

        mode = (int)s.readByte();
        if (mode < 0 || mode > ModeBatchLast)
        {
            throw new Ice.ProxyUnmarshalException();
        }

        secure = s.readBoolean();

        int sz = s.readInt();
        origEndpoints = new Endpoint[sz];
        for (int i = 0; i < sz; i++)
        {
            origEndpoints[i] = Endpoint.streamRead(s);
        }

        boolean same = s.readBoolean();
        if (same) // origEndpoints == endpoints
        {
            endpoints = origEndpoints;
        }
        else
        {
            sz = s.readInt();
            endpoints = new Endpoint[sz];
            for (int i = 0; i < sz; i++)
            {
                endpoints[i] = Endpoint.streamRead(s);
            }
        }

        calcHashValue();
    }

    public boolean
    equals(java.lang.Object obj)
    {
        Reference r = (Reference)obj;

        if (this == r)
        {
            return true;
        }

        if (!identity.equals(r.identity))
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

        return true;
    }

    //
    // Marshal the reference
    //
    public void
    streamWrite(Ice.Stream s)
    {
        //
        // Don't write the identity here. Operations calling streamWrite
        // write the identity.
        //

        s.writeString(facet);

        s.writeByte((byte)mode);

        s.writeBoolean(secure);

        s.writeInt(origEndpoints.length);
        for (int i = 0; i < origEndpoints.length; i++)
        {
            origEndpoints[i].streamWrite(s);
        }

        if (endpointsEqual())
        {
            s.writeBoolean(true);
        }
        else
        {
            s.writeBoolean(false);
            s.writeInt(endpoints.length);
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
        s.append(identity);

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
    public String identity;
    public String facet;
    public int mode;
    public boolean secure;
    public Endpoint[] origEndpoints; // Original endpoints
    public Endpoint[] endpoints; // Actual endpoints (set by a loc fwd)
    public int hashValue;

    //
    // Get a new reference, based on the existing one, overwriting
    // certain values.
    //
    public Reference
    changeIdentity(String newIdentity)
    {
        if (newIdentity.equals(identity))
        {
            return this;
        }
        else
        {
            return new Reference(instance, newIdentity, facet, mode, secure,
                                 origEndpoints, endpoints);
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
            return new Reference(instance, identity, newFacet, mode, secure,
                                 origEndpoints, endpoints);
        }
    }

    public Reference
    changeTimeout(int timeout)
    {
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

        Reference ref = new Reference(instance, identity, facet, mode, secure,
                                      newOrigEndpoints, newEndpoints);

        if (ref.equals(this))
        {
            return this;
        }

        return ref;
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
            return new Reference(instance, identity, facet, newMode, secure,
                                 origEndpoints, endpoints);
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
            return new Reference(instance, identity, facet, mode, newSecure,
                                 origEndpoints, endpoints);
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
            return new Reference(instance, identity, facet, mode, newSecure,
                                 origEndpoints, newEndpoints);
        }
    }

    private void
    calcHashValue()
    {
        int h = 0;

        int sz = identity.size();
        for (int i = 0; i < sz; i++)
        {   
            h = 5 * h + (int)identity.charAt(i);
        }

        sz = facet.size();
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
            _checkEndpointEqual = false;
        }

        return _endpointEqual;
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
