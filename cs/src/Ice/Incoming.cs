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
using System.Diagnostics;

sealed public class Incoming : IncomingBase
{
    public
    Incoming(Instance instance, Connection connection, Ice.ObjectAdapter adapter,
             bool response, byte compress)
	 : base(instance, connection, adapter, response, compress)
    {
	_is = new BasicStream(instance);
    }
    
    //
    // This function allows this object to be reused, rather than
    // reallocated.
    //
    public override void
    reset(Instance instance, Connection connection, Ice.ObjectAdapter adapter, bool response, byte compress)
    {
	base.reset(instance, connection, adapter, response, compress);
	
	if(_is == null)
	{
	    _is = new BasicStream(instance);
	}
	else
	{
	    _is.reset();
	}
    }
    
    public void
    invoke(ServantManager servantManager)
    {
	//
	// Read the current.
	//
	_current.id.__read(_is);
	_current.facet = _is.readFacetPath();
	_current.operation = _is.readString();
	_current.mode = (Ice.OperationMode)(int)_is.readByte();
	int sz = _is.readSize();
	while(sz-- > 0)
	{
	    string first = _is.readString();
	    string second = _is.readString();
	    if(_current.ctx == null)
	    {
		_current.ctx = new Ice.Context();
	    }
	    _current.ctx[first] = second;
	}
	
	_is.startReadEncaps();
	
	if(_response)
	{
	    Debug.Assert(_os.size() == Protocol.headerSize + 4); // Dispatch status position.
	    _os.writeByte((byte)0);
	    _os.startWriteEncaps();
	}
	
	DispatchStatus status;
	
	//
	// Don't put the code above into the try block below. Exceptions
	// in the code above are considered fatal, and must propagate to
	// the caller of this operation.
	//
	
	try
	{
	    if(servantManager != null)
	    {
		_servant = servantManager.findServant(_current.id);
		
		if(_servant == null && _current.id.category.Length > 0)
		{
		    _locator = servantManager.findServantLocator(_current.id.category);
		    if(_locator != null)
		    {
			_servant = _locator.locate(_current, out _cookie);
		    }
		}
		
		if(_servant == null)
		{
		    _locator = servantManager.findServantLocator("");
		    if(_locator != null)
		    {
			_servant = _locator.locate(_current, out _cookie);
		    }
		}
	    }
	    
	    if(_servant == null)
	    {
		status = DispatchStatus.DispatchObjectNotExist;
	    }
	    else
	    {
		if(_current.facet.Count > 0)
		{
		    Ice.Object facetServant = _servant.ice_findFacetPath(_current.facet, 0);
		    if(facetServant == null)
		    {
			status = DispatchStatus.DispatchFacetNotExist;
		    }
		    else
		    {
			status = facetServant.__dispatch(this, _current);
		    }
		}
		else
		{
		    status = _servant.__dispatch(this, _current);
		}
	    }
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
		    _os.writeByte((byte)DispatchStatus._DispatchObjectNotExist);
		}
		else if(ex is Ice.FacetNotExistException)
		{
		    _os.writeByte((byte)DispatchStatus._DispatchFacetNotExist);
		}
		else if(ex is Ice.OperationNotExistException)
		{
		    _os.writeByte((byte)DispatchStatus._DispatchOperationNotExist);
		}
		else
		{
		    Debug.Assert(false);
		}
		ex.id.__write(_os);
		_os.writeStringSeq(ex.facet);
		_os.writeString(ex.operation);
	    }
	    
	    __finishInvoke();
	    _is.endReadEncaps();
	    return;
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
		_os.writeByte((byte)DispatchStatus._DispatchUnknownLocalException);
		_os.writeString(ex.ToString());
	    }
	    
	    __finishInvoke();
	    _is.endReadEncaps();
	    return;
	}

	/* Not possible in Java - UserExceptions are checked exceptions // TODO
	catch(Ice.UserException ex)
	{
	// ...
	}
	*/

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
		_os.writeByte((byte) DispatchStatus._DispatchUnknownException);
		_os.writeString(ex.ToString());
	    }
	    
	    __finishInvoke();
	    _is.endReadEncaps();
	    return;
	}
	
	//
	// Don't put the code below into the try block above. Exceptions
	// in the code below are considered fatal, and must propagate to
	// the caller of this operation.
	//
	
	//
	// DispatchAsync is "pseudo dispatch status", used internally
	// only to indicate async dispatch.
	//
	if(status == DispatchStatus.DispatchAsync)
	{
	    //
	    // If this was an asynchronous dispatch, we're done
	    // here.  We do *not* call __finishInvoke(), because
	    // the call is not finished yet.
	    //
	    _is.endReadEncaps();
	    return;
	}
	
	if(_response)
	{
	    _os.endWriteEncaps();
	    
	    if(status != DispatchStatus.DispatchOK && status != DispatchStatus.DispatchUserException)
	    {
		Debug.Assert(status == DispatchStatus.DispatchObjectNotExist ||
		             status == DispatchStatus.DispatchFacetNotExist ||
			     status == DispatchStatus.DispatchOperationNotExist);
		
		_os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		_os.writeByte((byte)status.val());
		
		_current.id.__write(_os);
		_os.writeStringSeq(_current.facet);
		_os.writeString(_current.operation);
	    }
	    else
	    {
		int save = _os.pos();
		_os.pos(Protocol.headerSize + 4); // Dispatch status position.
		_os.writeByte((byte)status.val());
		_os.pos(save);
	    }
	}
	
	__finishInvoke();
	_is.endReadEncaps();
    }
    
    public BasicStream
    istr()
    {
	return _is;
    }
    
    public BasicStream
    ostr()
    {
	return _os;
    }
    
    //
    // Reclaim resources.
    //
    public override void
    __destroy()
    {
	base.__destroy();
	
	if(_is != null)
	{
	    _is.destroy();
	    _is = null;
	}
    }
    
    internal Incoming next; // For use by Connection.
    
    private BasicStream _is;
}

}
