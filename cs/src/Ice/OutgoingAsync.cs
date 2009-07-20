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

    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public abstract class OutgoingAsyncMessageCallback
    {
        public abstract void sent__(Ice.ConnectionI connection);
        public abstract void finished__(Ice.LocalException ex);
        public abstract void ice_exception(Ice.Exception ex);

        public BasicStream ostr__()
        {
            return os__;
        }

        public void sent__(Instance instance)
        {
            try
            {
                ((Ice.AMISentCallback)this).ice_sent();
            }
            catch(System.Exception ex)
            {
                warning__(instance, ex);
            }
        }
        
        public void exception__(Ice.Exception exc)
        {
            try
            {
                ice_exception(exc);
            }
            catch(System.Exception ex)
            {
                warning__(ex);
            }
            finally
            {
                releaseCallback__();
            }
        }

        protected void acquireCallback__(Ice.ObjectPrx proxy)
        {
            lock(monitor__)
            {
                //
                // We must first wait for other requests to finish.
                //
                while(os__ != null)
                {
                    Monitor.Wait(monitor__);
                }
                
                Reference rf = ((Ice.ObjectPrxHelperBase)proxy).reference__();
                Debug.Assert(is__ == null);
                is__ = new BasicStream(rf.getInstance());
                Debug.Assert(os__ == null);
                os__ = new BasicStream(rf.getInstance());
            }
        }

        public void releaseCallback__(Ice.LocalException ex)
        {
            Debug.Assert(os__ != null);
            
            //
            // This is called by the invoking thread to release the callback following a direct 
            // failure to marshal/send the request. We call the ice_exception() callback with
            // the thread pool to avoid potential deadlocks in case the invoking thread locked 
            // some mutexes/resources (which couldn't be re-acquired by the callback).
            //
            
            try
            {
                os__.instance().clientThreadPool().execute(delegate(bool unused)
                                                           {
                                                               exception__(ex);
                                                           });
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                releaseCallback__();
                throw; // CommunicatorDestroyedException is the only exception that can propagate directly.
            }
        }

        protected void releaseCallback__()
        {
            lock(monitor__)
            {
                Debug.Assert(is__ != null);
                is__ = null;
                
                Debug.Assert(os__ != null);
                os__ = null;
                
                Monitor.Pulse(monitor__);
            }
        }

        protected void warning__(System.Exception ex)
        {
            if(os__ != null) // Don't print anything if cleanup() was already called.
            {
                warning__(os__.instance(), ex);
            }
        }

        protected void warning__(Instance instance, System.Exception ex)
        {
            if(instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
            {
                instance.initializationData().logger.warning("exception raised by AMI callback:\n" + ex);
            }
        }

        protected object monitor__ = new object();
        protected BasicStream is__;
        protected BasicStream os__; // Cannot rename because the generated code assumes this name
    }

    public abstract class OutgoingAsync : OutgoingAsyncMessageCallback
    {
        private class TaskI : TimerTask
        {
            internal TaskI(OutgoingAsync @out, Ice.ConnectionI connection)
            {
                _out = @out;
                _connection = connection;
            }

            public void runTimerTask()
            {
                _out.runTimerTask__(_connection);
            }

            private OutgoingAsync _out;
            private Ice.ConnectionI _connection;
        }

        public override void sent__(Ice.ConnectionI connection)
        {
            lock(monitor__)
            {
                _sent = true;

                if(!_proxy.ice_isTwoway())
                {
                    releaseCallback__();
                }
                else if(_response)
                {
                    //
                    // If the response was already received notify finished() which is waiting.
                    //
                    Monitor.PulseAll(monitor__);
                }
                else if(connection.timeout() >= 0)
                {   
                    Debug.Assert(_timerTask == null);
                    _timerTask = new TaskI(this, connection);
                    _proxy.reference__().getInstance().timer().schedule(_timerTask, connection.timeout());
                }
            }
        }

        public void finished__(BasicStream istr)
        {
            Debug.Assert(_proxy.ice_isTwoway()); // Can only be called for twoways.

            byte replyStatus;
            try
            {
                lock(monitor__)
                {
                    Debug.Assert(os__ != null);
                    _response = true;

                    if(_timerTask != null && _proxy.reference__().getInstance().timer().cancel(_timerTask))
                    {
                        _timerTask = null; // Timer cancelled.
                    }

                    while(!_sent || _timerTask != null)
                    {
                        Monitor.Wait(monitor__);
                    }

                    is__.swap(istr);
                    replyStatus = is__.readByte();

                    switch(replyStatus)
                    {
                        case ReplyStatus.replyOK:
                        case ReplyStatus.replyUserException:
                        {
                            break;
                        }

                        case ReplyStatus.replyObjectNotExist:
                        case ReplyStatus.replyFacetNotExist:
                        case ReplyStatus.replyOperationNotExist:
                        {
                            Ice.Identity id = new Ice.Identity();
                            id.read__(is__);

                            //
                            // For compatibility with the old FacetPath.
                            //
                            string[] facetPath = is__.readStringSeq();
                            string facet;
                            if(facetPath.Length > 0)
                            {
                                if(facetPath.Length > 1)
                                {
                                    throw new Ice.MarshalException();
                                }
                                facet = facetPath[0];
                            }
                            else
                            {
                                facet = "";
                            }

                            string operation = is__.readString();

                            Ice.RequestFailedException ex = null;
                            switch(replyStatus)
                            {
                            case ReplyStatus.replyObjectNotExist:
                            {
                                ex = new Ice.ObjectNotExistException();
                                break;
                            }

                            case ReplyStatus.replyFacetNotExist:
                            {
                                ex = new Ice.FacetNotExistException();
                                break;
                            }

                            case ReplyStatus.replyOperationNotExist:
                            {
                                ex = new Ice.OperationNotExistException();
                                break;
                            }

                            default:
                            {
                                Debug.Assert(false);
                                break;
                            }
                            }

                            ex.id = id;
                            ex.facet = facet;;
                            ex.operation = operation;
                            throw ex;
                        }

                        case ReplyStatus.replyUnknownException:
                        case ReplyStatus.replyUnknownLocalException:
                        case ReplyStatus.replyUnknownUserException:
                        {
                            string unknown = is__.readString();

                            Ice.UnknownException ex = null;
                            switch(replyStatus)
                            {
                            case ReplyStatus.replyUnknownException:
                            {
                                ex = new Ice.UnknownException();
                                break;
                            }
                            case ReplyStatus.replyUnknownLocalException:
                            {
                                ex = new Ice.UnknownLocalException();
                                break;
                            }
                            case ReplyStatus.replyUnknownUserException:
                            {
                                ex = new Ice.UnknownUserException();
                                break;
                            }
                            default:
                            {
                                Debug.Assert(false);
                                break;
                            }
                            }
                            ex.unknown = unknown;
                            throw ex;
                        }

                        default:
                        {
                            throw new Ice.UnknownReplyStatusException();
                        }
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                finished__(ex);
                return;
            }

            Debug.Assert(replyStatus == ReplyStatus.replyOK || replyStatus == ReplyStatus.replyUserException);

            try
            {
                response__(replyStatus == ReplyStatus.replyOK);
            }
            catch(System.Exception ex)
            {
                warning__(ex);
                releaseCallback__();
            }
        }

        public override void finished__(Ice.LocalException exc)
        {
            lock(monitor__)
            {
                if(os__ != null) // Might be called from prepare__ or before prepare__
                {
                    if(_timerTask != null && _proxy.reference__().getInstance().timer().cancel(_timerTask))
                    {
                        _timerTask = null; // Timer cancelled.
                    }

                    while(_timerTask != null)
                    {
                        Monitor.Wait(monitor__);
                    }
                }
            }

            //
            // NOTE: at this point, synchronization isn't needed, no other threads should be
            // calling on the callback.
            //
                
            try
            {
                handleException(exc); // This will throw if the invocation can't be retried.
            }
            catch(Ice.LocalException ex)
            {
                exception__(ex);
            }
        }
        
        public void finished__(LocalExceptionWrapper ex)
        {
            Debug.Assert(os__ != null && !_sent);

            try
            {
                handleException(ex); // This will throw if the invocation can't be retried.
            }
            catch(Ice.LocalException exc)
            {
                exception__(exc);
            }
        }

        public void retry__(int interval)
        {
            //
            // This method is called by the proxy to retry an
            // invocation. No other threads can access this object.
            //
            if(interval > 0)
            {                
                Debug.Assert(os__ != null);
                os__.instance().retryQueue().add(this, interval);
            }
            else
            {
                send__();
            }
        }

        public bool send__()
        {
            try
            {
                _sent = false;
                _response = false;
                _delegate = _proxy.getDelegate__(true);
                _sentSynchronously = _delegate.getRequestHandler__().sendAsyncRequest(this);
            }
            catch(LocalExceptionWrapper ex)
            {
                handleException(ex);
            }
            catch(Ice.LocalException ex)
            {
                handleException(ex);
            }
            return _sentSynchronously;
        }

        protected void prepare__(Ice.ObjectPrx prx, string operation, Ice.OperationMode mode,
                                 Dictionary<string, string> context)
        {
            Debug.Assert(os__ != null);

            _proxy = (Ice.ObjectPrxHelperBase)prx;
            _delegate = null;
            _cnt = 0;
            _mode = mode;
            _sentSynchronously = false;

            //
            // Can't call async via a batch proxy.
            //
            if(_proxy.ice_isBatchOneway() || _proxy.ice_isBatchDatagram())
            {
                throw new Ice.FeatureNotSupportedException("can't send batch requests with AMI");
            }

            Reference rf = _proxy.reference__();

            os__.writeBlob(IceInternal.Protocol.requestHdr);

            rf.getIdentity().write__(os__);

            //
            // For compatibility with the old FacetPath.
            //
            string facet = rf.getFacet();
            if(facet == null || facet.Length == 0)
            {
                os__.writeStringSeq(null);
            }
            else
            {
                string[] facetPath = { facet };
                os__.writeStringSeq(facetPath);
            }

            os__.writeString(operation);

            os__.writeByte((byte)mode);

            if(context != null)
            {
                //
                // Explicit context
                //
                Ice.ContextHelper.write(os__, context);
            }
            else
            {
                //
                // Implicit context
                //
                Ice.ImplicitContextI implicitContext = rf.getInstance().getImplicitContext();
                Dictionary<string, string> prxContext = rf.getContext();

                if(implicitContext == null)
                {
                    Ice.ContextHelper.write(os__, prxContext);
                }
                else
                {
                    implicitContext.write(prxContext, os__);
                }
            }

            os__.startWriteEncaps();
        }

        protected abstract void response__(bool ok);

        protected void throwUserException__()
        {
            try
            {
                is__.startReadEncaps();
                is__.throwException();
            }
            catch(Ice.UserException)
            {
                is__.endReadEncaps();
                throw;
            }
        }

        private void handleException(LocalExceptionWrapper ex)
        {
            if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent)
            {
                _proxy.handleExceptionWrapperRelaxed__(_delegate, ex, this, ref _cnt);
            }
            else
            {
                _proxy.handleExceptionWrapper__(_delegate, ex, this);
            }
        }

        private void handleException(Ice.LocalException exc)
        {
            try
            {
                //
                // A CloseConnectionException indicates graceful server shutdown, and is therefore
                // always repeatable without violating "at-most-once". That's because by sending a
                // close connection message, the server guarantees that all outstanding requests
                // can safely be repeated.
                //
                // An ObjectNotExistException can always be retried as well without violating 
                // "at-most-once" (see the implementation of the checkRetryAfterException method of
                // the ProxyFactory class for the reasons why it can be useful).
                // 
                if(!_sent || exc is Ice.CloseConnectionException || exc is Ice.ObjectNotExistException)
                {
                    throw exc;
                }
                
                //
                // Throw the exception wrapped in a LocalExceptionWrapper, to indicate that the 
                // request cannot be resent without potentially violating the "at-most-once" 
                // principle.
                //
                throw new LocalExceptionWrapper(exc, false);
            }
            catch(LocalExceptionWrapper ex)
            {
                if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent)
                {
                    _proxy.handleExceptionWrapperRelaxed__(_delegate, ex, this, ref _cnt);
                }
                else
                {
                    _proxy.handleExceptionWrapper__(_delegate, ex, this);
                }
            }
            catch(Ice.LocalException ex)
            {
                _proxy.handleException__(_delegate, ex, this, ref _cnt);
            }
        }

        private void runTimerTask__(Ice.ConnectionI connection)
        {
            lock(monitor__)
            {
                Debug.Assert(_timerTask != null && _sent); // Can only be set once the request is sent.

                if(_response) // If the response was just received, don't close the connection.
                {
                    connection = null;
                }
                _timerTask = null;
                Monitor.PulseAll(monitor__);
            }

            if(connection != null)
            {
                connection.exception(new Ice.TimeoutException());
            }
        }

        private bool _sent;
        private bool _sentSynchronously;
        private bool _response;
        private Ice.ObjectPrxHelperBase _proxy;
        private Ice.ObjectDel_ _delegate;
        private int _cnt;
        private Ice.OperationMode _mode;
        private TimerTask _timerTask;
    }

    public abstract class BatchOutgoingAsync : OutgoingAsyncMessageCallback
    {
        public override void sent__(Ice.ConnectionI connection)
        {
            releaseCallback__();
        }

        public override void finished__(Ice.LocalException exc)
        {
            exception__(exc);
        }
    }

}

