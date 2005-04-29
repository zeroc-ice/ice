// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class IncomingAsync extends IncomingBase
{
    public
    IncomingAsync(Incoming in) // Adopts the argument. It must not be used afterwards.
    {
	super(in);
    }

    final protected void
    __response(boolean ok)
    {
	try
	{
	    if(_locator != null && _servant != null)
	    {
		_locator.finished(_current, _servant, _cookie.value);
	    }

	    if(_response)
	    {
		_os.endWriteEncaps();
		
		int save = _os.pos();
		_os.pos(Protocol.headerSize + 4); // Dispatch status position.
		
		if(ok)
		{
		    _os.writeByte((byte)DispatchStatus._DispatchOK);
		}
		else
		{
		    _os.writeByte((byte)DispatchStatus._DispatchUserException);
		}
		
		_os.pos(save);

		_connection.sendResponse(_os, _compress);
	    }
	    else
	    {
		_connection.sendNoResponse();
	    }
	}
	catch(Ice.LocalException ex)
	{
	    _connection.exception(ex);
	}
	catch(java.lang.Exception ex)
	{
	    Ice.UnknownException uex = new Ice.UnknownException();
	    //uex.unknown = ex.toString();
	    java.io.StringWriter sw = new java.io.StringWriter();
	    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	    ex.printStackTrace(pw);
	    pw.flush();
	    uex.unknown = sw.toString();
	    _connection.exception(uex);
	}
    }

    final protected void
    __exception(java.lang.Exception exc)
    {
	try
	{
	    if(_locator != null && _servant != null)
	    {
		_locator.finished(_current, _servant, _cookie.value);
	    }

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
			String[] facetPath = { ex.facet };
			_os.writeStringSeq(facetPath);
		    }

		    _os.writeString(ex.operation);

		    _connection.sendResponse(_os, _compress);
		}
		else
		{
		    _connection.sendNoResponse();
		}
	    }
	    catch(Ice.UnknownLocalException ex)
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
		    _os.writeString(ex.unknown);
		    _connection.sendResponse(_os, _compress);
		}
		else
		{
		    _connection.sendNoResponse();
		}
	    }
	    catch(Ice.UnknownUserException ex)
	    {
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
	    }
	    catch(Ice.UnknownException ex)
	    {
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
		    _os.writeByte((byte)DispatchStatus._DispatchUnknownUserException);
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
	    }
	    catch(java.lang.Exception ex)
	    {
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
	    }
	}
	catch(Ice.LocalException ex)
	{
	    _connection.exception(ex);
	}
	catch(java.lang.Exception ex)
	{
	    Ice.UnknownException uex = new Ice.UnknownException();
	    //uex.unknown = ex.toString();
	    java.io.StringWriter sw = new java.io.StringWriter();
	    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	    ex.printStackTrace(pw);
	    pw.flush();
	    uex.unknown = sw.toString();
	    _connection.exception(uex);
	}
    }

    final protected BasicStream
    __os()
    {
	return _os;
    }
}
