// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;
    using System.IO;

    public class IncomingBase
    {
	protected internal IncomingBase(Instance instance, Ice.ConnectionI connection,
	                                Ice.ObjectAdapter adapter, bool response, byte compress)
	{
	    _response = response;
	    _compress = compress;
	    _os = new BasicStream(instance);
	    _connection = connection;

	    _current = new Ice.Current();
	    _current.id = new Ice.Identity();
	    _current.adapter = adapter;
	    _current.con = connection;
	    
	    _cookie = null;
	}
	
	protected internal IncomingBase(IncomingBase inc)
	{
	    _current = inc._current;
	    
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
	public virtual void reset(Instance instance, Ice.ConnectionI connection,
	                          Ice.ObjectAdapter adapter, bool response, byte compress)
	{
            //
            // Don't recycle the Current object, because servants may keep a reference to it.
            //
	    _current = new Ice.Current();
	    _current.id = new Ice.Identity();
	    _current.adapter = adapter;
	    _current.con = connection;
	    
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
	
	protected internal void __warning(System.Exception ex)
	{
	    Debug.Assert(_os != null);
	    
	    StringWriter sw = new StringWriter();
	    IceUtil.OutputBase output = new IceUtil.OutputBase(sw);
	    output.setUseTab(false);
	    output.print("dispatch exception:");
	    output.print("\nidentity: " + Ice.Util.identityToString(_current.id));
	    output.print("\nfacet: " + IceUtil.StringUtil.escapeString(_current.facet, ""));
	    output.print("\noperation: " + _current.operation);
	    output.print("\n");
	    output.print(ex.ToString());
	    _os.instance().logger().warning(sw.ToString());
	}
	
	protected internal Ice.Current _current;
	protected internal Ice.Object _servant;
	protected internal Ice.ServantLocator _locator;
	protected internal Ice.LocalObject _cookie;
	
	protected internal bool _response;
	protected internal byte _compress;
	
	protected internal BasicStream _os;
	
	protected Ice.ConnectionI _connection;
    }
	
    sealed public class Incoming : IncomingBase
    {
	public Incoming(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter,
		        bool response, byte compress)
	     : base(instance, connection, adapter, response, compress)
	{
	    _is = new BasicStream(instance);
	}
	
	//
	// Do NOT user a finalizer, this would cause a sever perforrmance
	// penalty! We must make sure that __destroy() is called instead,
	// to reclaim resources.
	//
	public override void __destroy()
	{
	    base.__destroy();
	    
	    if(_is != null)
	    {
		_is.destroy();
		_is = null;
	    }
	}
	
	//
	// This function allows this object to be reused, rather than
	// reallocated.
	//
	public override void reset(Instance instance, Ice.ConnectionI connection,
	                           Ice.ObjectAdapter adapter, bool response, byte compress)
	{
	    if(_is == null)
	    {
		_is = new BasicStream(instance);
	    }
	    else
	    {
		_is.reset();
	    }

	    base.reset(instance, connection, adapter, response, compress);
	}
	
	public void invoke(ServantManager servantManager)
	{
	    //
	    // Read the current.
	    //
	    _current.id.__read(_is);

            //
            // For compatibility with the old FacetPath.
            //
            string[] facetPath = _is.readStringSeq();
            if(facetPath.Length > 0)
            {
                if(facetPath.Length > 1)
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
	    _current.mode = (Ice.OperationMode)(int)_is.readByte();
	    int sz = _is.readSize();
	    while(sz-- > 0)
	    {
		string first = _is.readString();
		string second = _is.readString();
		if(_current.ctx == null)
		{
		    _current.ctx = new Ice.Context();
		}
		_current.ctx[first] = second;
	    }
	    
	    _is.startReadEncaps();
	    
	    if(_response)
	    {
		Debug.Assert(_os.size() == Protocol.headerSize + 4); // Dispatch status position.
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
			
			if(_servant == null && _current.id.category.Length > 0)
			{
			    _locator = servantManager.findServantLocator(_current.id.category);
			    if(_locator != null)
			    {
				_servant = _locator.locate(_current, out _cookie);
			    }
			}
			
			if(_servant == null)
			{
			    _locator = servantManager.findServantLocator("");
			    if(_locator != null)
			    {
				_servant = _locator.locate(_current, out _cookie);
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
			_locator.finished(_current, _servant, _cookie);
		    }
		}
	    }
	    catch(Ice.RequestFailedException ex)
	    {
	        _is.endReadEncaps();

		if(ex.id.name == null)
		{
		    ex.id = _current.id;
		}
		
		if(ex.facet == null)
		{
		    ex.facet = _current.facet;
		}
		
		if(ex.operation == null || ex.operation.Length == 0)
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
		    if(ex is Ice.ObjectNotExistException)
		    {
			_os.writeByte((byte)DispatchStatus.DispatchObjectNotExist);
		    }
		    else if(ex is Ice.FacetNotExistException)
		    {
			_os.writeByte((byte)DispatchStatus.DispatchFacetNotExist);
		    }
		    else if(ex is Ice.OperationNotExistException)
		    {
			_os.writeByte((byte)DispatchStatus.DispatchOperationNotExist);
		    }
		    else
		    {
			Debug.Assert(false);
		    }
		    ex.id.__write(_os);
 
                    //
                    // For compatibility with the old FacetPath.
                    //
                    if(ex.facet == null || ex.facet.Length == 0)
                    {
                        _os.writeStringSeq(null);
                    }
                    else
                    {
                        string[] facetPath2 = { ex.facet };
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
		    _os.writeByte((byte)DispatchStatus.DispatchUnknownLocalException);
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
		    _os.writeByte((byte)DispatchStatus.DispatchUnknownUserException);
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
		    _os.writeByte((byte)DispatchStatus.DispatchUnknownException);
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
		    _os.writeByte((byte)DispatchStatus.DispatchUnknownLocalException);
		    _os.writeString(ex.ToString());
		    _connection.sendResponse(_os, _compress);
		}
		else
		{
		    _connection.sendNoResponse();
		}

		return;
	    }

	    catch(Ice.UserException ex)
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
		    _os.writeByte((byte)DispatchStatus.DispatchUnknownUserException);
		    _os.writeString(ex.ToString());
		    _connection.sendResponse(_os, _compress);
		}
		else
		{
		    _connection.sendNoResponse();
		}

		return;
	    }

	    catch(System.Exception ex)
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
		    _os.writeByte((byte) DispatchStatus.DispatchUnknownException);
		    _os.writeString(ex.ToString());
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
		// If this was an asynchronous dispatch, we're done
		// here.
		//
		return;
	    }
	    
	    if(_response)
	    {
		_os.endWriteEncaps();
		
		if(status != DispatchStatus.DispatchOK && status != DispatchStatus.DispatchUserException)
		{
		    Debug.Assert(status == DispatchStatus.DispatchObjectNotExist ||
				 status == DispatchStatus.DispatchFacetNotExist ||
				 status == DispatchStatus.DispatchOperationNotExist);
		    
		    _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		    _os.writeByte((byte)status);
		    
		    _current.id.__write(_os);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    if(_current.facet == null || _current.facet.Length == 0)
                    {
                        _os.writeStringSeq(null);
                    }
                    else
                    {
                        string[] facetPath2 = { _current.facet };
                        _os.writeStringSeq(facetPath2);
                    }

		    _os.writeString(_current.operation);
		}
		else
		{
		    int save = _os.pos();
		    _os.pos(Protocol.headerSize + 4); // Dispatch status position.
		    _os.writeByte((byte)status);
		    _os.pos(save);
		}

		_connection.sendResponse(_os, _compress);
	    }
	    else
	    {
		_connection.sendNoResponse();
	    }
	}
	
	public BasicStream istr()
	{
	    return _is;
	}
	
	public BasicStream ostr()
	{
	    return _os;
	}
	
	public Incoming next; // For use by Connection.
	
	private BasicStream _is;
    }

}
