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

public class _ObjectDelD implements _ObjectDel
{
    public boolean
    ice_isA(String s)
        throws LocationForward, IceInternal.NonRepeatable
    {
        IceInternal.Direct __direct =
            new IceInternal.Direct(__adapter, __reference, "ice_isA");
        return __direct.facetServant().ice_isA(s);
    }

    public void
    ice_ping()
        throws LocationForward, IceInternal.NonRepeatable
    {
        IceInternal.Direct __direct =
            new IceInternal.Direct(__adapter, __reference, "ice_ping");
        __direct.facetServant().ice_ping();
    }

    public void
    ice_flush()
    {
        // Nothing to do for direct delegates
    }

    protected ObjectAdapter __adapter;
    protected IceInternal.Reference __reference;

    //
    // Only for use by ObjectPrx
    //
    final void
    setup(IceInternal.Reference ref, ObjectAdapter adapter)
    {
        //
        // No need to synchronize, as this operation is only called
        // upon initial initialization.
        //
        __reference = ref;
        __adapter = adapter;
    }
}
