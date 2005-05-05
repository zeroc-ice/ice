// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final public class Incoming extends IncomingBase
{
    public
    Incoming(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter, boolean response, byte compress)
    {
	super(instance, connection, adapter, response, compress);

        _is = new BasicStream(instance);
    }

    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void
    reset(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter, boolean response, byte compress)
    {
	if(_is == null)
	{
	    _is = new BasicStream(instance);
	}

	super.reset(instance, connection, adapter, response, compress);
    }

    public void
    reclaim()
    {
	if(_is != null)
	{
	    _is.reset();
	}

	super.reclaim();
    }

    public void
    invoke(ServantManager servantManager)
    {
	//
	// Read the current.
	//
        _current.id.__read(_is);

        //
        // For compatibility with the old FacetPath.
        //
        String[] facetPath = _is.readStringSeq();
        if(facetPath.length > 0)
        {
	    if(facetPath.length > 1)
	    {
	        throw new Ice.MarshalException();
	    }
            _current.facet = facetPath[0];
        }
	else
	{
            _current.facet = "";
	}

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

	// Initialize status to some value, to keep the compiler happy.
	DispatchStatus status = DispatchStatus.DispatchOK;
	
	//
	// Don't put the code above into the try block below. Exceptions
	// in the code above are considered fatal, and must propagate to
	// the caller of this operation.
	//

        try
        {
	    try
	    {
		if(servantManager != null)
		{
		    _servant = servantManager.findServant(_current.id, _current.facet);
		    
		    if(_servant == null && _current.id.category.length() > 0)
		    {
			_locator = servantManager.findServantLocator(_current.id.category);
			if(_locator != null)
			{
			    _servant = _locator.locate(_current, _cookie);
			}
		    }
		    
		    if(_servant == null)
		    {
			_locator = servantManager.findServantLocator("");
			if(_locator != null)
			{
			    _servant = _locator.locate(_current, _cookie);
			}
		    }
		}
		
		if(_servant == null)
		{
		    if(servantManager != null && servantManager.hasServant(_current.id))
		    {
			status = DispatchStatus.DispatchFacetNotExist;
		    }
		    else
		    {
			status = DispatchStatus.DispatchObjectNotExist;
		    }
		}
		else
		{
		    status = _servant.__dispatch(this, _current);
		}
	    }
	    finally
	    {
		if(_locator != null && _servant != null && status != DispatchStatus.DispatchAsync)
		{
		    _locator.finished(_current, _servant, _cookie.value);
		}
	    }
	}
	catch(Ice.RequestFailedException ex)
	{
	    _is.endReadEncaps();
	    
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

	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
	    {
		__warning(ex);
	    }

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

                //
                // For compatibility with the old FacetPath.
                //
                if(ex.facet == null || ex.facet.length() == 0)
                {
                    _os.writeStringSeq(null);
                }
                else
                {
                    String[] facetPath2 = { ex.facet };
                    _os.writeStringSeq(facetPath2);
                }

		_os.writeString(ex.operation);

		_connection.sendResponse(_os, _compress);
	    }
	    else
	    {
		_connection.sendNoResponse();
	    }

	    return;
        }
        catch(Ice.UnknownLocalException ex)
        {
	    _is.endReadEncaps();

	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownLocalException);
		_os.writeString(ex.unknown);
		_connection.sendResponse(_os, _compress);
	    }
	    else
	    {
		_connection.sendNoResponse();
	    }

	    return;
        }
        catch(Ice.UnknownUserException ex)
        {
	    _is.endReadEncaps();

	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownUserException);
		_os.writeString(ex.unknown);
		_connection.sendResponse(_os, _compress);
	    }
	    else
	    {
		_connection.sendNoResponse();
	    }

	    return;
        }
        catch(Ice.UnknownException ex)
        {
	    _is.endReadEncaps();

	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
		_os.writeString(ex.unknown);
		_connection.sendResponse(_os, _compress);
	    }
	    else
	    {
		_connection.sendNoResponse();
	    }

	    return;
        }
        catch(Ice.LocalException ex)
        {
	    _is.endReadEncaps();

	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownLocalException);
		//_os.writeString(ex.toString());
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		_os.writeString(sw.toString());
		_connection.sendResponse(_os, _compress);
	    }
	    else
	    {
		_connection.sendNoResponse();
	    }

	    return;
        }
        /* Not possible in Java - UserExceptions are checked exceptions
        catch(Ice.UserException ex)
        {
	// ...
	}
	*/
        catch(java.lang.Exception ex)
        {
	    _is.endReadEncaps();

	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
		//_os.writeString(ex.toString());
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		_os.writeString(sw.toString());
		_connection.sendResponse(_os, _compress);
	    }
	    else
	    {
		_connection.sendNoResponse();
	    }

	    return;
        }
	
	//
	// Don't put the code below into the try block above. Exceptions
	// in the code below are considered fatal, and must propagate to
	// the caller of this operation.
	//

	_is.endReadEncaps();

	//
	// DispatchAsync is "pseudo dispatch status", used internally
	// only to indicate async dispatch.
	//
	if(status == DispatchStatus.DispatchAsync)
	{
	    //
	    // If this was an asynchronous dispatch, we're done here.
	    //
	    return;
	}

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

                //
                // For compatibility with the old FacetPath.
                //
                if(_current.facet == null || _current.facet.length() == 0)
                {
                    _os.writeStringSeq(null);
                }
                else
                {
                    String[] facetPath2 = { _current.facet };
                    _os.writeStringSeq(facetPath2);
                }

		_os.writeString(_current.operation);
	    }
	    else
	    {
		int save = _os.pos();
		_os.pos(Protocol.headerSize + 4); // Dispatch status position.
		_os.writeByte((byte)status.value());
		_os.pos(save);
	    }

	    _connection.sendResponse(_os, _compress);
	}
	else
	{
	    _connection.sendNoResponse();
	}
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

    public Incoming next; // For use by ConnectionI.

    private BasicStream _is;
}
