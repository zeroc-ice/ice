// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
	_finished = false;
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
	assert(!_finished);
	_finished = true;
	
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
	
	//
	// Must be called last, so that if an exception is raised,
	// this function is definitely *not* called.
	//
	__finishInvoke();
    }

    final protected void
    __exception(Exception exc)
    {
	assert(!_finished);
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
//		_os.writeString(ex.toString());
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		_os.writeString(sw.toString());
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
//		_os.writeString(ex.toString());
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		_os.writeString(sw.toString());
            }
        }
	catch(Exception ex)
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
//		_os.writeString(ex.toString());
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		_os.writeString(sw.toString());
            }
	}

	//
	// Must be called last, so that if an exception is raised,
	// this function is definitely *not* called.
	//
	__finishInvoke();
    }

    final protected BasicStream
    __os()
    {
	return _os;
    }

    protected boolean _finished;
}
