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
            new IceInternal.Outgoing(__connection, __reference, false,
                                     "ice_isA", true, __context);
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
            new IceInternal.Outgoing(__connection, __reference, false,
                                     "ice_ping", true, __context);
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
            new IceInternal.Outgoing(__connection, __reference, false,
                                     operation, nonmutating, __context);
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

    protected IceInternal.Connection __connection;
    protected IceInternal.Reference __reference;

    //
    // Only for use by ObjectPrx
    //
    final void
    setup(IceInternal.Reference ref)
    {
        //
        // No need to synchronize, as this operation is only called
        // upon initial initialization.
        //
        __reference = ref;

        java.util.ArrayList endpoints = new java.util.ArrayList();
        switch (__reference.mode)
        {
            case IceInternal.Reference.ModeTwoway:
            case IceInternal.Reference.ModeOneway:
            case IceInternal.Reference.ModeBatchOneway:
            {
                for (int i = 0; i < __reference.endpoints.length; i++)
                {
                    if (!__reference.endpoints[i].datagram())
                    {
                        endpoints.add(__reference.endpoints[i]);
                    }
                }
                break;
            }

            case IceInternal.Reference.ModeDatagram:
            case IceInternal.Reference.ModeBatchDatagram:
            {
                for (int i = 0; i < __reference.endpoints.length; i++)
                {
                    if (__reference.endpoints[i].datagram())
                    {
                        endpoints.add(__reference.endpoints[i]);
                    }
                }
                break;
            }
        }

        //
        // Randomize the order of endpoints.
        //
        random_shuffle(endpoints);

        IceInternal.Endpoint[] arr;

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
            arr = new IceInternal.Endpoint[endpoints.size()];
            endpoints.toArray(arr);
        }
        else
        {
            arr = new IceInternal.Endpoint[endpoints.size()];
            endpoints.toArray(arr);
            java.util.Arrays.sort((java.lang.Object[])arr, __comparator);
        }

        if (arr.length == 0)
        {
            throw new NoEndpointException();
        }

        IceInternal.OutgoingConnectionFactory factory =
            __reference.instance.outgoingConnectionFactory();
        __connection = factory.create(arr);
        assert(__connection != null);
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

    private static java.util.Random __random = new java.util.Random();

    private static void
    random_shuffle(java.util.ArrayList arr)
    {
        final int sz = arr.size();
        for (int i = 0; i < sz; i++)
        {
            int pos = Math.abs(__random.nextInt() % sz);
            java.lang.Object tmp = arr.get(pos);
            arr.set(pos, arr.get(i));
            arr.set(i, tmp);
        }
    }
}
