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

public class IncomingAsync extends IncomingBase
{
    public
    IncomingAsync(Incoming in) // Adopts the argument. It must not be used afterwards.
    {
	super(in);
    }

    protected void
    finalize()
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

    final protected void
    __response(boolean ok)
    {
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
	}
	
	__finishInvoke();
    }

    final protected void
    __exception(Exception exc)
    {
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
        }
        catch(Ice.UserException ex)
        {
	    __warning(ex);

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownUserException);
		_os.writeString(ex.toString());
            }
        }
	catch(Exception ex)
	{
	    __warning(ex);

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
		_os.writeString(ex.toString());
            }
	}

	__finishInvoke();
    }

    final protected BasicStream
    __is()
    {
	return _is;
    }

    final protected BasicStream
    __os()
    {
	return _os;
    }
};
