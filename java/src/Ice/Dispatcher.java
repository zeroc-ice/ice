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

public interface Dispatcher
{
    IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current);
}
