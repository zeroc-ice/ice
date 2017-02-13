// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * <code>BlobjectAsync</code> is the base class for asynchronous dynamic
 * dispatch servants. A server application derives a concrete servant
 * class that implements the {@link BlobjectAsync#ice_invoke_async} method,
 * which is called by the Ice run time to deliver every request on this
 * object.
 **/
public abstract class BlobjectAsync extends Ice.ObjectImpl
{
    /**
     * Dispatch an incoming request.
     *
     * @param cb The callback object through which the invocation's results
     * must be delivered.
     * @param inEncaps The encoded input parameters.
     * @param current The Current object, which provides important information
     * about the request, such as the identity of the target object and the
     * name of the operation.
     **/
    public abstract void
    ice_invoke_async(AMD_Object_ice_invoke cb, byte[] inEncaps, Current current);

    @Override
    public DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        byte[] inEncaps = in.readParamEncaps();
        AMD_Object_ice_invoke cb = new _AMD_Object_ice_invoke(in);
        try
        {
            ice_invoke_async(cb, inEncaps, current);
        }
        catch(java.lang.Exception ex)
        {
            cb.ice_exception(ex);
        }
        return DispatchStatus.DispatchAsync;
    }
}
