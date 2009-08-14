// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback object for {@link Blobject} AMI invocations.
 *
 * @see Blobject
 **/
public abstract class AMI_Object_ice_invoke extends IceInternal.OutgoingAsync
{
    /**
     * The Ice run time calls <code>ice_response</code> when an asynchronous operation invocation
     * completes successfully or raises a user exception.
     *
     * @param ok Indicates the result of the invocation. If <code>true</code>, the operation
     * completed succesfully; if <code>false</code>, the operation raised a user exception.
     * @param outParams Contains the encoded out-parameters of the operation (if any) if <code>ok</code>
     * is <code>true</code>; otherwise, if <code>ok</code> is <code>false</code>, contains the
     * encoded user exception raised by the operation.
     **/
    public abstract void ice_response(boolean ok, byte[] outParams);

    /**
     * The Ice run time calls <code>ice_exception</code> when an asynchronous operation invocation
     * raises an Ice run-time exception.
     *
     * @param ex The encoded Ice run-time exception raised by the operation.
     **/
    public abstract void ice_exception(LocalException ex);

    public final boolean __invoke(Ice.ObjectPrx prx, String operation, OperationMode mode,
                                  byte[] inParams, java.util.Map<String, String> context)
    {
        __acquireCallback(prx);
        try
        {
            __prepare(prx, operation, mode, context);
            __os.writeBlob(inParams);
            __os.endWriteEncaps();
            return __send();
        }
        catch(LocalException ex)
        {
            __releaseCallback(ex);
            return false;
        }
    }

    protected final void __response(boolean ok) // ok == true means no user exception.
    {
        byte[] outParams;
        try
        {
            __is.startReadEncaps();
            int sz = __is.getReadEncapsSize();
            outParams = __is.readBlob(sz);
            __is.endReadEncaps();
        }
        catch(LocalException ex)
        {
            __finished(ex);
            return;
        }
        ice_response(ok, outParams);
        __releaseCallback();
    }
}
