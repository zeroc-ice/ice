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

package IceInternal;

public abstract class OutgoingAsync
{
    public
    OutgoingAsync()
    {
	_is = null;
	_os = null;
    }

    public abstract void ice_exception(Ice.LocalException ex);

    public void
    __setup(Connection connection, Reference ref, String operation, Ice.OperationMode mode, java.util.Map context)
    {
	try
	{
	    _connection = connection;
	    if(_is == null)
	    {
		_is = new BasicStream(ref.instance);
	    }
	    else
	    {
		_is.reset();
	    }
	    if(_os == null)
	    {
		_os = new BasicStream(ref.instance);
	    }
	    else
	    {
		_os.reset();
	    }

	    _connection.prepareRequest(_os);

	    ref.identity.__write(_os);
	    _os.writeStringSeq(ref.facet);
	    _os.writeString(operation);
	    _os.writeByte((byte)mode.value());
	    if(context == null)
	    {
		_os.writeSize(0);
	    }
	    else
	    {
		final int sz = context.size();
		_os.writeSize(sz);
		if(sz > 0)
		{
		    java.util.Iterator i = context.entrySet().iterator();
		    while(i.hasNext())
		    {
			java.util.Map.Entry entry = (java.util.Map.Entry)i.next();
			_os.writeString((String)entry.getKey());
			_os.writeString((String)entry.getValue());
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
        catch(RuntimeException ex)
	{
	    destroy();
	    throw ex;
	}
    }

    public void
    __invoke()
    {
	try
	{
	    _connection.sendAsyncRequest(this);
	    if(_connection.timeout() >= 0)
	    {
		_absoluteTimeoutMillis = System.currentTimeMillis() + _connection.timeout() * 1000;
	    }
	}
        catch(RuntimeException ex)
	{
	    destroy();
	    throw ex;
	}
    }

    public void
    __finished(BasicStream is)
    {
	try
	{
            _is.swap(is);
            byte status = _is.readByte();
	    
            switch((int)status)
            {
		case DispatchStatus._DispatchOK:
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
		
		case DispatchStatus._DispatchUserException:
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
		
		case DispatchStatus._DispatchObjectNotExist:
		{
		    Ice.ObjectNotExistException ex = new Ice.ObjectNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(_is);
		    ex.facet = _is.readStringSeq();
		    ex.operation = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus._DispatchFacetNotExist:
		{
		    Ice.FacetNotExistException ex = new Ice.FacetNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(_is);
		    ex.facet = _is.readStringSeq();
		    ex.operation = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus._DispatchOperationNotExist:
		{
		    Ice.OperationNotExistException ex = new Ice.OperationNotExistException();
		    ex.id = new Ice.Identity();
		    ex.id.__read(_is);
		    ex.facet = _is.readStringSeq();
		    ex.operation = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus._DispatchUnknownException:
		{
		    Ice.UnknownException ex = new Ice.UnknownException();
		    ex.unknown = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus._DispatchUnknownLocalException:
		{
		    Ice.UnknownLocalException ex = new Ice.UnknownLocalException();
		    ex.unknown = _is.readString();
		    ice_exception(ex);
		    break;
		}
		
		case DispatchStatus._DispatchUnknownUserException:
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
	catch(Exception ex)
	{
	    warning(ex);
	}
	finally
	{
	    destroy();
	}
    }

    public void
    __finished(Ice.LocalException exc)
    {
	try
	{
	    ice_exception(exc);
	}
	catch(Exception ex)
	{
	    warning(ex);
	}
	finally
	{
	    destroy();
	}
    }

    public boolean
    __timedOut()
    {
	if(_connection.timeout() >= 0)
	{
	    return System.currentTimeMillis() >= _absoluteTimeoutMillis;
	}
	else
	{
	    return false;
	}
    }

    public BasicStream
    __is()
    {
        return _is;
    }

    public BasicStream
    __os()
    {
        return _os;
    }

    protected abstract void __response(boolean ok);

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
    warning(Exception ex)
    {
	if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    java.io.StringWriter sw = new java.io.StringWriter();
	    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	    IceUtil.OutputBase out = new IceUtil.OutputBase(pw);
	    out.setUseTab(false);
	    out.print("exception raised by AMI callback:\n");
	    ex.printStackTrace(pw);
	    pw.flush();
	    _os.instance().logger().warning(sw.toString());
	}
    }

    private Connection _connection;
    private long _absoluteTimeoutMillis;
    private BasicStream _is;
    private BasicStream _os;
}
