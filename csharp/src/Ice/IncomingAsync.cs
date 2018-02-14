// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;

    public class IncomingAsync : IncomingBase, Ice.AMDCallback
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

        virtual public void ice_exception(System.Exception ex)
        {
            //
            // Only call exception__ if this incoming is not retriable or if
            // all the interceptors return true and no response has been sent
            // yet.
            //
            
            if(_retriable)
            {
                try
                {
                    if(interceptorAsyncCallbackList_ != null)
                    {
                        foreach(Ice.DispatchInterceptorAsyncCallback cb in  interceptorAsyncCallbackList_)
                        {
                            if(cb.exception(ex) == false)
                            {
                                return;
                            }
                        }
                    }
                }
                catch(System.Exception)
                {
                    return;
                }
    
                lock(this)
                {
                    if(!_active)
                    {
                        return;
                    }
                    _active = false;
                }
            }

            if(responseHandler_ != null)
            {
                exception__(ex);
            }
            else
            {
                //
                // Response has already been sent.
                //
                if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning__(ex);
                }
            }
        }

        internal void deactivate__(Incoming inc)
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

        protected void response__()
        {
            try
            {
                if(locator_ != null && !servantLocatorFinished__(true))
                {
                    return;
                }

                Debug.Assert(responseHandler_ != null);

                if(response_)
                {
                    if(observer_ != null)
                    {
                        observer_.reply(os_.size() - Protocol.headerSize - 4);
                    }
                    responseHandler_.sendResponse(current_.requestId, os_, compress_, true);
                }
                else
                {
                    responseHandler_.sendNoResponse();
                }

                if(observer_ != null)
                {
                    observer_.detach();
                    observer_ = null;
                }
                responseHandler_ = null;
            }
            catch(Ice.LocalException ex)
            {
                responseHandler_.invokeException(current_.requestId, ex, 1, true);
            }
        }
        
        protected internal void exception__(System.Exception exc)
        {
            try
            {
                if(locator_ != null && !servantLocatorFinished__(true))
                {
                    return;
                }

                handleException__(exc, true);
            }
            catch(Ice.LocalException ex)
            {
                responseHandler_.invokeException(current_.requestId, ex, 1, true);
            }
        }
        
        protected internal BasicStream getOs__()
        {
            return os_;
        }

        protected bool validateResponse__(bool ok)
        {
            //
            // Only returns true if this incoming is not retriable or if all
            // the interceptors return true and no response has been sent
            // yet. Upon getting a true return value, the caller should send
            // the response.
            //

            if(_retriable)
            {
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
            
                lock(this)
                {   
                    if(!_active)
                    {
                        return false;
                    }
                    _active = false;
                }
            }
            return true;
        }

        private readonly bool _retriable;
        private bool _active = false; // only meaningful when _retriable == true
    }
}

namespace Ice
{

    /// <summary>
    /// Callback interface for Blobject AMD servants.
    /// </summary>
    public interface AMD_Object_ice_invoke : Ice.AMDCallback
    {
        /// <summary>
        /// Indicates to the Ice run time that an operation
        /// completed.
        /// </summary>
        /// <param name="ok">True indicates that the operation
        /// completed successfully; false indicates that the
        /// operation raised a user exception.</param>
        /// <param name="outEncaps">The encoded out-parameters for the operation or,
        /// if ok is false, the encoded user exception.</param>
        void ice_response(bool ok, byte[] outEncaps);
    }

    sealed class _AMD_Object_ice_invoke : IceInternal.IncomingAsync, AMD_Object_ice_invoke
    {
        public _AMD_Object_ice_invoke(IceInternal.Incoming inc)
            : base(inc)
        {
        }
        
        public void ice_response(bool ok, byte[] outEncaps)
        {
            try
            {
                writeParamEncaps__(outEncaps, ok);
            }
            catch(Ice.LocalException ex)
            {
                exception__(ex);
                return;
            }
            response__();
        }
    }
}
