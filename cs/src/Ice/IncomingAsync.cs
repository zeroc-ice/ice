// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
            _finished = false;
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
            Debug.Assert(!_finished);
            _finished = true;
	    
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
            }
	    
            //
            // Must be called last, so that if an exception is raised,
            // this function is definitely *not* called.
            //
            __finishInvoke();
        }
	
        protected internal void __exception(System.Exception exc)
        {
            Debug.Assert(!_finished);
            _finished = true;
	    
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
                }
            }
	    
            //
            // Must be called last, so that if an exception is raised,
            // this function is definitely *not* called.
            //
            __finishInvoke();
        }
	
        protected internal BasicStream __os()
        {
            return _os;
        }
	
        protected internal bool _finished;
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
