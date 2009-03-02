// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
            _retriable = inc.isRetriable();
            if(_retriable)
            {
                inc.setActive(this);
                _active = true;
            }
        }

        internal void
        deactivate__(Incoming inc)
        {
            Debug.Assert(_retriable);
            
            lock(this)
            {
                if(!_active)
                {
                    //
                    // Since __deactivate can only be called on an active object,
                    // this means the response has already been sent (see validateXXX below)
                    //
                    throw new Ice.ResponseSentException();
                }
                _active = false;
            }
            inc.adopt(this);
        }


        protected void response__(bool ok)
        {
            try
            {
                if(locator_ != null && !servantLocatorFinished__())
                {
                    return;
                }

                if(response_)
                {
                    os_.endWriteEncaps();
                
                    int save = os_.pos();
                    os_.pos(Protocol.headerSize + 4); // Reply status position.
                
                    if(ok)
                    {
                        os_.writeByte(ReplyStatus.replyOK);
                    }
                    else
                    {
                        os_.writeByte(ReplyStatus.replyUserException);
                    }
                
                    os_.pos(save);

                    connection_.sendResponse(os_, compress_);
                }
                else
                {
                    connection_.sendNoResponse();
                }
            }
            catch(Ice.LocalException ex)
            {
                connection_.invokeException(ex, 1);
            }
        }
        
        protected internal void exception__(System.Exception exc)
        {
            try
            {
                if(locator_ != null && !servantLocatorFinished__())
                {
                    return;
                }

                handleException__(exc);
            }
            catch(Ice.LocalException ex)
            {
                connection_.invokeException(ex, 1);
            }
        }
        
        protected internal BasicStream os__()
        {
            return os_;
        }

        protected bool 
        validateResponse__(bool ok)
        {
            if(!_retriable)
            {
                return true;
            }
            
            try
            {
                if(interceptorAsyncCallbackList_ != null)
                {
                    foreach(Ice.DispatchInterceptorAsyncCallback cb in  interceptorAsyncCallbackList_)
                    {
                        if(cb.response(ok) == false)
                        {
                            return false;
                        }
                    }
                }
            }
            catch(System.Exception)
            {
                return false;
            }
            
            //
            // interceptorAsyncCallbackList is null or all its elements returned OK
            // 
            
            lock(this)
            {   
                if(_active)
                {
                    _active = false;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        
        protected bool 
        validateException__(System.Exception exc)
        {
            if(!_retriable)
            {
                return true;
            }
            
            try
            {
                if(interceptorAsyncCallbackList_ != null)
                {
                    foreach(Ice.DispatchInterceptorAsyncCallback cb in  interceptorAsyncCallbackList_)
                    {
                        if(cb.exception(exc) == false)
                        {
                            return false;
                        }
                    }
                }
            }
            catch(System.Exception)
            {
                return false;
            }

            //
            // interceptorAsyncCallbackList is null or all its elements returned OK
            // 
            
            lock(this)
            {
                if(_active)
                {
                    _active = false;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        private readonly bool _retriable;
        private bool _active = false; // only meaningful when _retriable == true
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
                os__().writeBlob(outParams);
            }
            catch(Ice.LocalException ex)
            {
                exception__(ex);
                return;
            }
            response__(ok);
        }
        
        public void ice_exception(System.Exception ex)
        {
            exception__(ex);
        }
    }
}
