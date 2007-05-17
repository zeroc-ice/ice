// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final public class Incoming
{
    public
    Incoming(Instance instance, Ice.Connection connection, BasicStream is, Ice.ObjectAdapter adapter)
    {
        _os = new BasicStream(instance);
	_is = is;
	_connection = connection;

	setAdapter(adapter);
    }
    
    public void
    setAdapter(Ice.ObjectAdapter adapter)
    {
	_adapter = adapter;
	if(_adapter != null)
	{
	    _servantManager = _adapter.getServantManager();
	    if(_servantManager == null)
	    {
	        _adapter = null;
	    }
	}
	else
	{
	    _servantManager = null;
	}
    }

    public Ice.ObjectAdapter
    getAdapter()
    {
	return _adapter;
    }
    
    public void
    invoke(boolean response, int requestId)
    {
        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.con = _connection;
	_current.adapter = _adapter;
	_current.requestId = requestId;

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
	if(sz > 0)
	{
	    _current.ctx = new java.util.Hashtable();
	}
        while(sz-- > 0)
        {
            String first = _is.readString();
            String second = _is.readString();
            _current.ctx.put(first, second);
        }

        _is.startReadEncaps();

        if(response)
        {
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_os.size() == Protocol.headerSize + 4); // Reply status position.
	    }
            _os.writeByte(ReplyStatus.replyOK);
            _os.startWriteEncaps();
        }

	byte replyStatus = ReplyStatus.replyOK;
	Ice.DispatchStatus dispatchStatus = Ice.DispatchStatus.DispatchOK;
	
	//
	// Don't put the code above into the try block below. Exceptions
	// in the code above are considered fatal, and must propagate to
	// the caller of this operation.
	//

        try
        {
	    Ice.Object servant = null;
	    if(_servantManager != null)
	    {
	        servant = _servantManager.findServant(_current.id, _current.facet);
	    }
	    
	    if(servant == null)
	    {
	        if(_servantManager != null && _servantManager.hasServant(_current.id))
	        {
		    replyStatus = ReplyStatus.replyFacetNotExist;
	        }
	        else
	        {
		    replyStatus = ReplyStatus.replyObjectNotExist;
	        }
	    }
	    else
	    {
	        dispatchStatus = servant.__dispatch(this, _current);
                if(dispatchStatus == Ice.DispatchStatus.DispatchUserException)
                {
                    replyStatus = ReplyStatus.replyUserException;
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

	    if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
	    {
		__warning(ex);
	    }

            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
		if(ex instanceof Ice.ObjectNotExistException)
		{
		    _os.writeByte(ReplyStatus.replyObjectNotExist);
		}
		else if(ex instanceof Ice.FacetNotExistException)
		{
		    _os.writeByte(ReplyStatus.replyFacetNotExist);
		}
		else if(ex instanceof Ice.OperationNotExistException)
		{
		    _os.writeByte(ReplyStatus.replyOperationNotExist);
		}
		else
		{
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(false);
		    }
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

		_connection.sendResponse(_os);
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

	    if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownLocalException);
		_os.writeString(ex.unknown);
		_connection.sendResponse(_os);
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

	    if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownUserException);
		_os.writeString(ex.unknown);
		_connection.sendResponse(_os);
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

	    if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownException);
		_os.writeString(ex.unknown);
		_connection.sendResponse(_os);
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

	    if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownLocalException);
		//_os.writeString(ex.toString());
		java.io.ByteArrayOutputStream sw = new java.io.ByteArrayOutputStream();
		java.io.PrintStream pw = new java.io.PrintStream(sw);
		pw.println(ex.toString());
		pw.flush();
		_os.writeString(sw.toString());
		_connection.sendResponse(_os);
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

	    if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownException);
		//_os.writeString(ex.toString());
		java.io.ByteArrayOutputStream sw = new java.io.ByteArrayOutputStream();
		java.io.PrintStream pw = new java.io.PrintStream(sw);
		pw.println(ex.toString());
		pw.flush();
		_os.writeString(sw.toString());
		_connection.sendResponse(_os);
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

	if(response)
	{
	    _os.endWriteEncaps();
	    
            if(replyStatus != ReplyStatus.replyOK && replyStatus != ReplyStatus.replyUserException)
            {
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(replyStatus == ReplyStatus.replyObjectNotExist ||
                                         replyStatus == ReplyStatus.replyFacetNotExist);
		}
		
		_os.resize(Protocol.headerSize + 4, false); // Reply status position.
		_os.writeByte(replyStatus);
		
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
		_os.pos(Protocol.headerSize + 4); // Reply status position.
		_os.writeByte(replyStatus);
		_os.pos(save);
	    }

	    _connection.sendResponse(_os);
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

    final private void
    __warning(java.lang.Exception ex)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_os != null);
	}

	StringBuffer sb = new StringBuffer();
	sb.append("dispatch exception:");
	sb.append("\nidentity: " + _os.instance().identityToString(_current.id));
	sb.append("\nfacet: " + IceUtil.StringUtil.escapeString(_current.facet, ""));
	sb.append("\noperation: " + _current.operation);
	sb.append("\n");
	sb.append(ex.toString());
	_os.instance().initializationData().logger.warning(sb.toString());
    }

    //
    // The following three members were 'final' but javac 1.1.8 complains.
    //
    private BasicStream _os;
    private BasicStream _is;
    private Ice.Connection _connection;

    private Ice.ObjectAdapter _adapter;
    private ServantManager _servantManager;
    private Ice.Current _current;
}
