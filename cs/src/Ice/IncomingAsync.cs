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

    using System.Diagnostics;

    public class IncomingAsync : IncomingBase
    {
        public IncomingAsync(Incoming inc)
            : base(inc)
        {
        }
	
        ~IncomingAsync()
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
	
        protected internal void __response(bool ok)
        {
	    try
	    {
		if(_locator != null && _servant != null)
		{
		    _locator.finished(_current, _servant, _cookie);
		}

		if(_response)
		{
		    _os.endWriteEncaps();
		
		    int save = _os.pos();
		    _os.pos(Protocol.headerSize + 4); // Dispatch status position.
		
		    if(ok)
		    {
			_os.writeByte((byte)DispatchStatus.DispatchOK);
		    }
		    else
		    {
			_os.writeByte((byte)DispatchStatus.DispatchUserException);
		    }
		
		    _os.pos(save);

		    _connection.sendResponse(_os, _compress);
		}
		else
		{
		    _connection.sendNoResponse();
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		_connection.exception(ex);
	    }
	    catch(System.Exception ex)
	    {
		Ice.UnknownException uex = new Ice.UnknownException();
		uex.unknown = ex.ToString();
		_connection.exception(uex);
	    }
        }
	
        protected internal void __exception(System.Exception exc)
        {
	    try
	    {
		if(_locator != null && _servant != null)
		{
		    _locator.finished(_current, _servant, _cookie);
		}

		try
		{
		    throw exc;
		}
		catch(Ice.RequestFailedException ex)
		{
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
			    string[] facetPath = { ex.facet };
			    _os.writeStringSeq(facetPath);
			}

			_os.writeString(ex.operation);

			_connection.sendResponse(_os, _compress);
		    }
		    else
		    {
			_connection.sendNoResponse();
		    }
		}
		catch(Ice.UnknownLocalException ex)
		{
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
		}
		catch(Ice.UnknownUserException ex)
		{
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
		}
		catch(Ice.UnknownException ex)
		{
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
			_os.writeByte((byte)DispatchStatus.DispatchUnknownLocalException);
			_os.writeString(ex.ToString());
			_connection.sendResponse(_os, _compress);
		    }
		    else
		    {
			_connection.sendNoResponse();
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
			_os.writeByte((byte)DispatchStatus.DispatchUnknownUserException);
			_os.writeString(ex.ToString());
			_connection.sendResponse(_os, _compress);
		    }
		    else
		    {
			_connection.sendNoResponse();
		    }
		}
		catch(System.Exception ex)
		{
		    if(_os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
		    {
			__warning(ex);
		    }
		
		    if(_response)
		    {
			_os.endWriteEncaps();
			_os.resize(Protocol.headerSize + 4, false); // Dispatch status position.
			_os.writeByte((byte)DispatchStatus.DispatchUnknownException);
			_os.writeString(ex.ToString());
			_connection.sendResponse(_os, _compress);
		    }
		    else
		    {
			_connection.sendNoResponse();
		    }
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		_connection.exception(ex);
	    }
	    catch(System.Exception ex)
	    {
		Ice.UnknownException uex = new Ice.UnknownException();
		uex.unknown = ex.ToString();
		_connection.exception(uex);
	    }
        }
	
        protected internal BasicStream __os()
        {
            return _os;
        }
    }
}

namespace Ice
{

    public interface AMD_Object_ice_invoke
    {
        void ice_response(bool ok, byte[] outParams);
        void ice_exception(System.Exception ex);
    }

    sealed class _AMD_Object_ice_invoke : IceInternal.IncomingAsync, AMD_Object_ice_invoke
    {
        public _AMD_Object_ice_invoke(IceInternal.Incoming inc)
            : base(inc)
        {
        }
	
        public void ice_response(bool ok, byte[] outParams)
        {
	    try
	    {
		__os().writeBlob(outParams);
	    }
	    catch(Ice.LocalException ex)
	    {
		__exception(ex);
		return;
	    }
	    __response(ok);
	}
	
        public void ice_exception(System.Exception ex)
        {
	    __exception(ex);
        }
    }
}
