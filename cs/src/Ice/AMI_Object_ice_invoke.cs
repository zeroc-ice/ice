// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    public abstract class AMI_Object_ice_invoke : IceInternal.OutgoingAsync
    {
	public abstract void ice_response(bool ok, byte[] outParams);
	public abstract override void ice_exception(Ice.Exception ex);

	public void __invoke(Ice.ObjectPrx prx, string operation, OperationMode mode,
		             byte[] inParams, Ice.Context context)
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

	protected internal override void __response(bool ok) // ok == true means no user exception.
	{
	    byte[] outParams;
	    try
	    {
		int sz = __is.getReadEncapsSize();
		outParams = __is.readBlob(sz);
	    }
	    catch(LocalException ex)
	    {
		ice_exception(ex);
		return;
	    }
	    ice_response(ok, outParams);
	}
    }
	
}
