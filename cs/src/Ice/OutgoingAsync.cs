// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System.Threading;
    using System.Collections.Generic;
    using System.Diagnostics;

    ///
    /// <summary>
    /// Callback that requires the application to down-cast the proxy.
    /// </summary>
    ///
    public delegate void AsyncCallback(AsyncResult r);

    ///
    /// <summary>
    /// Callback for the successful completion of an operation
    /// that returns no data.
    /// </summary>
    ///
    public delegate void OnewayCallback();

    ///
    /// <summary>
    /// Callback for the successful completion of an operation
    /// that returns no data.
    /// </summary>
    ///
    public delegate void SentCallback(bool sentSynchronously);

    ///
    /// <summary>
    /// Called when an invocation raises an exception.
    /// </summary>
    ///
    public delegate void ExceptionCallback(Ice.Exception ex);

    ///
    /// <summary>
    /// <!-- TODO -->
    /// </summary>
    public interface AsyncResult : System.IAsyncResult
    {
        Ice.Communicator getCommunicator();

        Ice.Connection getConnection();

        ObjectPrx getProxy();

        bool isCompleted_();
        void waitForCompleted();

        bool isSent();
        void waitForSent();

        bool sentSynchronously();

        string getOperation();

        AsyncResult whenSent(Ice.AsyncCallback cb);
        AsyncResult whenSent(Ice.SentCallback cb);

        AsyncResult whenCompleted(Ice.ExceptionCallback excb);
    }

    public interface AsyncResult<T> : AsyncResult
    {
        AsyncResult<T> whenCompleted(T cb, Ice.ExceptionCallback excb);

        new AsyncResult<T> whenCompleted(Ice.ExceptionCallback excb);
        new AsyncResult<T> whenSent(Ice.SentCallback cb);
    }
}

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public delegate void ProxyTwowayCallback<T>(Ice.AsyncResult result, T cb, Ice.ExceptionCallback excb);
    public delegate void ProxyOnewayCallback<T>(T cb);

    //
    // This interface is used by the connection to handle OutgoingAsync
    // and BatchOutgoingAsync messages.
    //
    public interface OutgoingAsyncMessageCallback
    {
        //
        // Called by the connection when the message is confirmed sent. The connection is locked
        // when this is called so this method can call the sent callback. Instead, this method
        // returns true if there's a sent callback and false otherwise. If true is returned, the
        // connection will call the __sent() method bellow (which in turn should call the sent
        // callback).
        //
        Ice.AsyncCallback sent__(Ice.ConnectionI connection);

        //
        // Called by the connection to call the user sent callback.
        //
        void sent__(Ice.AsyncCallback cb);

        //
        // Called by the connection when the request failed. The boolean indicates whether or
        // not the message was possibly sent (this is useful for retry to figure out whether
        // or not the request can't be retried without breaking at-most-once semantics.)
        //
        void finished__(Ice.LocalException ex, bool sent);
    }

    abstract public class OutgoingAsyncBase : Ice.AsyncResult
    {
        public virtual Ice.Communicator getCommunicator()
        {
            return null;
        }

        public virtual Ice.Connection getConnection()
        {
            return null;
        }

        public virtual Ice.ObjectPrx getProxy()
        {
            return null;
        }

        public bool isCompleted_()
        {
            monitor_.Lock();
            try
            {
                return (state_ & Done) != 0;
            }
            finally
            {
                monitor_.Unlock();
            }
        }

        public void waitForCompleted()
        {
            monitor_.Lock();
            try
            {
                while((state_ & Done) == 0)
                {
                    monitor_.Wait();
                }
            }
            finally
            {
                monitor_.Unlock();
            }
        }

        public bool isSent()
        {
            monitor_.Lock();
            try
            {
                return (state_ & Sent) != 0;
            }
            finally
            {
                monitor_.Unlock();
            }
        }

        public void waitForSent()
        {
            monitor_.Lock();
            try
            {
                while((state_ & (Sent | Done)) == 0)
                {
                    monitor_.Wait();
                }
            }
            finally
            {
                monitor_.Unlock();
            }
        }

        public void throwLocalException()
        {
            lock(monitor_)
            {
                if(exception_ != null)
                {
                    throw exception_;
                }
            }
        }

        public bool sentSynchronously()
        {
            return sentSynchronously_; // No lock needed, immutable once send__() is called
        }

        //
        // Implementation of System.IAsyncResult properties
        //

        public bool IsCompleted
        {
            get
            {
                return isCompleted_();
            }
        }

        public bool CompletedSynchronously
        {
            get
            {
                if(getProxy() != null && getProxy().ice_isTwoway())
                {
                    return false;
                }
                return sentSynchronously_;
            }
        }

        public object AsyncState
        {
            get
            {
                return _cookie; // No lock needed, cookie is immutable.
            }
        }

        public WaitHandle AsyncWaitHandle
        {
            get
            {
                monitor_.Lock();
                try
                {
                    if(waitHandle_ == null)
                    {
                        waitHandle_ = new EventWaitHandle(false, EventResetMode.ManualReset);
                    }
                    if((state_ & Done) != 0)
                    {
                        waitHandle_.Set();
                    }
                    return waitHandle_;
                }
                finally
                {
                    monitor_.Unlock();
                }
            }
        }        

        public Ice.AsyncResult whenSent(Ice.AsyncCallback cb)
        {
            monitor_.Lock();
            try
            {
                if(cb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                if(sentCallback_ != null)
                {
                    throw new System.ArgumentException("sent callback already set");
                }
                sentCallback_ = cb;
                if((state_ & Sent) == 0)
                {
                    return this;
                }
            }
            finally
            {
                monitor_.Unlock();
            }

            if(sentSynchronously_)
            {
                try
                {
                    sentCallback_(this);
                }
                catch(System.Exception ex)
                {
                    warning__(ex);
                }
            }
            else
            {
                instance_.clientThreadPool().dispatch(delegate()
                                                      {
                                                          try
                                                          {
                                                              sentCallback_(this);
                                                          }
                                                          catch(System.Exception ex)
                                                          {
                                                              warning__(ex);
                                                          }
                                                      });
            }
            return this;
        }

        public Ice.AsyncResult whenSent(Ice.SentCallback cb)
        {
            monitor_.Lock();
            try
            {
                if(cb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                if(sentCallback_ != null)
                {
                    throw new System.ArgumentException("sent callback already set");
                }
                sentCallback_ = delegate(Ice.AsyncResult result) 
                                {
                                    cb(result.sentSynchronously());                        
                                };
                if((state_ & Sent) == 0)
                {
                    return this;
                }
            }
            finally
            {
                monitor_.Unlock();
            }

            if(sentSynchronously_)
            {
                try
                {
                    cb(true);
                }
                catch(System.Exception ex)
                {
                    warning__(ex);
                }
            }
            else
            {
                instance_.clientThreadPool().dispatch(delegate()
                                                      {
                                                          try
                                                          {
                                                              cb(false);
                                                          }
                                                          catch(System.Exception ex)
                                                          {
                                                              warning__(ex);
                                                          }
                                                      });
            }
            return this;
        }

        public Ice.AsyncResult whenCompletedWithAsyncCallback(Ice.AsyncCallback cb)
        {
            monitor_.Lock();
            try
            {
                setCompletedCallback(cb);
                if((state_ & Done) == 0)
                {
                    return this;
                }
                else if((getProxy() == null || getProxy().ice_isOneway()) && exception_ == null)
                {
                    return this;
                }
            }
            finally
            {
                monitor_.Unlock();
            }

            instance_.clientThreadPool().dispatch(delegate()
                                                  {
                                                      try
                                                      {
                                                          cb(this);
                                                      }
                                                      catch(System.Exception ex)
                                                      {
                                                          warning__(ex);
                                                      }
                                                  });
            return this;
        }

        public Ice.AsyncResult whenCompleted(Ice.ExceptionCallback cb)
        {
            monitor_.Lock();
            try
            {
                if(cb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                setCompletedCallback(getCompletedCallback());
                exceptionCallback_ = cb;
                if((state_ & Done) == 0)
                {
                    return this;
                }
                else if((getProxy() == null || getProxy().ice_isOneway()) && exception_ == null)
                {
                    return this;
                }
            }
            finally
            {
                monitor_.Unlock();
            }

            instance_.clientThreadPool().dispatch(delegate()
                                                  {
                                                      try
                                                      {
                                                          completedCallback_(this);
                                                      }
                                                      catch(System.Exception ex)
                                                      {
                                                          warning__(ex);
                                                      }
                                                  });
            return this;
        }

        public string getOperation()
        {
            return operation_;
        }

        public IceInternal.BasicStream ostr__
        {
            get
            {
                return os_;
            }
        }

        public IceInternal.BasicStream istr__
        {
            get
            {
                return is_;
            }
        }

        public bool wait__()
        {
            monitor_.Lock();
            try
            {
                if((state_ & EndCalled) != 0)
                {
                    throw new System.ArgumentException("end_ method called more than once");
                }
                state_ |= EndCalled;
                while((state_ & Done) == 0)
                {
                    monitor_.Wait();
                }
                if(exception_ != null)
                {
                    throw exception_;
                }
                return (state_ & OK) != 0;
            }
            finally
            {
                monitor_.Unlock();
            }
        }

        public void throwUserException__()
        {
            try
            {
                is_.startReadEncaps();
                is_.throwException();
            }
            catch(Ice.UserException)
            {
                is_.endReadEncaps();
                throw;
            }
        }

        public void exceptionAsync__(Ice.Exception ex)
        {
            //
            // This is called when it's not safe to call the exception callback synchronously
            // from this thread. Instead the exception callback is called asynchronously from
            // the client thread pool.
            //
            try
            {
                instance_.clientThreadPool().dispatch(delegate()
                                                      {
                                                          exception__(ex);
                                                      });
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                throw; // CommunicatorDestroyedException is the only exception that can propagate directly.
            }
        }

        public void sentAsync__(Ice.AsyncCallback callback)
        {
            //
            // This is called when it's not safe to call the exception callback synchronously
            // from this thread. Instead the exception callback is called asynchronously from
            // the client thread pool.
            //
            if(callback != null)
            {
                try
                {
                    instance_.clientThreadPool().dispatch(delegate()
                                                          {
                                                              sent__(callback);
                                                          });
                }
                catch(Ice.CommunicatorDestroyedException)
                {
                }
            }
        }

        public static void check__(OutgoingAsyncBase r, Ice.ObjectPrx prx, string operation)
        {
            check__(r, operation);
            if(r.getProxy() != prx)
            {
                throw new System.ArgumentException("Proxy for call to end_" + operation +
                                                   " does not match proxy that was used to call corresponding begin_" +
                                                   operation + " method");
            }
        }

        public static void check__(OutgoingAsyncBase r, Ice.Connection con, string operation)
        {
            check__(r, operation);
            if(r.getConnection() != con)
            {
                throw new System.ArgumentException("Connection for call to end_" + operation +
                                                   " does not match connection that was used to call " +
                                                   "corresponding begin_" + operation + " method");
            }
        }

        public static void check__(OutgoingAsyncBase r, Ice.Communicator com, string operation)
        {
            check__(r, operation);
            if(r.getCommunicator() != com)
            {
                throw new System.ArgumentException("Communicator for call to end_" + operation +
                                                   " does not match communicator that was used to call " +
                                                   "corresponding begin_" + operation + " method");
            }
        }

        protected static void check__(OutgoingAsyncBase r, string operation)
        {
            if(r == null)
            {
                throw new System.ArgumentException("AsyncResult == null");
            }
            if(r.getOperation() != operation)
            {
                throw new System.ArgumentException("Incorrect operation for end_" + operation + " method: " +
                                                   r.getOperation());
            }
        }

        protected OutgoingAsyncBase(IceInternal.Instance instance, string op, object cookie)
        {
            instance_ = instance;
            operation_ = op;
            is_ = new IceInternal.BasicStream(instance);
            os_ = new IceInternal.BasicStream(instance);
            state_ = 0;
            exception_ = null;
            _cookie = cookie;
        }

        protected void setCompletedCallback(Ice.AsyncCallback cb)
        {
            if(cb == null)
            {
                throw new System.ArgumentException("callback is null");
            }
            if(completedCallback_ != null)
            {
                throw new System.ArgumentException("completed callback already set");
            }
            completedCallback_ = cb;
        }

        protected void sent__(Ice.AsyncCallback cb)
        {
            //
            // Note: no need to change the state_ here, specializations are responsible for
            // changing the state.
            //

            if(cb != null)
            {
                try
                {
                    cb(this);
                }
                catch(System.Exception ex)
                {
                    warning__(ex);
                }
            }
        }

        protected void response__(Ice.AsyncCallback cb)
        {
            //
            // Note: no need to change the state_ here, specializations are responsible for
            // changing the state.
            //

            if(cb != null)
            {
                try
                {
                    cb(this);
                }
                catch(System.Exception ex)
                {
                    warning__(ex);
                }
            }
        }

        protected void exception__(Ice.Exception ex)
        {
            Ice.AsyncCallback cb;
            monitor_.Lock();
            try
            {
                state_ |= Done;
                exception_ = ex;
                monitor_.NotifyAll();
                if(waitHandle_ != null)
                {
                    waitHandle_.Set();
                }
                cb = completedCallback_;
            }
            finally
            {
                monitor_.Unlock();
            }

            if(cb != null)
            {
                try
                {
                    cb(this);
                }
                catch(System.Exception exc)
                {
                    warning__(exc);
                }
            }
        }

        protected virtual Ice.AsyncCallback getCompletedCallback()
        {
            return completed__;
        }

        private void completed__(Ice.AsyncResult result)
        {
            Debug.Assert(exceptionCallback_ != null);
            try
            {
                ((OutgoingAsyncBase)result).wait__();
            }
            catch(Ice.Exception ex)
            {
                exceptionCallback_(ex);
                return;
            }
        }

        protected void warning__(System.Exception ex)
        {
            if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
            {
                instance_.initializationData().logger.warning("exception raised by AMI callback:\n" + ex);
            }
        }

        protected IceInternal.Instance instance_;
        protected string operation_;

        protected readonly IceUtilInternal.Monitor monitor_ = new IceUtilInternal.Monitor();
        protected IceInternal.BasicStream is_;
        protected IceInternal.BasicStream os_;

        protected const int OK = 0x1;
        protected const int Done = 0x2;
        protected const int Sent = 0x4;
        protected const int EndCalled = 0x8;

        protected int state_;
        protected bool sentSynchronously_;
        protected Exception exception_;
        protected EventWaitHandle waitHandle_;

        protected Ice.AsyncCallback completedCallback_;
        protected Ice.AsyncCallback sentCallback_;
        protected Ice.ExceptionCallback exceptionCallback_;

        private object _cookie;
    }

    abstract public class OutgoingAsync : OutgoingAsyncBase, OutgoingAsyncMessageCallback
    {
        public OutgoingAsync(Ice.ObjectPrx prx, string operation, object cookie) :
            base(((Ice.ObjectPrxHelperBase)prx).reference__().getInstance(), operation, cookie)
        {
            proxy_ = (Ice.ObjectPrxHelperBase)prx;
        }

        public void prepare__(string operation, Ice.OperationMode mode, Dictionary<string, string> context,
                              bool explicitContext)
        {
            _delegate = null;
            _cnt = 0;
            _mode = mode;
            sentSynchronously_ = false;

            if(explicitContext && context == null)
            {
                context = emptyContext_;
            }

            //
            // Can't call async via a batch proxy.
            //
            if(proxy_.ice_isBatchOneway() || proxy_.ice_isBatchDatagram())
            {
                throw new Ice.FeatureNotSupportedException("can't send batch requests with AMI");
            }

            os_.writeBlob(Protocol.requestHdr);

            Reference rf = proxy_.reference__();

            rf.getIdentity().write__(os_);

            //
            // For compatibility with the old FacetPath.
            //
            string facet = rf.getFacet();
            if(facet == null || facet.Length == 0)
            {
                os_.writeStringSeq(null);
            }
            else
            {
                string[] facetPath = { facet };
                os_.writeStringSeq(facetPath);
            }

            os_.writeString(operation);

            os_.writeByte((byte)mode);

            if(context != null)
            {
                //
                // Explicit context
                //
                Ice.ContextHelper.write(os_, context);
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
                    Ice.ContextHelper.write(os_, prxContext);
                }
                else
                {
                    implicitContext.write(prxContext, os_);
                }
            }

            os_.startWriteEncaps();
        }

        public override Ice.ObjectPrx getProxy()
        {
            return proxy_;
        }

        public Ice.AsyncCallback sent__(Ice.ConnectionI connection)
        {
            monitor_.Lock();
            try
            {
                bool alreadySent = (state_ & Sent) != 0;
                state_ |= Sent;

                //
                // It's possible for the request to be done already when using IOCP. This 
                // is the case for example if the send callback is dispatched after the 
                // read callback for the response/exception.
                //
                if((state_ & Done) == 0)
                {
                    if(!proxy_.ice_isTwoway())
                    {
                        state_ |= Done | OK;
                        if(waitHandle_ != null)
                        {
                            waitHandle_.Set();
                        }
                    }
                    else if(connection.timeout() > 0)
                    {
                        Debug.Assert(_timerTaskConnection == null && _timerTask == null);
                        _timerTaskConnection = connection;
                        _timerTask = new TaskI(this);
                        proxy_.reference__().getInstance().timer().schedule(_timerTask, connection.timeout());
                    }
                }
                monitor_.NotifyAll();
                return alreadySent ? null : sentCallback_; // Don't call the sent call is already sent.
            }
            finally
            {
                monitor_.Unlock();
            }
        }

        public new void sent__(Ice.AsyncCallback cb)
        {
            base.sent__(cb);
        }

        public void finished__(Ice.LocalException exc, bool sent)
        {
            monitor_.Lock();
            try
            {
                Debug.Assert((state_ & Done) == 0);
                if(_timerTaskConnection != null)
                {
                    Debug.Assert(_timerTask != null);
                    instance_.timer().cancel(_timerTask);
                    _timerTaskConnection = null; // Timer cancelled.
                    _timerTask = null;
                }
            }
            finally
            {
                monitor_.Unlock();
            }

            //
            // NOTE: at this point, synchronization isn't needed, no other threads should be
            // calling on the callback.
            //

            try
            {
                int interval = handleException(exc, sent); // This will throw if the invocation can't be retried.
                if(interval > 0)
                {
                    instance_.retryQueue().add(this, interval); 
                }
                else
                {
                    send__(false);
                }
            }
            catch(Ice.LocalException ex)
            {
                exception__(ex);
            }
        }

        public void finished__(LocalExceptionWrapper exc)
        {
            //
            // NOTE: at this point, synchronization isn't needed, no other threads should be
            // calling on the callback. The LocalExceptionWrapper exception is only called
            // before the invocation is sent.
            //

            try
            {
                int interval = handleException(exc); // This will throw if the invocation can't be retried.
                if(interval > 0)
                {
                    instance_.retryQueue().add(this, interval); 
                }
                else
                {
                    send__(false);
                }
            }
            catch(Ice.LocalException ex)
            {
                exception__(ex);
            }
        }

        public void finished__(BasicStream istr)
        {
            Debug.Assert(proxy_.ice_isTwoway()); // Can only be called for twoways.

            byte replyStatus;
            Ice.AsyncCallback cb = null;
            try
            {
                monitor_.Lock();
                try
                {
                    Debug.Assert(exception_ == null && (state_ & Done) == 0);

                    if(_timerTaskConnection != null)
                    {
                        Debug.Assert(_timerTask != null);
                        proxy_.reference__().getInstance().timer().cancel(_timerTask);
                        _timerTaskConnection = null; // Timer cancelled.
                        _timerTask = null;
                    }

                    is_.swap(istr);
                    replyStatus = is_.readByte();

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
                            id.read__(is_);

                            //
                            // For compatibility with the old FacetPath.
                            //
                            string[] facetPath = is_.readStringSeq();
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

                            string operation = is_.readString();

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
                            string unknown = is_.readString();

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

                    state_ |= Done;
                    if(waitHandle_ != null)
                    {
                        waitHandle_.Set();
                    }
                    if(replyStatus == ReplyStatus.replyOK)
                    {
                        state_ |= OK;
                    }
                    cb = completedCallback_;
                    monitor_.NotifyAll();
                }
                finally
                {
                    monitor_.Unlock();
                }
            }
            catch(Ice.LocalException ex)
            {
                finished__(ex, true);
                return;
            }

            Debug.Assert(replyStatus == ReplyStatus.replyOK || replyStatus == ReplyStatus.replyUserException);
            response__(cb);
        }

        public bool send__(bool synchronous)
        {
            while(true)
            {
                int interval = 0;
                try
                {
                    _delegate = proxy_.getDelegate__(true);
                    Ice.AsyncCallback sentCallback;
                    if(_delegate.getRequestHandler__().sendAsyncRequest(this, out sentCallback))
                    {
                        if(synchronous) // Only set sentSynchronously_ If called synchronously by the user thread.
                        {
                            sentSynchronously_ = true;
                            sent__(sentCallback);
                        }
                        else
                        {
                            sentAsync__(sentCallback);
                        }
                    }
                    break;
                }
                catch(LocalExceptionWrapper ex)
                {
                    interval = handleException(ex);
                }
                catch(Ice.LocalException ex)
                {
                    interval = handleException(ex, false);
                }
                
                if(interval > 0)
                {
                    instance_.retryQueue().add(this, interval);
                    return false;
                }
            }
            return sentSynchronously_;
        }

        private int handleException(LocalExceptionWrapper ex)
        {
            if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent)
            {
                return proxy_.handleExceptionWrapperRelaxed__(_delegate, ex, false, ref _cnt);
            }
            else
            {
                return proxy_.handleExceptionWrapper__(_delegate, ex);
            }
        }

        private int handleException(Ice.LocalException exc, bool sent)
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
                if(!sent || exc is Ice.CloseConnectionException || exc is Ice.ObjectNotExistException)
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
                    return proxy_.handleExceptionWrapperRelaxed__(_delegate, ex, false, ref _cnt);
                }
                else
                {
                    return proxy_.handleExceptionWrapper__(_delegate, ex);
                }
            }
            catch(Ice.LocalException ex)
            {
                return proxy_.handleException__(_delegate, ex, false, ref _cnt);
            }
        }

        private class TaskI : TimerTask
        {
            internal TaskI(OutgoingAsync @out)
            {
                _out = @out;
            }

            public void runTimerTask()
            {
                _out.runTimerTask__();
            }

            private OutgoingAsync _out;
        }

        private void runTimerTask__()
        {
            Ice.ConnectionI connection = null;
            monitor_.Lock();
            try
            {
                connection = _timerTaskConnection;
                _timerTaskConnection = null;
                _timerTask = null;
            }
            finally
            {
                monitor_.Unlock();
            }

            if(connection != null)
            {
                connection.exception(new Ice.TimeoutException());
            }
        }

        protected Ice.ObjectPrxHelperBase proxy_;

        private TimerTask _timerTask;
        private Ice.ConnectionI _timerTaskConnection;

        private Ice.ObjectDel_ _delegate;
        private int _cnt;
        private Ice.OperationMode _mode;

        private static Dictionary<string, string> emptyContext_ = new Dictionary<string, string>();
    }

    abstract public class OutgoingAsync<T> : OutgoingAsync, Ice.AsyncResult<T>
    {
        public OutgoingAsync(Ice.ObjectPrx prx, string operation, object cookie) :
            base(prx, operation, cookie)
        {
        }

        new public Ice.AsyncResult<T> whenCompleted(Ice.ExceptionCallback excb)
        {
            monitor_.Lock();
            try
            {
                if(excb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                setCompletedCallback(getCompletedCallback());
                exceptionCallback_ = excb;
                if((state_ & Done) == 0)
                {
                    return this;
                }
                else if((getProxy() == null || getProxy().ice_isOneway()) && exception_ == null)
                {
                    return this;
                }
            }
            finally
            {
                monitor_.Unlock();
            }

            instance_.clientThreadPool().dispatch(delegate()
                                                 {
                                                     try
                                                     {
                                                         completedCallback_(this);
                                                     }
                                                     catch(System.Exception ex)
                                                     {
                                                         warning__(ex);
                                                     }
                                                 });
            return this;
        }

        virtual public Ice.AsyncResult<T> whenCompleted(T cb, Ice.ExceptionCallback excb)
        {
            monitor_.Lock();
            try
            {
                if(cb == null && excb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                setCompletedCallback(getCompletedCallback());
                responseCallback_ = cb;
                exceptionCallback_ = excb;
                if((state_ & Done) == 0)
                {
                    return this;
                }
                else if((getProxy() == null || getProxy().ice_isOneway()) && exception_ == null)
                {
                    return this;
                }
            }
            finally
            {
                monitor_.Unlock();
            }

            instance_.clientThreadPool().dispatch(delegate()
                                                 {
                                                     try
                                                     {
                                                         completedCallback_(this);
                                                     }
                                                     catch(System.Exception ex)
                                                     {
                                                         warning__(ex);
                                                     }
                                                 });
            return this;
        }

        new public Ice.AsyncResult<T> whenSent(Ice.SentCallback cb)
        {
            base.whenSent(cb);
            return this;
        }

        protected T responseCallback_;
    };

    public class TwowayOutgoingAsync<T> : OutgoingAsync<T>
    {
        public TwowayOutgoingAsync(Ice.ObjectPrx prx, string operation, ProxyTwowayCallback<T> cb, object cookie) :
            base(prx, operation, cookie)
        {
            Debug.Assert(cb != null);
            _completed = cb;
        }

        override protected Ice.AsyncCallback getCompletedCallback()
        {
            return completed__;
        }

        private void completed__(Ice.AsyncResult result)
        {
            _completed(this, responseCallback_, exceptionCallback_);
        }
        
        private ProxyTwowayCallback<T> _completed;
    };

    public class OnewayOutgoingAsync<T> : OutgoingAsync<T>
    {
        public OnewayOutgoingAsync(Ice.ObjectPrx prx, string operation, ProxyOnewayCallback<T> cb, object cookie) :
            base(prx, operation, cookie)
        {
            Debug.Assert(cb != null);
            _completed = cb;
        }
        
        override protected Ice.AsyncCallback getCompletedCallback()
        {
            return completed__;
        }

        protected void completed__(Ice.AsyncResult r__)
        {
            try
            {
                IceInternal.OutgoingAsync outAsync__ = (IceInternal.OutgoingAsync)r__;
                ((Ice.ObjectPrxHelperBase)(outAsync__.getProxy())).end__(outAsync__, outAsync__.getOperation());
            }
            catch(Ice.Exception ex__)
            {
                if(exceptionCallback_ != null)
                {
                    exceptionCallback_(ex__);
                }
                return;
            }
            _completed(responseCallback_);
        }

        private ProxyOnewayCallback<T> _completed;
    }

    public class BatchOutgoingAsync : OutgoingAsyncBase, OutgoingAsyncMessageCallback
    {
        public BatchOutgoingAsync(Instance instance, string operation, object cookie) :
            base(instance, operation, cookie)
        {
        }

        public Ice.AsyncCallback sent__(Ice.ConnectionI connection)
        {
            monitor_.Lock();
            try
            {
                Debug.Assert((state_ & (Done | OK | Sent)) == 0);
                state_ |= (Done | OK | Sent);
                monitor_.NotifyAll();
                if(waitHandle_ != null)
                {
                    waitHandle_.Set();
                }
                return sentCallback_;
            }
            finally
            {
                monitor_.Unlock();
            }
        }

        public new void sent__(Ice.AsyncCallback cb)
        {
            base.sent__(cb);
        }

        public void finished__(Ice.LocalException exc, bool sent)
        {
            exception__(exc);
        }
    }

    public class ProxyBatchOutgoingAsync : BatchOutgoingAsync
    {
        public ProxyBatchOutgoingAsync(Ice.ObjectPrx proxy, string operation, object cookie) :
            base(((Ice.ObjectPrxHelperBase)proxy).reference__().getInstance(), operation, cookie)
        {
            _proxy = proxy;
        }

        public void send__()
        {
            //
            // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
            // requests were queued with the connection, they would be lost without being noticed.
            //
            Ice.ObjectDel_ @delegate = null;
            int cnt = -1; // Don't retry.
            try
            {
                @delegate = ((Ice.ObjectPrxHelperBase)_proxy).getDelegate__(false);
                Ice.AsyncCallback sentCallback;
                if(@delegate.getRequestHandler__().flushAsyncBatchRequests(this, out sentCallback))
                {
                    sentSynchronously_ = true;
                    if(sentCallback != null)
                    {
                        sent__(sentCallback);
                    }
                }
            }
            catch(Ice.LocalException __ex)
            {
                ((Ice.ObjectPrxHelperBase)_proxy).handleException__(@delegate, __ex, false, ref cnt);
            }
        }

        public override Ice.ObjectPrx getProxy()
        {
            return _proxy;
        }

        private Ice.ObjectPrx _proxy;
    }

    public class ConnectionBatchOutgoingAsync : BatchOutgoingAsync
    {
        public ConnectionBatchOutgoingAsync(Ice.ConnectionI con, Instance instance, string operation, object cookie) :
            base(instance, operation, cookie)
        {
            _connection = con;
        }

        public void send__()
        {
            Ice.AsyncCallback sentCallback;
            if(_connection.flushAsyncBatchRequests(this, out sentCallback))
            {
                sentSynchronously_ = true;
                sent__(sentCallback);
            }
        }

        public override Ice.Connection getConnection()
        {
            return _connection;
        }

        private Ice.ConnectionI _connection;
    }

    public class CommunicatorBatchOutgoingAsync : BatchOutgoingAsync
    {
        public CommunicatorBatchOutgoingAsync(Ice.Communicator communicator, Instance instance, String operation,
                                              object cookie) :
            base(instance, operation, cookie)
        {
            _communicator = communicator;

            //
            // _useCount is initialized to 1 to prevent premature callbacks.
            // The caller must invoke ready() after all flush requests have
            // been initiated.
            //
            _useCount = 1;

            //
            // Assume all connections are flushed synchronously.
            //
            sentSynchronously_ = true;
        }

        public override Ice.Communicator getCommunicator()
        {
            return _communicator;
        }

        public void flushConnection(Ice.Connection con)
        {
            monitor_.Lock();
            try
            {
                ++_useCount;
            }
            finally
            {
                monitor_.Unlock();
            }
            Ice.AsyncResult r = con.begin_flushBatchRequests(completed, null);
            r.whenSent((Ice.AsyncCallback)sent);
        }

        public void ready()
        {
            check(null, null, true);
        }

        private void completed(Ice.AsyncResult r)
        {
            Ice.Connection con = r.getConnection();
            Debug.Assert(con != null);

            try
            {
                con.end_flushBatchRequests(r);
                Debug.Assert(false); // completed() should only be called when an exception occurs.
            }
            catch(Ice.LocalException ex)
            {
                check(r, ex, false);
            }
        }

        private void sent(Ice.AsyncResult r)
        {
            check(r, null, r.sentSynchronously());
        }

        private void check(Ice.AsyncResult r, Ice.LocalException ex, bool userThread)
        {
            bool done = false;
            Ice.AsyncCallback sentCallback = null;

            monitor_.Lock();
            try
            {
                Debug.Assert(_useCount > 0);
                --_useCount;

                //
                // We report that the communicator flush request was sent synchronously
                // if all of the connection flush requests are sent synchronously.
                //
                if((r != null && !r.sentSynchronously()) || ex != null)
                {
                    sentSynchronously_ = false;
                }

                if(_useCount == 0)
                {
                    done = true;
                    state_ |= Done | OK | Sent;
                    sentCallback = sentCallback_;
                    monitor_.NotifyAll();
                }
            }
            finally
            {
                monitor_.Unlock();
            }

            if(done)
            {
                //
                // sentSynchronously_ is immutable here.
                //
                if(!sentSynchronously_ && userThread)
                {
                    sentAsync__(sentCallback);
                }
                else
                {
                    Debug.Assert(sentSynchronously_ == userThread); // sentSynchronously && !userThread is impossible.
                    sent__(sentCallback);
                }
            }
        }

        private Ice.Communicator _communicator;
        private int _useCount;
    }
}

namespace Ice
{

    /// <summary>
    /// Deprecated callback interface for AMI invocations. This is useful
    /// for applications that send asynchronous invocations that
    /// might not be sent for some time and, therefore, are
    /// queued in the Ice run time for transmission.
    /// The Ice run time calls ice_sent if an
    /// an AMI invocation cannot be written to the transport and was
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
    }

    public abstract class AMICallbackBase
    {
        /// <summary>
        /// The Ice run time calls ice_exception when an asynchronous operation invocation
        /// raises an Ice run-time exception.
        /// </summary>
        /// <param name="ex">The encoded Ice run-time exception raised by the operation.</param>
        public abstract void ice_exception(Ice.Exception ex);

        public void exception__(Ice.Exception ex)
        {
            ice_exception(ex);
        }

        public void sent__(Ice.AsyncResult result)
        {
            if(!result.sentSynchronously())
            {
                ((AMISentCallback)this).ice_sent();
            }
        }
    }
}