namespace Ice
{
    using System.Collections.Generic;

    /// <summary>
    /// Callback interface for AMI invocations. This is useful
    /// for applications that send asynchronous invocations that
    /// might not be sent for some time and, therefore, are
    /// queued in the Ice run time for transmission.
    /// The Ice run time calls ice_sent if an
    /// an AMI invocation cannot be written to transport and was
    /// queued for later transmission. Applications that need to
    /// implement flow control for AMI invocations can implement
    /// this interface on the corresponding AMI callback object.
    /// </summary>
    public interface AMISentCallback
    {
        /// <summary>
        /// Indicates to the caller of an AMI operation that
        /// the invocation was queued for later transmission.
        /// </summary>
        void ice_sent();
    };

    /// <summary>
    /// Callback object for Blobject AMI invocations.
    /// </summary>
    public abstract class AMI_Object_ice_invoke : IceInternal.OutgoingAsync
    {
         /// <summary>
         /// The Ice run time calls <code>ice_response</code> when an asynchronous operation invocation
         /// completes successfully or raises a user exception.
         /// </summary>
         /// <param name="ok">Indicates the result of the invocation. If true, the operation
         /// completed succesfully; if false, the operation raised a user exception.</param>
         /// <param name="outParams">Contains the encoded out-parameters of the operation (if any) if ok
         /// is true; otherwise, if ok is false, contains the
         /// encoded user exception raised by the operation.</param>
        public abstract void ice_response(bool ok, byte[] outParams);

