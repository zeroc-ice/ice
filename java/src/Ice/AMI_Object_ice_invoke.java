// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class AMI_Object_ice_invoke extends IceInternal.OutgoingAsync
{
    public abstract void ice_response(boolean ok, byte[] outParams);
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
