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

namespace IceInternal
{

using System.Collections;

public abstract class OutgoingAsync
{
    public
    OutgoingAsync()
    {
	_is = null;
	_os = null;
    }
    
    public abstract void ice_exception(Ice.LocalException ex);
    
    public virtual void
    __setup(Connection connection, Reference r, string operation, Ice.OperationMode mode, Ice.Context context)
    {
	try
	{
	    _connection = connection;
	    if(_is == null)
	    {
		_is = new BasicStream(r.instance);
	    }
	    else
	    {
		_is.reset();
	    }
	    if(_os == null)
	    {
		_os = new BasicStream(r.instance);
	    }
	    else
	    {
		_os.reset();
	    }
	    
	    _connection.prepareRequest(_os);
	    
	    r.identity.__write(_os);
	    _os.writeStringSeq(r.facet);
	    _os.writeString(operation);
	    _os.writeByte((byte)mode);
	    if(context == null)
	    {
		_os.writeSize(0);
	    }
	    else
	    {
		int sz = context.Count;
		_os.writeSize(sz);
		if(sz > 0)
		{
		    foreach(DictionaryEntry entry in context)
		    {
			_os.writeString((string)entry.Key);
			_os.writeString((string)entry.Value);
		    }
		}
	    }
	    
	    //
	    // Input and output parameters are always sent in an
	    // encapsulation, which makes it possible to forward
	    // requests as blobs.
	    //
	    _os.startWriteEncaps();
	}
	catch(System.Exception ex)
	{
	    destroy();
	    throw ex;
	}
    }
    
    public virtual void
    __invoke()
    {
	try
	{
	    _os.endWriteEncaps();
	    
	    _connection.sendAsyncRequest(this);
	    
	    if(_connection.timeout() >= 0)
	    {
		_absoluteTimeoutMillis = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
		                         + _connection.timeout();
	    }
	}
	catch(System.Exception ex)
	{
	    destroy();
	    throw ex;
	}
    }
    
    public virtual void
    __finished(BasicStream istr)
    {
	try
	{
	    _is.swap(istr);
	    byte status = _is.readByte();
	    
	    switch((DispatchStatus)status)
	    {
		case DispatchStatus.DispatchOK: 
		{
		    //
		    // Input and output parameters are always sent in an
		    // encapsulation, which makes it possible to forward
		    // oneway requests as blobs.
		    //
		    _is.startReadEncaps();
		    __response(true);
		    break;
		}
		
		case DispatchStatus.DispatchUserException: 
		{
		    //
		    // Input and output parameters are always sent in an
		    // encapsulation, which makes it possible to forward
		    // oneway requests as blobs.
		    //
		    _is.startReadEncaps();
		    __response(false);
		    break;
		}
		
		case DispatchStatus.DispatchObjectNotExist: 
		{
		    Ice.ObjectNotExistException ex = new Ice.ObjectNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(_is);
		    ex.facet = _is.readFacetPath();
		    ex.operation = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus.DispatchFacetNotExist: 
		{
		    Ice.FacetNotExistException ex = new Ice.FacetNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(_is);
		    ex.facet = _is.readFacetPath();
		    ex.operation = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus.DispatchOperationNotExist: 
		{
		    Ice.OperationNotExistException ex = new Ice.OperationNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(_is);
		    ex.facet = _is.readFacetPath();
		    ex.operation = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus.DispatchUnknownException: 
		{
		    Ice.UnknownException ex = new Ice.UnknownException();
		    ex.unknown = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus.DispatchUnknownLocalException: 
		{
		    Ice.UnknownLocalException ex = new Ice.UnknownLocalException();
		    ex.unknown = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus.DispatchUnknownUserException: 
		{
		    Ice.UnknownUserException ex = new Ice.UnknownUserException();
		    ex.unknown = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		default: 
		{
		    ice_exception(new Ice.UnknownReplyStatusException());
		    break;
		}
	    }
	}
	catch(System.Exception ex)
	{
	    warning(ex);
	}
	finally
	{
	    destroy();
	}
    }
    
    public virtual void
    __finished(Ice.LocalException exc)
    {
	try
	{
	    ice_exception(exc);
	}
	catch(System.Exception ex)
	{
	    warning(ex);
	}
	finally
	{
	    destroy();
	}
    }
    
    public virtual bool
    __timedOut()
    {
	if(_connection.timeout() >= 0)
	{
	    return (System.DateTime.Now.Ticks - 621355968000000000) / 10000 >= _absoluteTimeoutMillis;
	}
	else
	{
	    return false;
	}
    }
    
    public virtual BasicStream
    __is()
    {
	return _is;
    }
    
    public virtual BasicStream
    __os()
    {
	return _os;
    }
    
    protected internal abstract void __response(bool ok);
    
    private void
    destroy()
    {
	if(_is != null)
	{
	    _is.destroy();
	    _is = null;
	}
	if(_os != null)
	{
	    _os.destroy();
	    _os = null;
	}
    }
    
    private void
    warning(System.Exception ex)
    {
	if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    IceUtil.OutputBase output = new IceUtil.OutputBase("");
	    output.setUseTab(false);
	    output.print("exception raised by AMI callback:\n");
	    output.print(ex.ToString());
	    _os.instance().logger().warning(output.ToString()); // TODO: need to override ToString()?
	}
    }
    
    private Connection _connection;
    private long _absoluteTimeoutMillis;
    private BasicStream _is;
    private BasicStream _os;
}

}
