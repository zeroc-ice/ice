// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    using System.Diagnostics;

    public class IncomingBase
    {
	protected internal IncomingBase(Instance instance, Connection connection,
	                                Ice.ObjectAdapter adapter, bool response, byte compress)
	{
	    _current = new Ice.Current();
	    _current.id = new Ice.Identity();
	    _current.adapter = adapter;
	    
	    _cookie = null;
	    
	    _response = response;
	    
	    _compress = compress;
	    
	    _os = new BasicStream(instance);
	    
	    _connection = connection;
	}
	
	protected internal IncomingBase(IncomingBase inc)
	{
	    _current = inc._current;
	    inc._current = null;
	    
	    _servant = inc._servant;
	    inc._servant = null;
	    
	    _locator = inc._locator;
	    inc._locator = null;
	    
	    _cookie = inc._cookie;
	    inc._cookie = null;
	    
	    _response = inc._response;
	    inc._response = false;
	    
	    _compress = inc._compress;
	    inc._compress = 0;
	    
	    _os = inc._os;
	    inc._os = null;
	    
	    _connection = inc._connection;
	    inc._connection = null;
	}
	
	//
	// Do NOT use a finalizer, this would cause a severe performance
	// penalty! We must make sure that __destroy() is called instead,
	// to reclaim resources.
	//
	public virtual void __destroy()
	{
	    if(_os != null)
	    {
		_os.destroy();
		_os = null;
	    }
	}
	
	//
	// This function allows this object to be reused, rather than
	// reallocated.
	//
	public virtual void reset(Instance instance, Connection connection,
	                          Ice.ObjectAdapter adapter, bool response, byte compress)
	{
	    if(_current == null)
	    {
		_current = new Ice.Current();
		_current.id = new Ice.Identity();
		_current.adapter = adapter;
	    }
	    else
	    {
		Debug.Assert(_current.id != null);
		_current.adapter = adapter;
		
		if(_current.ctx != null)
		{
		    _current.ctx.Clear();
		}
	    }
	    
	    _servant = null;
	    
	    _locator = null;
	    
	    _cookie = null;
	    
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
	
	protected internal void __finishInvoke()
	{
	    if(_locator != null && _servant != null)
	    {
		_locator.finished(_current, _servant, _cookie);
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
	
	protected internal void __warning(System.Exception ex)
	{
	    Debug.Assert(_os != null);
	    
	    IceUtil.OutputBase output = new IceUtil.OutputBase("");
	    output.setUseTab(false);
	    output.print("dispatch exception:");
	    output.print("\nidentity: " + Ice.Util.identityToString(_current.id));
	    output.print("\nfacet: ");
	    IceInternal.ValueWriter.write(_current.facet, output);
	    output.print("\noperation: " + _current.operation);
	    output.print("\n");
	    output.print(ex.ToString());
	    _os.instance().logger().warning(output.ToString()); // TODO: need to override ToString()?
	}
	
	protected internal Ice.Current _current;
	protected internal Ice.Object _servant;
	protected internal Ice.ServantLocator _locator;
	protected internal Ice.LocalObject _cookie;
	
	protected internal bool _response;
	protected internal byte _compress;
	
	protected internal BasicStream _os;
	
	private Connection _connection;
    }
	
}
