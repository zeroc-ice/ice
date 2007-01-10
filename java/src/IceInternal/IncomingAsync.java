// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
	    if(!__servantLocatorFinished())
	    {
		return;
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
	    _connection.invokeException(ex, 1);
	}
    }

    final protected void
    __exception(java.lang.Exception exc)
    {
	try
	{
	    if(!__servantLocatorFinished())
	    {
		return;
	    }

	    __handleException(exc);
	}
	catch(Ice.LocalException ex)
	{
	    _connection.invokeException(ex, 1);
	}
    }

    final private boolean
    __servantLocatorFinished()
    {
	try
	{
	    if(_locator != null && _servant != null)
	    {
		_locator.finished(_current, _servant, _cookie.value);
	    }
	    return true;
	}
        catch(java.lang.Exception ex)
        {
	    __handleException(ex);
	    return false;
        }
    }

    final protected BasicStream
    __os()
    {
	return _os;
    }
}
