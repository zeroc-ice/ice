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

public class IncomingBase
{
    protected
    IncomingBase(Instance instance, Connection connection, Ice.ObjectAdapter adapter, boolean response, byte compress)
    {
        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.adapter = adapter;

        _cookie = new Ice.LocalObjectHolder();

	_response = response;

        _compress = compress;

        _os = new BasicStream(instance);

	_connection = connection;
    }

    protected
    IncomingBase(IncomingBase in) // Adopts the argument. It must not be used afterwards.
    {
	_current = in._current;
	in._current = null;

	_servant = in._servant;
	in._servant = null;

	_locator = in._locator;
	in._locator = null;

	_cookie = in._cookie;
	in._cookie = null;

	_response = in._response;
	in._response = false;

	_compress = in._compress;
	in._compress = 0;

	_os = in._os;
	in._os = null;

	_connection = in._connection;
	in._connection = null;
    }

    //
    // This function allows this object to be reused, rather than
    // reallocated.
    //
    public void
    reset(Instance instance, Connection connection, Ice.ObjectAdapter adapter, boolean response, byte compress)
    {
	if(_current == null)
	{
	    _current = new Ice.Current();
	    _current.id = new Ice.Identity();
	    _current.adapter = adapter;
	}
	else
	{
	    assert(_current.id != null);
	    _current.adapter = adapter;

	    if(_current.ctx != null)
	    {
		_current.ctx.clear();
	    }
	}

	_servant = null;

	_locator = null;

	if(_cookie == null)
	{
	    _cookie = new Ice.LocalObjectHolder();
	}
	else
	{
	    _cookie.value = null;
	}

	_response = response;

        _compress = compress;

	if(_os == null)
	{
	    _os = new BasicStream(instance);
	}
	else
	{
	    _os.reset();
	}

	_connection = connection;
    }

    //
    // Reclaim resources.
    //
    public void
    __destroy()
    {
	if(_os != null)
	{
	    _os.destroy();
	    _os = null;
	}
    }

    final protected void
    __finishInvoke()
    {
	if(_locator != null && _servant != null)
	{
	    _locator.finished(_current, _servant, _cookie.value);
	}
	
	//
	// Send a response if necessary. If we don't need to send a
	// response, we still need to tell the connection that we're
	// finished with dispatching.
	//
	if(_response)
	{
	    _connection.sendResponse(_os, _compress);
	}
	else
	{
	    _connection.sendNoResponse();
	}
    }

    final protected void
    __warning(Exception ex)
    {
	assert(_os != null);

	java.io.StringWriter sw = new java.io.StringWriter();
	java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	IceUtil.OutputBase out = new IceUtil.OutputBase(pw);
	out.setUseTab(false);
	out.print("dispatch exception:");
	out.print("\nidentity: " + Ice.Util.identityToString(_current.id));
	out.print("\nfacet: ");
	IceInternal.ValueWriter.write(_current.facet, out);
	out.print("\noperation: " + _current.operation);
	out.print("\n");
	ex.printStackTrace(pw);
	pw.flush();
	_os.instance().logger().warning(sw.toString());
    }

    protected Ice.Current _current;
    protected Ice.Object _servant;
    protected Ice.ServantLocator _locator;
    protected Ice.LocalObjectHolder _cookie;

    protected boolean _response;
    protected byte _compress;

    protected BasicStream _os;

    private Connection _connection;
};
