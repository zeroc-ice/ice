// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class _ObjectDelM implements _ObjectDel
{
    public boolean
    ice_isA(String __id, java.util.Map __context)
        throws LocationForward, IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out =
            new IceInternal.Outgoing(__connection, __reference, "ice_isA",
                                     true, __context);
        IceInternal.BasicStream __is = __out.is();
        IceInternal.BasicStream __os = __out.os();
        __os.writeString(__id);
        if (!__out.invoke())
        {
            throw new UnknownUserException();
        }
        return __is.readBool();
    }

    public void
    ice_ping(java.util.Map __context)
        throws LocationForward, IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out =
            new IceInternal.Outgoing(__connection, __reference, "ice_ping",
                                     true, __context);
        if (!__out.invoke())
        {
            throw new UnknownUserException();
        }
    }

    public byte[]
    ice_invoke(String operation, boolean nonmutating, byte[] inParams,
               java.util.Map __context)
        throws LocationForward, IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out =
            new IceInternal.Outgoing(__connection, __reference, operation,
                                     nonmutating, __context);
        IceInternal.BasicStream __os = __out.os();
        __os.writeBlob(inParams);
        __out.invoke();
        if (__reference.mode == IceInternal.Reference.ModeTwoway)
        {
            IceInternal.BasicStream __is = __out.is();
            int sz = __is.getReadEncapsSize();
            return __is.readBlob(sz);
        }
        return null;
    }

    public void
    ice_flush()
    {
        __connection.flushBatchRequest();
    }

    //
    // Only for use by ObjectPrx
    //
    final void
    __copyFrom(_ObjectDelM from)
    {
        //
        // No need to synchronize "from", as the delegate is immutable
        // after creation.
        //

        //
        // No need to synchronize, as this operation is only called
        // upon initialization.
        //

        __reference = from.__reference;
        __connection = from.__connection;
    }

    protected IceInternal.Connection __connection;
    protected IceInternal.Reference __reference;

    public void
    setup(IceInternal.Reference ref)
    {
        //
        // No need to synchronize, as this operation is only called
        // upon initialization.
        //
        __reference = ref;

        if (__reference.reverseAdapter != null)
        {
            //
            // If we have a reverse object adapter, we use the incoming
            // connections from such object adapter.
            //
            ObjectAdapterI adapter = (ObjectAdapterI)__reference.reverseAdapter;
            IceInternal.Connection[] connections =
                adapter.getIncomingConnections();

            IceInternal.Endpoint[] endpoints =
                new IceInternal.Endpoint[connections.length];
            for (int i = 0; i < connections.length; i++)
            {
                endpoints[i] = connections[i].endpoint();
            }
            endpoints = filterEndpoints(endpoints);

            if (endpoints.length == 0)
            {
                throw new NoEndpointException();
            }

            int j;
            for (j = 0; j < connections.length; j++)
            {
                if (connections[j].endpoint().equals(endpoints[0]))
                {
                    break;
                }
            }
            assert(j < connections.length);
            __connection = connections[j];
        }
        else
        {
            IceInternal.Endpoint[] endpoints = null;
            if (__reference.routerInfo != null)
            {
                //
                // If we route, we send everything to the router's client
                // proxy endpoints.
                //
                ObjectPrx proxy = __reference.routerInfo.getClientProxy();
                endpoints = filterEndpoints(((ObjectPrxHelper)proxy).__reference().endpoints);
            }
            else
            {
                endpoints = filterEndpoints(__reference.endpoints);
            }

            if (endpoints.length == 0)
            {
                throw new NoEndpointException();
            }

            IceInternal.OutgoingConnectionFactory factory =
                __reference.instance.outgoingConnectionFactory();
            __connection = factory.create(endpoints);
            assert(__connection != null);

            //
            // If we have a router, add the object adapter for this router (if
            // any) to the new connection, so that callbacks from the router
            // can be received over this new connection.
            //
            if (__reference.routerInfo != null)
            {
                __connection.setAdapter(__reference.routerInfo.getAdapter());
            }
        }
    }

    private IceInternal.Endpoint[]
    filterEndpoints(IceInternal.Endpoint[] allEndpoints)
    {
        java.util.ArrayList endpoints = new java.util.ArrayList();

        //
        // Filter out unknown endpoints.
        //
        for (int i = 0; i < allEndpoints.length; i++)
        {
            if (!allEndpoints[i].unknown())
            {
                endpoints.add(allEndpoints[i]);
            }
        }

        switch (__reference.mode)
        {
            case IceInternal.Reference.ModeTwoway:
            case IceInternal.Reference.ModeOneway:
            case IceInternal.Reference.ModeBatchOneway:
            {
                //
                // Filter out datagram endpoints.
                //
                java.util.Iterator i = endpoints.iterator();
                while (i.hasNext())
                {
                    IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.next();
                    if (endpoint.datagram())
                    {
                        i.remove();
                    }
                }
                break;
            }

            case IceInternal.Reference.ModeDatagram:
            case IceInternal.Reference.ModeBatchDatagram:
            {
                //
                // Filter out non-datagram endpoints.
                //
                java.util.Iterator i = endpoints.iterator();
                while (i.hasNext())
                {
                    IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.next();
                    if (!endpoint.datagram())
                    {
                        i.remove();
                    }
                }
                break;
            }
        }

        //
        // Randomize the order of endpoints.
        //
        java.util.Collections.shuffle(endpoints);

        //
        // If a secure connection is requested, remove all non-secure
        // endpoints. Otherwise make non-secure endpoints preferred over
        // secure endpoints by partitioning the endpoint vector, so that
        // non-secure endpoints come first.
        //
        if (__reference.secure)
        {
            java.util.Iterator i = endpoints.iterator();
            while (i.hasNext())
            {
                IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.next();
                if (!endpoint.secure())
                {
                    i.remove();
                }
            }
        }
        else
        {
            java.util.Collections.sort(endpoints, __comparator);
        }

        IceInternal.Endpoint[] arr = new IceInternal.Endpoint[endpoints.size()];
        endpoints.toArray(arr);
        return arr;
    }

    private static class EndpointComparator implements java.util.Comparator
    {
        public int
        compare(java.lang.Object l, java.lang.Object r)
        {
            IceInternal.Endpoint le = (IceInternal.Endpoint)l;
            IceInternal.Endpoint re = (IceInternal.Endpoint)r;
            boolean ls = le.secure();
            boolean rs = re.secure();
            if ((ls && rs) || (!ls && !rs))
            {
                return 0;
            }
            else if (!ls && rs)
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
    }
    private static EndpointComparator __comparator = new EndpointComparator();
}
