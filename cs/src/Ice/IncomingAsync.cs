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

    using System.Diagnostics;

    public class IncomingAsync : IncomingBase
    {
	public IncomingAsync(Incoming inc)
	    : base(inc)
	{
	    _finished = false;
	}
	
	~IncomingAsync()
	{
	    //
	    // I must call __destroy() in the finalizer and not in
	    // __response() or __exception(), because an exception may be
	    // raised after the creation of an IncomingAsync but before
	    // calling __response() or __exception(). This can happen if
	    // an AMD operation raises an exception instead of calling
	    // ice_response() or ice_exception().
	    //
	    __destroy();
	}
	
	protected internal void __response(bool ok)
	{
	    Debug.Assert(!_finished);
	    _finished = true;
	    
	    if(_response)
	    {
		_os.endWriteEncaps();
		
		int save = _os.pos();
		_os.pos(Protocol.headerSize + 4); // Dispatch status position.
		
		if(ok)
		{
		    _os.writeByte((byte)DispatchStatus.DispatchOK);
		}
		else
		{
		    _os.writeByte((byte)DispatchStatus.DispatchUserException);
		}
		
		_os.pos(save);
	    }
	    
	    __finishInvoke();
	}
	
	protected internal void __exception(System.Exception exc)
	{
	    Debug.Assert(!_finished);
	    _finished = true;
	    
	    try
	    {
		throw exc;
	    }
	    catch(Ice.RequestFailedException ex)
	    {
		if(ex.id == null)
		{
		    ex.id = _current.id;
		}
		
		if(ex.facet == null)
		{
		    ex.facet = _current.facet;
		}
		
		if(ex.operation == null || ex.operation.Length == 0)
		{
		    ex.operation = _current.operation;
		}
		
		if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
		{
		    __warning(ex);
		}
		
		if(_response)
		{
		    _os.endWriteEncaps();
		    _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		    if(ex is Ice.ObjectNotExistException)
		    {
			_os.writeByte((byte)DispatchStatus.DispatchObjectNotExist);
		    }
		    else if(ex is Ice.FacetNotExistException)
		    {
			_os.writeByte((byte)DispatchStatus.DispatchFacetNotExist);
		    }
		    else if(ex is Ice.OperationNotExistException)
		    {
			_os.writeByte((byte)DispatchStatus.DispatchOperationNotExist);
		    }
		    else
		    {
			Debug.Assert(false);
		    }
		    ex.id.__write(_os);
		    _os.writeStringSeq(ex.facet);
		    _os.writeString(ex.operation);
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		{
		    __warning(ex);
		}
		
		if(_response)
		{
		    _os.endWriteEncaps();
		    _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		    _os.writeByte((byte)DispatchStatus.DispatchUnknownLocalException);
		    _os.writeString(ex.ToString());
		}
	    }
	    catch(Ice.UserException ex)
	    {
		if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		{
		    __warning(ex);
		}
		
		if(_response)
		{
		    _os.endWriteEncaps();
		    _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		    _os.writeByte((byte)DispatchStatus.DispatchUnknownUserException);
		    _os.writeString(ex.ToString());
		}
	    }
	    catch(System.Exception ex)
	    {
		if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		{
		    __warning(ex);
		}
		
		if(_response)
		{
		    _os.endWriteEncaps();
		    _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		    _os.writeByte((byte)DispatchStatus.DispatchUnknownException);
		    _os.writeString(ex.ToString());
		}
	    }
	    
	    __finishInvoke();
	}
	
	protected internal BasicStream __os()
	{
	    return _os;
	}
	
	protected internal bool _finished;
    }
	
}
