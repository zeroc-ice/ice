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
    IncomingBase(Instance instance, Connection connection, Ice.ObjectAdapter adapter, boolean response)
    {
        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.adapter = adapter;
        _cookie = new Ice.LocalObjectHolder();
	_connection = connection;
	_response = response;
        _is = new BasicStream(instance);
        _os = new BasicStream(instance);
    }

    protected
    IncomingBase(IncomingBase in) // Adopts the argument. It must not be used afterwards.
    {
	_current = in._current;
	_servant = in._servant;
	_locator = in._locator;
	_cookie = in._cookie;
	_connection = in._connection;
	_response = in._response;
	_is = in._is;
	in._is = null;
	_os = in._os;
	in._os = null;
    }

    //
    // Reclaim resources.
    //
    final public void
    __destroy()
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

    final protected void
    __finishInvoke()
    {
	if(_locator != null && _servant != null)
	{
	    _locator.finished(_current, _servant, _cookie.value);
	}
	
	_is.endReadEncaps();
	
	//
	// Send a response if necessary. If we don't need to send a
	// response, we still need to tell the connection that we're
	// finished with dispatching.
	//
	if(_response)
	{
	    _connection.sendResponse(_os);
	}
	else
	{
	    _connection.sendNoResponse();
	}
    }

    final protected void
    __warning(Exception ex)
    {
	if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
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
    }

    protected Ice.Current _current;
    protected Ice.Object _servant;
    protected Ice.ServantLocator _locator;
    protected Ice.LocalObjectHolder _cookie;

    protected Connection _connection;

    protected boolean _response;

    protected BasicStream _is;
    protected BasicStream _os;
};
