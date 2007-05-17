// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class BlobjectAsync extends Ice.ObjectImpl
{
    public abstract void
    ice_invoke_async(AMD_Object_ice_invoke cb, byte[] inParams, Current current);

    public DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        byte[] inParams;
        int sz = in.is().getReadEncapsSize();
        inParams = in.is().readBlob(sz);
        AMD_Object_ice_invoke cb = new _AMD_Object_ice_invoke(in);
        try
        {
            ice_invoke_async(cb, inParams, current);
        }
        catch(java.lang.Exception ex)
        {
            cb.ice_exception(ex);
        }
        return DispatchStatus.DispatchAsync;
    }
}
