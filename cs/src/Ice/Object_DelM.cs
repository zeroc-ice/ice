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

    using System.Collections;
    using System.Diagnostics;
    using IceUtil;
	    
    public class Object_DelM : Object_Del
    {
	public Object_DelM()
	{
	    __outgoingMutex = new System.Object();
	}

	public virtual bool ice_isA(string __id, Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_isA", OperationMode.Nonmutating, __context);
	    try
	    {
		IceInternal.BasicStream __is = __out.istr();
		IceInternal.BasicStream __os = __out.ostr();
		__os.writeString(__id);
		if(!__out.invoke())
		{
		    throw new UnknownUserException();
		}
		try
		{
		    return __is.readBool();
		}
		catch(LocalException __ex)
		{
		    throw new IceInternal.NonRepeatable(__ex);
		}
	    }
	    finally
	    {
		reclaimOutgoing(__out);
	    }
	}
	
	public virtual void ice_ping(Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_ping", OperationMode.Nonmutating, __context);
	    try
	    {
		if(!__out.invoke())
		{
		    throw new UnknownUserException();
		}
	    }
	    finally
	    {
		reclaimOutgoing(__out);
	    }
	}
	
	public virtual string[] ice_ids(Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_ids", OperationMode.Nonmutating, __context);
	    try
	    {
		IceInternal.BasicStream __is = __out.istr();
		if(!__out.invoke())
		{
		    throw new UnknownUserException();
		}
		try
		{
		    return __is.readStringSeq();
		}
		catch(LocalException __ex)
		{
		    throw new IceInternal.NonRepeatable(__ex);
		}
	    }
	    finally
	    {
		reclaimOutgoing(__out);
	    }
	}
	
	public virtual string ice_id(Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_id", OperationMode.Nonmutating, __context);
	    try
	    {
		IceInternal.BasicStream __is = __out.istr();
		if(!__out.invoke())
		{
		    throw new UnknownUserException();
		}
		try
		{
		    return __is.readString();
		}
		catch(LocalException __ex)
		{
		    throw new IceInternal.NonRepeatable(__ex);
		}
	    }
	    finally
	    {
		reclaimOutgoing(__out);
	    }
	}
	
	public virtual FacetPath ice_facets(Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing("ice_facets", OperationMode.Nonmutating, __context);
	    try
	    {
		IceInternal.BasicStream __is = __out.istr();
		if(!__out.invoke())
		{
		    throw new UnknownUserException();
		}
		try
		{
		    return __is.readFacetPath();
		}
		catch(LocalException __ex)
		{
		    throw new IceInternal.NonRepeatable(__ex);
		}
	    }
	    finally
	    {
		reclaimOutgoing(__out);
	    }
	}
	
	public virtual bool ice_invoke(string operation, OperationMode mode, byte[] inParams, out byte[] outParams,
	                               Ice.Context __context)
	{
	    IceInternal.Outgoing __out = getOutgoing(operation, mode, __context);
	    try
	    {
		IceInternal.BasicStream __os = __out.ostr();
		__os.writeBlob(inParams);
		bool ok = __out.invoke();
		outParams = null;
		if(__reference.mode == IceInternal.Reference.ModeTwoway)
		{
		    try
		    {
			IceInternal.BasicStream __is = __out.istr();
			int sz = __is.getReadEncapsSize();
			outParams = __is.readBlob(sz);
		    }
		    catch(LocalException __ex)
		    {
			throw new IceInternal.NonRepeatable(__ex);
		    }
		}
		return ok;
	    }
	    finally
	    {
		reclaimOutgoing(__out);
	    }
	}
	
	//
	// Only for use by ObjectPrx
	//
	internal void __copyFrom(Object_DelM from)
	{
	    //
	    // No need to synchronize "from", as the delegate is immutable
	    // after creation.
	    //
	    
	    //
	    // No need to synchronize, as this operation is only called
	    // upon initialization.
	    //
	    
	    Debug.Assert(__reference == null);
	    Debug.Assert(__connection == null);
	    
	    __reference = from.__reference;
	    __connection = from.__connection;
	}
	
	protected internal IceInternal.Reference __reference;
	protected internal IceInternal.Connection __connection;
	
	public virtual void setup(IceInternal.Reference rf)
	{
	    //
	    // No need to synchronize, as this operation is only called
	    // upon initialization.
	    //
	    
	    Debug.Assert(__reference == null);
	    Debug.Assert(__connection == null);
	    
	    __reference = rf;
	    __connection = __reference.getConnection();
	}
	
	protected internal virtual IceInternal.Outgoing getOutgoing(string operation, OperationMode mode,
	                                                            Ice.Context context)
	{
	    IceInternal.Outgoing outg;
	    
	    lock(__outgoingMutex)
	    {
		if(__outgoingCache == null)
		{
		    outg = new IceInternal.Outgoing(__connection, __reference, operation, mode, context);
		}
		else
		{
		    outg = __outgoingCache;
		    __outgoingCache = __outgoingCache.next;
		    outg.reset(operation, mode, context);
		}
	    }
	    
	    return outg;
	}
	
	protected internal virtual void reclaimOutgoing(IceInternal.Outgoing outg)
	{
	    lock(__outgoingMutex)
	    {
		outg.next = __outgoingCache;
		__outgoingCache = outg;
	    }
	}
	
	~Object_DelM()
	{
	    while(__outgoingCache != null)
	    {
		IceInternal.Outgoing next = __outgoingCache.next;
		__outgoingCache.destroy();
		__outgoingCache.next = null;
		__outgoingCache = next;
	    }
	}
	
	private IceInternal.Outgoing __outgoingCache;
	private System.Object __outgoingMutex;
    }
}
