// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;

namespace IceInternal
{

    public abstract class EventHandler
    {
        //
        // Return true if the handler is for a datagram transport, false otherwise.
        //
        abstract public bool datagram();

        //
        // Return true if beginAsyncEvent/endAsyncEvent return data in the stream.
        //
        abstract public bool readable();

        //
        // Perform a nonblocking read. Returns true if all the requested data was read,
        // false otherwise.
        //
        abstract public bool read(BasicStream stream);

        //
        // EventStatus indicates the outcome of an asynchronous I/O request.
        //
        public enum EventStatus { Success, Fail, Defer, Restart };

        //
        // Begin an asynchronous I/O request using the supplied callback and state.
        // If readable() returns true, the thread pool expects the handler to add
        // data to the stream.
        //
        // Return Success if the request was started successfully, Fail if the request
        // caused an error, Defer if the request must be completed at a later time
        // (e.g., the handler is in a holding state), or Restart if the asynchronous
        // I/O request should be restarted.
        //
        abstract public EventStatus beginAsyncEvent(BasicStream stream, AsyncCallback callback, object state,
                                                    out IAsyncResult result);

        //
        // Complete an asynchronous I/O request. The handler can use the cookie argument
        // to pass data to the message() method.
        //
        // Return Success if the request completed successfully, Fail if the request
        // caused an error, Defer if the request must be completed at a later time
        // (e.g., the handler is in a holding state), or Restart if the asynchronous
        // I/O request should be restarted.
        //
        abstract public EventStatus endAsyncEvent(BasicStream stream, IAsyncResult result, out object cookie);

        //
        // A complete message has been received.
        //
        abstract public void message(BasicStream stream, ThreadPool threadPool, object cookie);

        //
        // Will be called if the event handler is finally
        // unregistered. (Calling unregister() does not unregister
        // immediately.)
        //
        abstract public void finished(ThreadPool threadPool);

        //
        // Propagate an exception to the event handler.
        //
        abstract public void exception(Ice.LocalException ex);

        //
        // Get a textual representation of the event handler.
        //
        public abstract override string ToString();

        public Instance instance()
        {
            return instance_;
        }

        protected internal EventHandler(Instance instance)
        {
            instance_ = instance;
            stream_ = new BasicStream(instance);
        }

        protected internal Instance instance_;

        //
        // The stream_ data member is only for use by the ThreadPool or by the
        // connection for validation.
        //
        internal BasicStream stream_;

        //
        // Only for use by the ThreadPool and allocated when the handler is registered
        // for the first time.
        //
        internal ThreadPoolData tpd_ = null;

        internal sealed class ThreadPoolData
        {
            internal enum State
            {
                Inactive, // The handler does not have an I/O request pending.
                Pending,  // An I/O request is pending.
                Ready     // An I/O request has completed, or an exception has occurred.
            }

            internal bool registered;   // Is the handler registered with the pool?
            internal bool scheduled;    // Is the handler scheduled for a callback from the async thread?
            internal bool queued;       // Is the handler queued on the ready list?
            internal bool async;        // Is an async I/O callback pending?
            internal State state;
            internal object cookie;     // For passing state from endAsyncEvent() to message().
            internal Ice.LocalException exception;
            internal IAsyncResult result;

            internal ThreadPoolData()
            {
                registered = false;
                scheduled = false;
                queued = false;
                async = false;
                state = State.Inactive;
                cookie = null;
                exception = null;
                result = null;
            }
        }
    }

}
