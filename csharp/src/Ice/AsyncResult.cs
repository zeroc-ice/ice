// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
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
        void cancel();

        Ice.Communicator getCommunicator();

        Ice.Connection getConnection();

        ObjectPrx getProxy();

        bool isCompleted_();
        void waitForCompleted();

        bool isSent();
        void waitForSent();

        void throwLocalException();

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
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public delegate void ProxyTwowayCallback<T>(Ice.AsyncResult result, T cb, Ice.ExceptionCallback excb);
    public delegate void ProxyOnewayCallback<T>(T cb);

    public class AsyncResultI : Ice.AsyncResult
    {
        public virtual void cancel()
        {
            cancel(new Ice.InvocationCanceledException());
        }

        public Ice.Communicator getCommunicator()
        {
            return _communicator;
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
            lock(this)
            {
                return (state_ & StateDone) != 0;
            }
        }

        public void waitForCompleted()
        {
            lock(this)
            {
                while((state_ & StateDone) == 0)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        public bool isSent()
        {
            lock(this)
            {
                return (state_ & StateSent) != 0;
            }
        }

        public void waitForSent()
        {
            lock(this)
            {
                while((state_ & StateSent) == 0 && _exception == null)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        public void throwLocalException()
        {
            lock(this)
            {
                if(_exception != null)
                {
                    throw _exception;
                }
            }
        }

        public bool sentSynchronously()
        {
            return sentSynchronously_; // No lock needed
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
                lock(this)
                {
                    if(_waitHandle == null)
                    {
                        _waitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
                    }
                    if((state_ & StateDone) != 0)
                    {
                        _waitHandle.Set();
                    }
                    return _waitHandle;
                }
            }
        }

        public Ice.AsyncResult whenSent(Ice.AsyncCallback cb)
        {
            lock(this)
            {
                if(cb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                if(_sentCallback != null)
                {
                    throw new System.ArgumentException("sent callback already set");
                }
                _sentCallback = cb;
                if((state_ & StateSent) == 0)
                {
                    return this;
                }
            }

            if(sentSynchronously_)
            {
                try
                {
                    _sentCallback(this);
                }
                catch(System.Exception ex)
                {
                    warning(ex);
                }
            }
            else
            {
                instance_.clientThreadPool().dispatch(() =>
                {
                    try
                    {
                        _sentCallback(this);
                    }
                    catch(System.Exception ex)
                    {
                        warning(ex);
                    }
                }, cachedConnection_);
            }
            return this;
        }

        public Ice.AsyncResult whenSent(Ice.SentCallback cb)
        {
            lock(this)
            {
                if(cb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                if(_sentCallback != null)
                {
                    throw new System.ArgumentException("sent callback already set");
                }
                _sentCallback = (Ice.AsyncResult result) => 
                {
                    cb(result.sentSynchronously());
                };
                if((state_ & StateSent) == 0)
                {
                    return this;
                }
            }

            if(sentSynchronously_)
            {
                try
                {
                    cb(true);
                }
                catch(System.Exception ex)
                {
                    warning(ex);
                }
            }
            else
            {
                instance_.clientThreadPool().dispatch(() =>
                {
                    try
                    {
                        cb(false);
                    }
                    catch(System.Exception ex)
                    {
                        warning(ex);
                    }
                }, cachedConnection_);
            }
            return this;
        }

        public Ice.AsyncResult whenCompletedWithAsyncCallback(Ice.AsyncCallback cb)
        {
            setCompletedCallback(cb);
            return this;
        }

        public Ice.AsyncResult whenCompleted(Ice.ExceptionCallback cb)
        {
            if(cb == null)
            {
                throw new System.ArgumentException("callback is null");
            }
            lock(this)
            {
                if(exceptionCallback_ != null)
                {
                    throw new System.ArgumentException("callback already set");
                }
                exceptionCallback_ = cb;
            }
            setCompletedCallback(getCompletedCallback());
            return this;
        }

        public string getOperation()
        {
            return _operation;
        }

        public void invokeSent(Ice.AsyncCallback cb)
        {
            Debug.Assert(cb != null);
            try
            {
                cb(this);
            }
            catch(System.Exception ex)
            {
                warning(ex);
            }

            if(observer_ != null)
            {
                Ice.ObjectPrx proxy = getProxy();
                if(proxy == null || !proxy.ice_isTwoway())
                {
                    observer_.detach();
                    observer_ = null;
                }
            }
        }

        public void invokeSentAsync(Ice.AsyncCallback cb)
        {
            //
            // This is called when it's not safe to call the exception callback synchronously
            // from this thread. Instead the exception callback is called asynchronously from
            // the client thread pool.
            //
            Debug.Assert(cb != null);
            try
            {
                instance_.clientThreadPool().dispatch(() =>
                    {
                        invokeSent(cb);
                    }, cachedConnection_);
            }
            catch(Ice.CommunicatorDestroyedException)
            {
            }
        }

        public void invokeCompleted(Ice.AsyncCallback cb)
        {
            Debug.Assert(cb != null);
            try
            {
                cb(this);
            }
            catch(System.Exception ex)
            {
                warning(ex);
            }
            
            if(observer_ != null)
            {
                observer_.detach();
                observer_ = null;
            }
        }

        public void invokeCompletedAsync(Ice.AsyncCallback cb)
        {
            //
            // This is called when it's not safe to call the exception callback synchronously
            // from this thread. Instead the exception callback is called asynchronously from
            // the client thread pool.
            //
            Debug.Assert(cb != null);

            // CommunicatorDestroyedException is the only exception that can propagate directly.
            instance_.clientThreadPool().dispatch(() =>
                {
                    invokeCompleted(cb);
                }, cachedConnection_);
        }

        public virtual void cancelable(CancellationHandler handler)
        {
            lock(this)
            {
                if(_cancellationException != null)
                {
                    try
                    {
                        throw _cancellationException;
                    }
                    finally
                    {
                        _cancellationException = null;
                    }
                }
                _cancellationHandler = handler;
            }
        }

        public bool wait()
        {
            lock(this)
            {
                if((state_ & StateEndCalled) != 0)
                {
                    throw new System.ArgumentException("end_ method called more than once");
                }
                state_ |= StateEndCalled;
                while((state_ & StateDone) == 0)
                {
                    System.Threading.Monitor.Wait(this);
                }
                if(_exception != null)
                {
                    throw _exception;
                }
                return (state_ & StateOK) != 0;
            }
        }

        public virtual void cacheMessageBuffers()
        {
        }

        protected AsyncResultI(Ice.Communicator communicator, Instance instance, string op, object cookie)
        {
            instance_ = instance;
            sentSynchronously_ = false;
            state_ = 0;

            _communicator = communicator;
            _operation = op;
            _exception = null;
            _cookie = cookie;
        }

        protected Ice.AsyncCallback sent(bool done)
        {
            lock(this)
            {        
                Debug.Assert(_exception == null);

                bool alreadySent = (state_ & StateSent) != 0;
                state_ |= StateSent;
                if(done)
                {
                    state_ |= StateDone | StateOK;
                    _cancellationHandler = null;
                    if(observer_ != null && _sentCallback == null)
                    {
                        observer_.detach();
                        observer_ = null;
                    }

                    //
                    // For oneway requests after the data has been sent
                    // the buffers can be reused unless this is a
                    // collocated invocation. For collocated invocations
                    // the buffer won't be reused because it has already
                    // been marked as cached in invokeCollocated.
                    //
                    cacheMessageBuffers();
                }
                if(_waitHandle != null)
                {
                    _waitHandle.Set();
                }
                System.Threading.Monitor.PulseAll(this);
                return !alreadySent ? _sentCallback : null;
            }
        }

        protected Ice.AsyncCallback finished(bool ok)
        {
            lock(this)
            {
                state_ |= StateDone;
                if(ok)
                {
                    state_ |= StateOK;
                }
                _cancellationHandler = null;
                if(_completedCallback == null)
                {
                    if(observer_ != null)
                    {
                        observer_.detach();
                        observer_ = null;
                    }
                }
                if(_waitHandle != null)
                {
                    _waitHandle.Set();
                }
                System.Threading.Monitor.PulseAll(this);
                return _completedCallback;
            }
        }

        protected Ice.AsyncCallback finished(Ice.Exception ex)
        {
            lock(this)
            {
                state_ |= StateDone;
                _exception = ex;
                _cancellationHandler = null;
                if(observer_ != null)
                {
                    observer_.failed(ex.ice_id());
                }
                if(_completedCallback == null)
                {
                    if(observer_ != null)
                    {
                        observer_.detach();
                        observer_ = null;
                    }
                }
                if(_waitHandle != null)
                {
                    _waitHandle.Set();
                }
                System.Threading.Monitor.PulseAll(this);
                return _completedCallback;
            }
        }

        protected void setCompletedCallback(Ice.AsyncCallback cb)
        {
            lock(this)
            {
                if(cb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                if(_completedCallback != null)
                {
                    throw new System.ArgumentException("callback already set");
                }
                _completedCallback = cb;
                if((state_ & StateDone) == 0)
                {
                    return;
                }
                else if((getProxy() == null || !getProxy().ice_isTwoway()) && _exception == null)
                {
                    return;
                }
            }

            instance_.clientThreadPool().dispatch(() =>
            {
                try
                {
                    cb(this);
                }
                catch(System.Exception ex)
                {
                    warning(ex);
                }
            }, cachedConnection_);
        }

        protected virtual Ice.AsyncCallback getCompletedCallback()
        {
            return (Ice.AsyncResult result) => 
            { 
                Debug.Assert(exceptionCallback_ != null);
                try
                {
                    ((AsyncResultI)result).wait();
                }
                catch(Ice.Exception ex)
                {
                    exceptionCallback_(ex);
                    return;
                }
            };
        }

        protected void cancel(Ice.LocalException ex)
        {
            CancellationHandler handler;
            lock(this)
            {
                _cancellationException = ex;
                if(_cancellationHandler == null)
                {
                    return;
                }
                handler = _cancellationHandler;
            }
            handler.asyncRequestCanceled((OutgoingAsyncBase)this, ex);
        }

        protected virtual Ice.Instrumentation.InvocationObserver getObserver()
        {
            return observer_;
        }

        protected static T check<T>(Ice.AsyncResult r, string operation)
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
            if(!(r is T))
            {
                throw new System.ArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
            }
            return (T)r;
        }

        protected void warning(System.Exception ex)
        {
            if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
            {
                instance_.initializationData().logger.warning("exception raised by AMI callback:\n" + ex);
            }
        }

        protected IceInternal.Instance instance_;
        protected Ice.Instrumentation.InvocationObserver observer_;
        protected Ice.Connection cachedConnection_;
        protected bool sentSynchronously_;

        private readonly Ice.Communicator _communicator;
        private readonly string _operation;
        private readonly object _cookie;
        private Ice.Exception _exception;
        private EventWaitHandle _waitHandle;

        private CancellationHandler _cancellationHandler;
        private Ice.LocalException _cancellationException;

        private Ice.AsyncCallback _completedCallback;
        private Ice.AsyncCallback _sentCallback;
        protected Ice.ExceptionCallback exceptionCallback_;

        protected const int StateOK = 0x1;
        protected const int StateDone = 0x2;
        protected const int StateSent = 0x4;
        protected const int StateEndCalled = 0x8;
        protected const int StateCachedBuffers = 0x10;
        protected int state_;
    }
}
