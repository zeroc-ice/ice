// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class FixedReference extends Reference
{
    public
    FixedReference(Instance inst,
                   Ice.Communicator com,
                   Ice.Identity ident,
                   java.util.Map ctx,
                   String fs,
                   int md,
                   Ice.ConnectionI[] fixedConns)
    {
        super(inst, com, ident, ctx, fs, md);
        _fixedConnections = fixedConns;
    }

    public boolean
    getSecure()
    {
        return false;
    }

    public boolean
    getPreferSecure()
    {
        return false;
    }

    public String
    getAdapterId()
    {
        return "";
    }

    public EndpointI[]
    getEndpoints()
    {
        return new EndpointI[0];
    }

    public boolean
    getCollocationOptimization()
    {
        return false;
    }
    
    public int
    getLocatorCacheTimeout()
    {
        return 0;
    }

    public final boolean
    getCacheConnection()
    {
        return false;
    }

    public final Ice.EndpointSelectionType
    getEndpointSelection()
    {
        return Ice.EndpointSelectionType.Random;
    }

    public boolean
    getThreadPerConnection()
    {
        return false;
    }

    public Reference
    changeSecure(boolean sec)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changePreferSecure(boolean prefSec)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeCollocationOptimization(boolean newCollocationOptimization)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeAdapterId(String newAdapterId)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeEndpoints(EndpointI[] newEndpoints)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeLocatorCacheTimeout(int newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    public final Reference
    changeCacheConnection(boolean newCache)
    {
        throw new Ice.FixedProxyException();
    }

    public final Reference
    changeEndpointSelection(Ice.EndpointSelectionType newType)
    {
        throw new Ice.FixedProxyException();
    }

    public final Reference
    changeThreadPerConnection(boolean newTpc)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeCompress(boolean newCompress)
    {
        // TODO: FixedReferences should probably have a _compress flag,
        // that gets its default from the fixed connection this reference
        // refers to. This should be changable with changeCompress().
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeTimeout(int newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeConnectionId(String connectionId)
    {
        throw new Ice.FixedProxyException();
    }

    public void
    streamWrite(BasicStream s)
        throws Ice.MarshalException
    {
        throw new Ice.FixedProxyException();
    }

    public String
    toString()
        throws Ice.MarshalException
    {
        throw new Ice.FixedProxyException();
    }

    public Ice.ConnectionI
    getConnection(Ice.BooleanHolder compress)
    {
        Ice.ConnectionI[] filteredConns = filterConnections(_fixedConnections);
        if(filteredConns.length == 0)
        {
            Ice.NoEndpointException ex = new Ice.NoEndpointException();
            ex.proxy = ""; // No stringified representation for fixed proxies.
            throw ex;
        }

        Ice.ConnectionI connection = filteredConns[0];
        assert(connection != null);
        connection.throwException(); // Throw in case our connection is already destroyed.
        compress.value = connection.endpoint().compress();

        return connection;
    }

    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
        {
            return true;
        }
        if(!(obj instanceof FixedReference))
        {
            return false;
        }
        FixedReference rhs = (FixedReference)obj;
        if(!super.equals(rhs))
        {
            return false;
        }
        return java.util.Arrays.equals(_fixedConnections, rhs._fixedConnections);
    }

    //
    // Filter connections based on criteria from this reference.
    //
    private Ice.ConnectionI[]
    filterConnections(Ice.ConnectionI[] allConnections)
    {
        java.util.ArrayList connections = new java.util.ArrayList(allConnections.length);

        switch(getMode())
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeBatchOneway:
            {
                //
                // Filter out datagram connections.
                //
                for(int i = 0; i < allConnections.length; ++i)
                {
                    if(!allConnections[i].endpoint().datagram())
                    {
                        connections.add(allConnections[i]);
                    }
                }

                break;
            }

            case Reference.ModeDatagram:
            case Reference.ModeBatchDatagram:
            {
                //
                // Filter out non-datagram connections.
                //
                for(int i = 0; i < allConnections.length; i++)
                {
                    if(allConnections[i].endpoint().datagram())
                    {
                        connections.add(allConnections[i]);
                    }
                }

                break;
            }
        }

        //
        // Randomize the order of connections.
        //
        java.util.Collections.shuffle(connections);

        //
        // If a secure connection is requested or secure overrides is
        // set, remove all non-secure endpoints. Otherwise make
        // non-secure endpoints preferred over secure endpoints by
        // partitioning the endpoint vector, so that non-secure
        // endpoints come first.
        //
        DefaultsAndOverrides overrides = getInstance().defaultsAndOverrides();
        if(overrides.overrideSecure ? overrides.overrideSecureValue : getSecure())
        {
            java.util.Iterator i = connections.iterator();
            while(i.hasNext())
            {
                Ice.ConnectionI connection = (Ice.ConnectionI)i.next();
                if(!connection.endpoint().secure())
                {
                    i.remove();
                }
            }
        }
        else
        {
            java.util.Collections.sort(connections, _connectionComparator);
        }

        Ice.ConnectionI[] arr = new Ice.ConnectionI[connections.size()];
        connections.toArray(arr);
        return arr;
    }

    static class ConnectionComparator implements java.util.Comparator
    {
        public int
        compare(java.lang.Object l, java.lang.Object r)
        {
            Ice.ConnectionI lc = (Ice.ConnectionI)l;
            Ice.ConnectionI rc = (Ice.ConnectionI)r;
            boolean ls = lc.endpoint().secure();
            boolean rs = rc.endpoint().secure();
            if((ls && rs) || (!ls && !rs))
            {
                return 0;
            }
            else if(!ls && rs)
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
    }
    
    private static ConnectionComparator _connectionComparator = new ConnectionComparator();
    private Ice.ConnectionI _fixedConnections[];
}
