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

package IceInternal;

public abstract class OutgoingAsync
{
    public
    OutgoingAsync()
    {
	__is = null;
	__os = null;
    }

    public abstract void ice_exception(Ice.LocalException ex);

    public final void
    __finished(BasicStream is)
    {
	//
	// No mutex protection necessary, this function can only be
	// called after __send() and __prepare() have completed.
	//

	assert(_reference != null);
	assert(_connection != null);

	int status;

	try
	{
	    if(__is != null)
	    {
		__is.destroy();
	    }
	    __is = new BasicStream(_reference.instance);
            __is.swap(is);

            status = (int)__is.readByte();
	    
            switch(status)
            {
		case DispatchStatus._DispatchOK:
		case DispatchStatus._DispatchUserException:
		{
		    __is.startReadEncaps();
		    break;
		}
		
		case DispatchStatus._DispatchObjectNotExist:
		{
		    Ice.ObjectNotExistException ex = new Ice.ObjectNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(__is);
		    ex.facet = __is.readStringSeq();
		    ex.operation = __is.readString();
		    throw ex;
		}
		
		case DispatchStatus._DispatchFacetNotExist:
		{
		    Ice.FacetNotExistException ex = new Ice.FacetNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(__is);
		    ex.facet = __is.readStringSeq();
		    ex.operation = __is.readString();
		    throw ex;
		}
		
		case DispatchStatus._DispatchOperationNotExist:
		{
		    Ice.OperationNotExistException ex = new Ice.OperationNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(__is);
		    ex.facet = __is.readStringSeq();
		    ex.operation = __is.readString();
		    throw ex;
		}
		
		case DispatchStatus._DispatchUnknownException:
		{
		    Ice.UnknownException ex = new Ice.UnknownException();
		    ex.unknown = __is.readString();
		    throw ex;
		}
		
		case DispatchStatus._DispatchUnknownLocalException:
		{
		    Ice.UnknownLocalException ex = new Ice.UnknownLocalException();
		    ex.unknown = __is.readString();
		    throw ex;
		}
		
		case DispatchStatus._DispatchUnknownUserException:
		{
		    Ice.UnknownUserException ex = new Ice.UnknownUserException();
		    ex.unknown = __is.readString();
		    throw ex;
		}
		
		default:
		{
		    throw new Ice.UnknownReplyStatusException();
		}
	    }
	}
	catch(Ice.LocalException ex)
	{
	    __finished(ex);
	    return;
	}

	assert(status == DispatchStatus._DispatchOK || status == DispatchStatus._DispatchUserException);

	try
	{
	    __response(status == DispatchStatus._DispatchOK);
	}
	catch(Exception ex)
	{
	    warning(ex);
	}
    }

    public final void
    __finished(Ice.LocalException exc)
    {
	//
	// No mutex protection necessary, this function can only be called
	// after __send() and __prepare() have completed.
	//
	
	assert(_reference != null);
	//assert(_connection != null); // Might be null, if getConnection() failed.
	
	if(_reference.locatorInfo != null)
	{
	    _reference.locatorInfo.clearObjectCache(_reference);
	}
	
	boolean doRetry = false;
	
	//
	// A CloseConnectionException indicates graceful server
	// shutdown, and is therefore always repeatable without
	// violating "at-most-once". That's because by sending a close
	// connection message, the server guarantees that all
	// outstanding requests can safely be repeated. Otherwise, we
	// can also retry if the operation mode Nonmutating or
	// Idempotent.
	//
	if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent ||
	   exc instanceof Ice.CloseConnectionException)
	{
	    try
	    {
		ProxyFactory proxyFactory = _reference.instance.proxyFactory();
		if(proxyFactory != null)
		{
		    _cnt = proxyFactory.checkRetryAfterException(exc, _cnt);
		}
		else
		{
		    throw exc; // The communicator is already destroyed, so we cannot retry.
		}
		
		doRetry = true;
	    }
	    catch(Ice.LocalException ex)
	    {
	    }
	}
	
