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

public class Incoming
{
    public
    Incoming(Instance instance, Ice.ObjectAdapter adapter)
    {
        _is = new BasicStream(instance);
        _os = new BasicStream(instance);
        _current = new Ice.Current();
        _current.adapter = adapter;
        _current.id = new Ice.Identity();
        _cookie = new Ice.LocalObjectHolder();

	if(_current.adapter != null)
	{
	    ((Ice.ObjectAdapterI)(_current.adapter)).incUsageCount();
	}
    }

    //
    // This function allows this object to be reused, rather than
    // reallocated.
    //
    public void
    reset(Ice.ObjectAdapter adapter)
    {
        _is.reset();
        _os.reset();
        if(_current.ctx != null)
        {
            _current.ctx.clear();
        }

        if(_current.adapter != adapter)
        {
            if(_current.adapter != null)
            {
                ((Ice.ObjectAdapterI)(_current.adapter)).decUsageCount();
            }

            _current.adapter = adapter;

            if(_current.adapter != null)
            {
		((Ice.ObjectAdapterI)(_current.adapter)).incUsageCount();
            }
        }
    }

    //
    // Reclaim resources.
    //
    public void
    destroy()
    {
        _is.destroy();
        _os.destroy();

	if(_current.adapter != null)
	{
	    ((Ice.ObjectAdapterI)(_current.adapter)).decUsageCount();
	}
    }

    public void
    invoke(boolean response)
    {
	//
	// Read the current.
	//
        _current.id.__read(_is);
        _current.facet = _is.readStringSeq();
        _current.operation = _is.readString();
        _current.mode = Ice.OperationMode.convert(_is.readByte());
        int sz = _is.readSize();
        while(sz-- > 0)
        {
            String first = _is.readString();
            String second = _is.readString();
            if(_current.ctx == null)
            {
                _current.ctx = new java.util.HashMap();
            }
            _current.ctx.put(first, second);
        }

        _is.startReadEncaps();

        if(response)
        {
            assert(_os.size() == Protocol.headerSize + 4); // Dispatch status position.
            _os.writeByte((byte)0);
            _os.startWriteEncaps();
        }

        Ice.Object servant = null;
        Ice.ServantLocator locator = null;
        _cookie.value = null;
	DispatchStatus status;
	
	//
	// Don't put the code above into the try block below. Exceptions
	// in the code above are considered fatal, and must propagate to
	// the caller of this operation.
	//

        try
        {
	    if(_current.adapter != null)
	    {
		servant = _current.adapter.identityToServant(_current.id);
		
		if(servant == null && _current.id.category.length() > 0)
		{
		    locator = _current.adapter.findServantLocator(_current.id.category);
		    if(locator != null)
		    {
			servant = locator.locate(_current, _cookie);
		    }
		}
		
		if(servant == null)
		{
		    locator = _current.adapter.findServantLocator("");
		    if(locator != null)
		    {
			servant = locator.locate(_current, _cookie);
		    }
		}
	    }

            if(servant == null)
            {
                status = DispatchStatus.DispatchObjectNotExist;
            }
            else
            {
                if(_current.facet.length > 0)
                {
                    Ice.Object facetServant = servant.ice_findFacetPath(_current.facet, 0);
                    if(facetServant == null)
                    {
                        status = DispatchStatus.DispatchFacetNotExist;
                    }
                    else
                    {
                        status = facetServant.__dispatch(this, _current);
                    }
                }
                else
                {
                    status = servant.__dispatch(this, _current);
                }
            }
        }
        catch(Ice.RequestFailedException ex)
        {
            if(locator != null && servant != null)
            {
                locator.finished(_current, servant, _cookie.value);
            }

            _is.endReadEncaps();

            if(response)
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

		//
		// Write the data from the exception if set so that a
		// RequestFailedException can override the information
		// from _current.
		//
		if(ex.id != null)
		{
		    ex.id.__write(_os);
		}
		else
		{
		    _current.id.__write(_os);
		}
		
		if(ex.facet != null)
		{
		    _os.writeStringSeq(ex.facet);
		}
		else
		{
		    _os.writeStringSeq(_current.facet);
		}
		
		if(ex.operation != null && ex.operation.length() > 0)
		{
		    _os.writeString(ex.operation);
		}
		else
		{
		    _os.writeString(_current.operation);
		}
            }

	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
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

		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		String s = "dispatch exception:\n" + sw.toString();
		_os.instance().logger().warning(s);
	    }

	    return;
        }
        catch(Ice.LocalException ex)
        {
            if(locator != null && servant != null)
            {
                locator.finished(_current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownLocalException);
		_os.writeString(ex.toString());
            }

	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		warning("dispatch exception: unknown local exception:", ex);
	    }

	    return;
        }
        /* Not possible in Java - UserExceptions are checked exceptions
        catch(Ice.UserException ex)
        {
	// ...
	}
	*/
        catch(RuntimeException ex)
        {
            if(locator != null && servant != null)
            {
                locator.finished(_current, servant, _cookie.value);
            }

            _is.endReadEncaps();

            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
                _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
		_os.writeString(ex.toString());
            }
	    
	    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		warning("dispatch exception: unknown exception:", ex);
	    }

	    return;
        }
	
	if(locator != null && servant != null)
	{
	    locator.finished(_current, servant, _cookie.value);
	}
	
	_is.endReadEncaps();

	if(response)
	{
	    _os.endWriteEncaps();
	    
	    if(status != DispatchStatus.DispatchOK && status != DispatchStatus.DispatchUserException)
	    {
		assert(status == DispatchStatus.DispatchObjectNotExist ||
		       status == DispatchStatus.DispatchFacetNotExist ||
		       status == DispatchStatus.DispatchOperationNotExist);
		
		_os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
		_os.writeByte((byte)status.value());
		
		_current.id.__write(_os);
		_os.writeStringSeq(_current.facet);
		_os.writeString(_current.operation);
	    }
	    else
	    {
		int save = _os.pos();
		_os.pos(Protocol.headerSize + 4); // Dispatch status position.
		_os.writeByte((byte)status.value());
		_os.pos(save);
	    }
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

    private void
    warning(String s, Exception ex)
    {
	java.io.StringWriter sw = new java.io.StringWriter();
	java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	IceUtil.OutputBase out = new IceUtil.OutputBase(pw);
	out.setUseTab(false);
	out.print(s);
	out.print("\nidentity: " + Ice.Util.identityToString(_current.id));
	out.print("\nfacet: ");
	IceInternal.ValueWriter.write(_current.facet, out);
	out.print("\noperation: " + _current.operation);
	out.print("\n");
	ex.printStackTrace(pw);
	pw.flush();
	_os.instance().logger().warning(sw.toString());
    }

    private BasicStream _is;
    private BasicStream _os;
    private Ice.Current _current;
    private Ice.LocalObjectHolder _cookie;

    Incoming next; // For use by Connection
}
