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

public class _ObjectDisp implements Dispatcher
{
    public
    _ObjectDisp()
    {
        _delegate = null;
    }

    public
    _ObjectDisp(Ice.Object delegate)
    {
        _delegate = delegate;
    }

    public static IceInternal.DispatchStatus
    ice_isA(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
        IceInternal.BasicStream __is = __in.is();
        IceInternal.BasicStream __os = __in.os();
        String __id = __is.readString();
        boolean __ret = __obj.ice_isA(__id, __current);
        __os.writeBool(__ret);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    public static IceInternal.DispatchStatus
    ice_ping(Ice.Object __obj, IceInternal.Incoming __in, Current __current)
    {
        __obj.ice_ping(__current);
        return IceInternal.DispatchStatus.DispatchOK;
    }

    private static String[] __all =
    {
        "ice_isA",
        "ice_ping"
    };

    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        int pos = java.util.Arrays.binarySearch(__all, current.operation);
        if (pos < 0)
        {
            return IceInternal.DispatchStatus.DispatchOperationNotExist;
        }

        switch (pos)
        {
            case 0:
            {
                return ice_isA(_delegate, in, current);
            }
            case 1:
            {
                return ice_ping(_delegate, in, current);
            }
        }

        assert(false);
        return IceInternal.DispatchStatus.DispatchOperationNotExist;
    }

    private Ice.Object _delegate;
}