	if(doRetry)
	{
	    _connection = null;
	    __send();
	}
	else
	{
	    try
	    {
		ice_exception(exc);
	    }
	    catch(Exception ex)
	    {
		warning(ex);
	    }
	}
    }

    public final boolean
    __timedOut()
    {
	//
	// No mutex protection necessary, this function can only be called
	// after __send() and __prepare() have completed.
	//
	
	assert(_connection != null);
	
	if(_connection.timeout() >= 0)
	{
	    return System.currentTimeMillis() >= _absoluteTimeoutMillis;
	}
	else
	{
	    return false;
	}
    }

    protected final void
    __prepare(Reference ref, String operation, Ice.OperationMode mode, java.util.Map context)
    {
	//
	// No mutex protection necessary, using this object for a new
	// AMI call while another one is in progress is not allowed
	// and leads to undefined behavior.
	//
	
	_reference = ref;
	_connection = _reference.getConnection();
	_cnt = 0;
	_mode = mode;
	
	if(__os != null)
	{
	    __os.destroy();
	}
	__os = new BasicStream(_reference.instance);
	
	_connection.prepareRequest(__os);

	ref.identity.__write(__os);
	__os.writeStringSeq(ref.facet);
	__os.writeString(operation);
	__os.writeByte((byte)mode.value());
	if(context == null)
	{
	    __os.writeSize(0);
	}
	else
	{
	    final int sz = context.size();
	    __os.writeSize(sz);
	    if(sz > 0)
	    {
		java.util.Iterator i = context.entrySet().iterator();
		while(i.hasNext())
		{
		    java.util.Map.Entry entry = (java.util.Map.Entry)i.next();
		    __os.writeString((String)entry.getKey());
		    __os.writeString((String)entry.getValue());
		}
	    }
	}

	__os.startWriteEncaps();
    }

    protected final void
    __send()
    {
	assert(_reference != null);
	//assert(_connection != null); // Might be null, if called from __finished() for retry.
	
	try
	{
	    while(true)
	    {
		if(_connection == null)
		{
		    _connection = _reference.getConnection();
		}
		
		if(_connection.timeout() >= 0)
		{
		    _absoluteTimeoutMillis = System.currentTimeMillis() + _connection.timeout();
		}
		
		try
		{
		    _connection.sendAsyncRequest(__os, this);
		    
		    //
		    // Don't do anything after sendAsyncRequest() returned
		    // without an exception.  I such case, there will be
		    // callbacks, i.e., calls to the __finished()
		    // functions. Since there is no mutex protection, we
		    // cannot modify state here and in such callbacks.
		    //
		    return;
		}
		catch(Ice.LocalException ex)
		{
		    if(_reference.locatorInfo != null)
		    {
			_reference.locatorInfo.clearObjectCache(_reference);
		    }
		    
		    ProxyFactory proxyFactory = _reference.instance.proxyFactory();
		    if(proxyFactory != null)
		    {
			_cnt = proxyFactory.checkRetryAfterException(ex, _cnt);
		    }
		    else
		    {
			throw ex; // The communicator is already destroyed, so we cannot retry.
		    }
		}
		
		_connection = null;
	    }
	}
	catch(Ice.LocalException ex)
	{
	    __finished(ex);
	}
    }

    protected abstract void __response(boolean ok);

    private final void
    warning(Exception ex)
    {
	if(__os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    java.io.StringWriter sw = new java.io.StringWriter();
	    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	    IceUtil.OutputBase out = new IceUtil.OutputBase(pw);
	    out.setUseTab(false);
	    out.print("exception raised by AMI callback:\n");
	    ex.printStackTrace(pw);
	    pw.flush();
	    __os.instance().logger().warning(sw.toString());
	}
    }

    protected BasicStream __is;
    protected BasicStream __os;

    private Reference _reference;
    private Connection _connection;
    private int _cnt;
    private Ice.OperationMode _mode;

    private long _absoluteTimeoutMillis;
}
