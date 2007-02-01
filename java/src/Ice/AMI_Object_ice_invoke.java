// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    public final void __invoke(Ice.ObjectPrx prx, String operation, OperationMode mode,
                               byte[] inParams, java.util.Map context)
    {
        try
        {
            __prepare(prx, operation, mode, context);
            __os.writeBlob(inParams);
            __os.endWriteEncaps();
        }
        catch(LocalException ex)
        {
            __finished(ex);
            return;
        }
        __send();
    }

    protected final void __response(boolean ok) // ok == true means no user exception.
    {
        byte[] outParams;
        try
        {
            int sz = __is.getReadEncapsSize();
            outParams = __is.readBlob(sz);
        }
        catch(LocalException ex)
        {
            __finished(ex);
            return;
        }
        ice_response(ok, outParams);
    }
}