        /// <summary>
        /// The Ice run time calls ice_exception when an asynchronous operation invocation
        /// raises an Ice run-time exception.
        /// </summary>
        /// <param name="ex">The encoded Ice run-time exception raised by the operation.</param>
        public abstract override void ice_exception(Ice.Exception ex);

        public bool invoke__(Ice.ObjectPrx prx, string operation, OperationMode mode,
            byte[] inParams, Dictionary<string, string> context)
        {
            acquireCallback__(prx);
            try
            {
                prepare__(prx, operation, mode, context);
                os__.writeBlob(inParams);
                os__.endWriteEncaps();
                return send__();
            }
            catch(LocalException ex)
            {
                releaseCallback__(ex);
                return false;
            }
        }

        protected override void response__(bool ok) // ok == true means no user exception.
        {
            byte[] outParams;
            try
            {
                is__.startReadEncaps();
                int sz = is__.getReadEncapsSize();
                outParams = is__.readBlob(sz);
                is__.endReadEncaps();
            }
            catch(LocalException ex)
            {
                finished__(ex);
                return;
            }
            ice_response(ok, outParams);
            releaseCallback__();
        }
    }

    /// <summary>
    /// Callback object for ObjectPrx.ice_flushBatchRequests_async.
    /// </summary>
    public abstract class AMI_Object_ice_flushBatchRequests : IceInternal.BatchOutgoingAsync
    {
        /// <summary>
        /// Indicates to the caller that a call to <code>ice_flushBatchRequests_async</code>
        /// raised an Ice run-time exception.
        /// </summary>
        /// <param name="ex">The run-time exception that was raised.</param>
        public abstract override void ice_exception(Ice.Exception ex);

        public bool invoke__(Ice.ObjectPrx prx)
        {
            acquireCallback__(prx);
            try
            {
                //
                // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
                // requests were queued with the connection, they would be lost without being noticed.
                //
                Ice.ObjectDel_ @delegate = null;
                int cnt = -1; // Don't retry.
                Ice.ObjectPrxHelperBase proxy = (Ice.ObjectPrxHelperBase)prx;
                try
                {
                    @delegate = proxy.getDelegate__(true);
                    return @delegate.getRequestHandler__().flushAsyncBatchRequests(this);
                }
                catch(Ice.LocalException ex)
                {
                    proxy.handleException__(@delegate, ex, null, ref cnt);
                }
            }
            catch(Ice.LocalException ex)
            {
                releaseCallback__(ex);
            }
            return false;
        }
    }
}
