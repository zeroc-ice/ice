// **********************************************************************
//
// Copyright (c) 2002
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

final public class Incoming extends IncomingBase
{
    public
    Incoming(Instance instance, Connection connection, Ice.ObjectAdapter adapter, boolean response)
    {
	super(instance, connection, adapter, response);
    }

    //
    // This function allows this object to be reused, rather than
    // reallocated.
    //
    public void
    reset(Instance instance, Connection connection, Ice.ObjectAdapter adapter, boolean response)
    {
        _current.adapter = adapter;
        if(_current.ctx != null)
        {
            _current.ctx.clear();
        }
	_servant = null;
	_locator = null;
        _cookie.value = null;
	_connection = connection;
	_response = response;
	if(_is == null)
	{
	    _is = new BasicStream(instance);
	}
	else
	{
	    _is.reset();
	}
	if(_os == null)
	{
	    _os = new BasicStream(instance);
	}
	else
	{
	    _os.reset();
	}
    }

    public void
    invoke()
    {
	//
	// Read the current.
	//
        _current.id.__read(_is);
        _current.facet = _is.readStringSeq();
        _current.operation = _is.readString();
        _current.mode = Ice.OperationMode.convert(_is.readByte());
        int sz = _is.readSize();
        while(sz-- > 0)
        {
            String first = _is.readString();
            String second = _is.readString();
            if(_current.ctx == null)
            {
                _current.ctx = new java.util.HashMap();
            }
            _current.ctx.put(first, second);
        }

        _is.startReadEncaps();

        if(_response)
        {
            assert(_os.size() == Protocol.headerSize + 4); // Dispatch status position.
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
	    if(_current.adapter != null)
	    {
		_servant = _current.adapter.identityToServant(_current.id);
		
		if(_servant == null && _current.id.category.length() > 0)
		{
		    _locator = _current.adapter.findServantLocator(_current.id.category);
		    if(_locator != null)
		    {
			_servant = _locator.locate(_current, _cookie);
		    }
		}
		
		if(_servant == null)
		{
		    _locator = _current.adapter.findServantLocator("");
		    if(_locator != null)
		    {
			_servant = _locator.locate(_current, _cookie);
		    }
		}
	    }

            if(_servant == null)
            {
                status = DispatchStatus.DispatchObjectNotExist;
            }
            else
            {
                if(_current.facet.length > 0)
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

		//
		// In case of an asynchronous dispatch, _is is now
		// null, because an IncomingAsync has adopted this
		// Incoming.
		//
		if(_is == null)
		{
		    //
		    // If this was an asynchronous dispatch, we're
		    // done here.  We do *not* call __finishInvoke(),
		    // because the call is not finished yet.
		    //
		    assert(status == DispatchStatus.DispatchOK);
		    return;
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
	    
	    if(ex.operation == null || ex.operation.length() == 0)
	    {
		ex.operation = _current.operation;
	    }

	    __warning(ex);

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		if(ex instanceof Ice.ObjectNotExistException)
		{
		    _os.writeByte((byte)DispatchStatus._DispatchObjectNotExist);
		}
		else if(ex instanceof Ice.FacetNotExistException)
		{
		    _os.writeByte((byte)DispatchStatus._DispatchFacetNotExist);
		}
		else if(ex instanceof Ice.OperationNotExistException)
		{
		    _os.writeByte((byte)DispatchStatus._DispatchOperationNotExist);
		}
		else
		{
		    assert(false);
		}
		ex.id.__write(_os);
		_os.writeStringSeq(ex.facet);
		_os.writeString(ex.operation);
            }

	    __finishInvoke();
	    return;
        }
        catch(Ice.LocalException ex)
        {
	    __warning(ex);

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownLocalException);
		_os.writeString(ex.toString());
            }

	    __finishInvoke();
	    return;
        }
        /* Not possible in Java - UserExceptions are checked exceptions
        catch(Ice.UserException ex)
        {
	// ...
	}
	*/
        catch(RuntimeException ex)
        {
	    __warning(ex);

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
		_os.writeString(ex.toString());
            }
	    
	    __finishInvoke();
	    return;
        }
	
	//
	// Don't put the code below into the try block above. Exceptions
	// in the code below are considered fatal, and must propagate to
	// the caller of this operation.
	//

	if(_response)
	{
	    _os.endWriteEncaps();
	    
	    if(status != DispatchStatus.DispatchOK && status != DispatchStatus.DispatchUserException)
	    {
		assert(status == DispatchStatus.DispatchObjectNotExist ||
		       status == DispatchStatus.DispatchFacetNotExist ||
		       status == DispatchStatus.DispatchOperationNotExist);
		
		_os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		_os.writeByte((byte)status.value());
		
		_current.id.__write(_os);
		_os.writeStringSeq(_current.facet);
		_os.writeString(_current.operation);
	    }
	    else
	    {
		int save = _os.pos();
		_os.pos(Protocol.headerSize + 4); // Dispatch status position.
		_os.writeByte((byte)status.value());
		_os.pos(save);
	    }
	}

	__finishInvoke();
    }

    public BasicStream
    is()
    {
        return _is;
    }

    public BasicStream
    os()
    {
        return _os;
    }

    Incoming next; // For use by Connection.
}
