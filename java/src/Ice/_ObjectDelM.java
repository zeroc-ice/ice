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
    ice_isA(String __id, java.util.HashMap __context)
        throws LocationForward, IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out =
            new IceInternal.Outgoing(__emitter, __reference, false, "ice_isA",
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
    ice_ping(java.util.HashMap __context)
        throws LocationForward, IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out =
            new IceInternal.Outgoing(__emitter, __reference, false, "ice_ping",
                                     true, __context);
        if (!__out.invoke())
        {
            throw new UnknownUserException();
        }
    }

    public byte[]
    ice_invoke(String operation, boolean nonmutating, byte[] inParams,
               java.util.HashMap __context)
        throws LocationForward, IceInternal.NonRepeatable
    {
        IceInternal.Outgoing __out =
            new IceInternal.Outgoing(__emitter, __reference, false, operation,
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
        __emitter.flushBatchRequest();
    }

    protected IceInternal.Emitter __emitter;
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

        java.util.LinkedList endpoints = new java.util.LinkedList();
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

        if (__reference.secure)
        {
            java.util.ListIterator i = endpoints.listIterator();
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
            java.util.ListIterator i = endpoints.listIterator();
            while (i.hasNext())
            {
                IceInternal.Endpoint endpoint = (IceInternal.Endpoint)i.next();
                if (endpoint.secure())
                {
                    i.remove();
                }
            }
        }

        final int sz = endpoints.size();
        if (endpoints.isEmpty())
        {
            throw new NoEndpointException();
        }

        IceInternal.Endpoint[] arr =
            new IceInternal.Endpoint[endpoints.size()];
        endpoints.toArray(arr);

        // TODO
        //random_shuffle(endpoints.begin(), endpoints.end());

        IceInternal.EmitterFactory factory =
            __reference.instance.emitterFactory();
        __emitter = factory.create(arr);
        assert(__emitter != null);
    }
}
