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
        _adapter = adapter;
        _is = new BasicStream(instance);
        _os = new BasicStream(instance);
        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _cookie = new Ice.LocalObjectHolder();
    }

    public void
    invoke(boolean response)
    {
        _current.id.__read(_is);
        _current.facet = _is.readStringSeq();
        _current.operation = _is.readString();
        _current.nonmutating = _is.readBool();
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

        int statusPos = 0;
        if(response)
        {
            statusPos = _os.size();
            _os.writeByte((byte)0);
        }

        //
        // Input and output parameters are always sent in an
        // encapsulation, which makes it possible to forward requests as
        // blobs.
        //
        _is.startReadEncaps();
        if(response)
        {
            _os.startWriteEncaps();
        }

        Ice.Object servant = null;
        Ice.ServantLocator locator = null;
        _cookie.value = null;

        try
        {
            if(_adapter != null)
            {
                servant = _adapter.identityToServant(_current.id);

                if(servant == null && _current.id.category.length() > 0)
                {
                    locator = _adapter.findServantLocator(_current.id.category);
                    if(locator != null)
                    {
                        servant = locator.locate(_adapter, _current, _cookie);
                    }
                }

                if(servant == null)
                {
                    locator = _adapter.findServantLocator("");
                    if(locator != null)
                    {
                        servant = locator.locate(_adapter, _current, _cookie);
                    }
                }
            }

            DispatchStatus status;

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

            if(locator != null && servant != null)
            {
                assert(_adapter != null);
                locator.finished(_adapter, _current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();

                if(status != DispatchStatus.DispatchOK && status != DispatchStatus.DispatchUserException)
                {
                    _os.resize(statusPos, false);
                    _os.writeByte((byte)status.value());

		    if(status == DispatchStatus.DispatchObjectNotExist)
		    {
			_current.id.__write(_os);
		    }
		    else if(status == DispatchStatus.DispatchFacetNotExist)
		    {
			_os.writeStringSeq(_current.facet);
		    }
		    else if(status == DispatchStatus.DispatchOperationNotExist)
		    {
			_os.writeString(_current.operation);
		    }
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
        catch(Ice.LocationForward ex)
        {
            if(locator != null && servant != null)
            {
                assert(_adapter != null);
                locator.finished(_adapter, _current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(statusPos, false);
                _os.writeByte((byte)DispatchStatus._DispatchLocationForward);
                _os.writeProxy(ex._prx);
            }
        }
        catch(Ice.ObjectNotExistException ex)
        {
            if(locator != null && servant != null)
            {
                assert(_adapter != null);
                locator.finished(_adapter, _current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(statusPos, false);
                _os.writeByte((byte)DispatchStatus._DispatchObjectNotExist);
		// Not current.id.__write(_os), so that the identity
		// can be overwritten.
		ex.id.__write(_os);
            }

	    // Rethrow, so that the caller can print a warning.
            throw ex;
        }
        catch(Ice.FacetNotExistException ex)
        {
            if(locator != null && servant != null)
            {
                assert(_adapter != null);
                locator.finished(_adapter, _current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(statusPos, false);
                _os.writeByte((byte)DispatchStatus._DispatchFacetNotExist);
		// Not _os.write(current.facet), so that the facet can
		// be overwritten.
		_os.writeStringSeq(ex.facet);
            }

	    // Rethrow, so that the caller can print a warning.
            throw ex;
        }
        catch(Ice.OperationNotExistException ex)
        {
            if(locator != null && servant != null)
            {
                assert(_adapter != null);
                locator.finished(_adapter, _current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(statusPos, false);
                _os.writeByte((byte)DispatchStatus._DispatchOperationNotExist);
		// Not _os.write(current.operation), so that the
		// operation can be overwritten.
		_os.writeString(ex.operation);
            }

	    // Rethrow, so that the caller can print a warning.
            throw ex;
        }
        catch(Ice.LocalException ex)
        {
            if(locator != null && servant != null)
            {
                assert(_adapter != null);
                locator.finished(_adapter, _current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(statusPos, false);
                _os.writeByte(
                    (byte)DispatchStatus._DispatchUnknownLocalException);
            }

	    // Rethrow, so that the caller can print a warning.
            throw ex;
        }
        /* Not possible in Java - UserExceptions are checked exceptions
        catch(Ice.UserException ex)
        {
            if(locator != null && servant != null)
            {
                assert(_adapter != null);
                locator.finished(_adapter, _current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(statusPos, false);
                _os.writeByte(
                    (byte)DispatchStatus._DispatchUnknownUserException);
            }

            throw ex;
        }
        */
        catch(RuntimeException ex)
        {
            if(locator != null && servant != null)
            {
                assert(_adapter != null);
                locator.finished(_adapter, _current, servant, _cookie.value);
            }

            _is.endReadEncaps();
            if(response)
            {
                _os.endWriteEncaps();
                _os.resize(statusPos, false);
                _os.writeByte((byte)DispatchStatus._DispatchUnknownException);
            }

	    // Rethrow, so that the caller can print a warning.
            throw ex;
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

    //
    // reset() allows this object to be reused, rather than reallocated
    //
    public void
    reset()
    {
        _is.reset();
        _os.reset();
        if(_current.ctx != null)
        {
            _current.ctx.clear();
        }
    }

    //
    // Reclaim resources
    //
    public void
    destroy()
    {
        _is.destroy();
        _os.destroy();
    }

    private Ice.ObjectAdapter _adapter;
    private BasicStream _is;
    private BasicStream _os;
    private Ice.Current _current;
    private Ice.LocalObjectHolder _cookie;

    Incoming next; // For use by Connection
}
