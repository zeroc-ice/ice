// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    /// Callback to inform when a call has been passed to the local
    /// transport.
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

        Communicator getCommunicator();

        Connection getConnection();

        ObjectPrx getProxy();

        bool isCompleted_();

        void waitForCompleted();

        bool isSent();
        void waitForSent();

        void throwLocalException();

        bool sentSynchronously();

        string getOperation();

        AsyncResult whenSent(AsyncCallback cb);
        AsyncResult whenSent(SentCallback cb);
        AsyncResult whenCompleted(ExceptionCallback excb);
    }

    public interface AsyncResult<T> : AsyncResult
    {
        AsyncResult<T> whenCompleted(T cb, ExceptionCallback excb);

        new AsyncResult<T> whenCompleted(ExceptionCallback excb);
        new AsyncResult<T> whenSent(SentCallback cb);
    }
}

namespace IceInternal
{
    using System.Diagnostics;
    using System.Threading;

    abstract public class AsyncResultI : Ice.AsyncResult
    {
        public virtual void cancel()
        {
            Debug.Assert(outgoing_ != null);
            outgoing_.cancel();
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
                    Monitor.Wait(this);
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
                while((state_ & StateSent) == 0 && exception_ == null)
                {
                    Monitor.Wait(this);
                }
            }
        }

        public void throwLocalException()
        {
            lock(this)
            {
                if(exception_ != null)
                {
                    throw exception_;
                }
            }
        }

        public bool sentSynchronously()
        {
            Debug.Assert(outgoing_ != null);
            return  outgoing_.sentSynchronously(); // No lock needed
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
                Debug.Assert(outgoing_ != null);
                if(getProxy() != null && getProxy().ice_isTwoway())
                {
                    return false;
                }
                return outgoing_.sentSynchronously();
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
                    if(waitHandle_ == null)
                    {
                        waitHandle_ = new EventWaitHandle(false, EventResetMode.ManualReset);
                    }
                    if((state_ & StateDone) != 0)
                    {
                        waitHandle_.Set();
                    }
                    return waitHandle_;
                }
            }
        }

        public OutgoingAsyncBase OutgoingAsync
        {
            get
            {
                return outgoing_;
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
                if(sentCallback_ != null)
                {
                    throw new System.ArgumentException("sent callback already set");
                }
                sentCallback_ = cb;
                if((state_ & StateSent) == 0)
                {
                    return this;
                }
            }

            if(outgoing_.sentSynchronously())
            {
                try
                {
                    sentCallback_(this);
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
                        sentCallback_(this);
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
                if(sentCallback_ != null)
                {
                    throw new System.ArgumentException("sent callback already set");
                }
                sentCallback_ = (Ice.AsyncResult r) =>
                    {
                        cb(r.sentSynchronously());
                    };
                if((state_ & StateSent) == 0)
                {
                    return this;
                }
            }

            if(outgoing_.sentSynchronously())
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
                    Monitor.Wait(this);
                }
                if(exception_ != null)
                {
                    throw exception_;
                }
                return (state_ & StateOK) != 0;
            }
        }

        protected AsyncResultI(Ice.Communicator communicator,
                               Instance instance,
                               string op,
                               object cookie,
                               Ice.AsyncCallback cb)
        {
            instance_ = instance;
            state_ = 0;

            _communicator = communicator;
            _operation = op;
            exception_ = null;
            _cookie = cookie;
            completedCallback_ = cb;
        }

        protected void setCompletedCallback(Ice.AsyncCallback cb)
        {
            lock(this)
            {
                if(cb == null)
                {
                    throw new System.ArgumentException("callback is null");
                }
                if(completedCallback_ != null)
                {
                    throw new System.ArgumentException("callback already set");
                }
                completedCallback_ = cb;
                if((state_ & StateDone) == 0)
                {
                    return;
                }
                else if((getProxy() == null || !getProxy().ice_isTwoway()) && exception_ == null)
                {
                    return;
                }
            }

            instance_.clientThreadPool().dispatch(() =>
            {
                try
                {
                    try
                    {
                        cb(this);
                    }
                    catch(System.AggregateException ex)
                    {
                        throw ex.InnerException;
                    }
                }
                catch(System.Exception ex)
                {
                    warning(ex);
                }
            }, cachedConnection_);
        }

        abstract protected Ice.AsyncCallback getCompletedCallback();

        public static AsyncResultI check(Ice.AsyncResult r, Ice.ObjectPrx prx, string operation)
        {
            if(r != null && r.getProxy() != prx)
            {
                throw new System.ArgumentException("Proxy for call to end_" + operation +
                                                   " does not match proxy that was used to call corresponding begin_" +
                                                   operation + " method");
            }
            return check(r, operation);
        }

        public static AsyncResultI check(Ice.AsyncResult r, string operation)
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
            if(!(r is AsyncResultI))
            {
                throw new System.ArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
            }
            return (AsyncResultI)r;
        }

        protected void warning(System.Exception ex)
        {
            if(instance_.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
            {
                instance_.initializationData().logger.warning("exception raised by AMI callback:\n" + ex);
            }
        }

        protected Instance instance_;
        protected Ice.Instrumentation.InvocationObserver observer_;
        protected Ice.Connection cachedConnection_;

        private readonly Ice.Communicator _communicator;
        private readonly string _operation;
        private readonly object _cookie;
        protected Ice.Exception exception_;
        protected EventWaitHandle waitHandle_;

        protected Ice.AsyncCallback completedCallback_;
        protected Ice.AsyncCallback sentCallback_;
        protected Ice.ExceptionCallback exceptionCallback_;

        protected const int StateOK = 0x1;
        protected const int StateDone = 0x2;
        protected const int StateSent = 0x4;
        protected const int StateEndCalled = 0x8;
        protected int state_;
        protected OutgoingAsyncBase outgoing_;
    }
}
