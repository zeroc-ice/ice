// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
        _current.adapter = adapter;

	if(_current.adapter != null)
	{
	    ((Ice.ObjectAdapterI)(_current.adapter)).incUsageCount();
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

        int statusPos;
        if(response)
        {
            statusPos = _os.size();
            _os.writeByte((byte)0);
            _os.startWriteEncaps();
        }
	else
	{
	    statusPos = 0; // Initialize, to keep the compiler happy.
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
        catch(Ice.LocationForward ex)
        {
            if(locator != null && servant != null)
            {
                locator.finished(_current, servant, _cookie.value);
            }

            _is.endReadEncaps();

            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(statusPos, false);
                _os.writeByte((byte)DispatchStatus._DispatchLocationForward);
                _os.writeProxy(ex._prx);
            }

	    return;
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
                _os.resize(statusPos, false);
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
		// Write the data from the exception, not from _current,
		// so that a RequestFailedException can override the
		// information from _current.
		ex.id.__write(_os);
		_os.writeStringSeq(ex.facet);
		_os.writeString(ex.operation);
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
                _os.resize(statusPos, false);
                _os.writeByte((byte)DispatchStatus._DispatchUnknownLocalException);
		_os.writeString(ex.toString());
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
                _os.resize(statusPos, false);
                _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
		_os.writeString(ex.toString());
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
		
		_os.resize(statusPos, false);
		_os.writeByte((byte)status.value());
		
		_current.id.__write(_os);
		_os.writeStringSeq(_current.facet);
		_os.writeString(_current.operation);
	    }
	    else
	    {
		int save = _os.pos();
		_os.pos(statusPos);
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

    private BasicStream _is;
    private BasicStream _os;
    private Ice.Current _current;
    private Ice.LocalObjectHolder _cookie;

    Incoming next; // For use by Connection
}
