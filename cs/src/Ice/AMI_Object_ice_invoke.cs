// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************
namespace Ice
{

    public abstract class AMI_Object_ice_invoke : IceInternal.OutgoingAsync
    {
	public abstract void ice_response(bool ok, ByteSeq outParams);
	public abstract override void ice_exception(Ice.LocalException ex);

	public void __invoke(IceInternal.Reference r, string operation, OperationMode mode,
		             ByteSeq inParams, Ice.Context context)
	{
	    try
	    {
	        __prepare(r, operation, mode, context);
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
	    ByteSeq outParams;
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
