// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    public interface OutgoingAsyncMessageCallback
    {
        void sent__(Ice.ConnectionI connection);
        void finished__(Ice.LocalException ex);
    }

    public abstract class OutgoingAsync : OutgoingAsyncMessageCallback
    {
        public OutgoingAsync()
        {
        }

        public abstract void ice_exception(Ice.Exception ex);

        public BasicStream ostr__() // Avoids name clash with os__ member.
        {
            return os__;
        }

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

        public void sent__(Ice.ConnectionI connection)
        {
            lock(_monitor)
            {
                _sent = true;

                if(!_proxy.ice_isTwoway())
                {
                    cleanup(); // No response expected, we're done with the OutgoingAsync.
                }
                else if(_response)
                {
                    //
                    // If the response was already received notify finished() which is waiting.
                    //
                    Monitor.PulseAll(_monitor);
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
                lock(_monitor)
                {
                    Debug.Assert(os__ != null);
                    _response = true;

                    if(_timerTask != null && _proxy.reference__().getInstance().timer().cancel(_timerTask))
                    {
                        _timerTask = null; // Timer cancelled.
                    }

                    while(!_sent || _timerTask != null)
                    {
                        Monitor.Wait(_monitor);
                    }

                    is__.swap(istr);
                    replyStatus = is__.readByte();

                    switch(replyStatus)
                    {
                        case ReplyStatus.replyOK:
                        case ReplyStatus.replyUserException:
                        {
                            is__.startReadEncaps();
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
                warning(ex);
            }
            finally
            {
                lock(_monitor)
                {
                    cleanup();
                }
            }
        }

        public void finished__(Ice.LocalException exc)
        {
            bool retry = false;
            lock(_monitor)
            {
                if(os__ != null) // Might be called from prepare__ or before prepare__
                {
                    if(_timerTask != null && _proxy.reference__().getInstance().timer().cancel(_timerTask))
                    {
                        _timerTask = null; // Timer cancelled.
                    }

                    while(_timerTask != null)
                    {
                        Monitor.Wait(_monitor);
                    }

                    //
                    // A CloseConnectionException indicates graceful
                    // server shutdown, and is therefore always repeatable
                    // without violating "at-most-once". That's because by
                    // sending a close connection message, the server
                    // guarantees that all outstanding requests can safely
                    // be repeated. Otherwise, we can also retry if the
                    // operation mode is Nonmutating or Idempotent.
                    //
                    // An ObjectNotExistException can always be retried as
                    // well without violating "at-most-once".
                    //  
                    if(!_sent ||
                       _mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent ||
                       exc is Ice.CloseConnectionException || exc is Ice.ObjectNotExistException)
                    {
                        retry = true;
                    }
                }
            }

            if(retry)
            {
                try
                {
                    _cnt = _proxy.handleException__(_delegate, exc, _cnt);
                    send__();
                    return;
                }
                catch(Ice.LocalException)
                {
                }
            }

            try
            {
                ice_exception(exc);
            }
            catch(System.Exception ex)
            {
                warning(ex);
            }
            finally
            {
                lock(_monitor)
                {
                    cleanup();
                }
            }
        }

        public void finished__(LocalExceptionWrapper ex)
        {
            //
            // NOTE: This is called if sendRequest/sendAsyncRequest fails with
            // a LocalExceptionWrapper exception. It's not possible for the 
            // timer to be set at this point because the request couldn't be 
            // sent.
            //
            Debug.Assert(!_sent && _timerTask == null);

            try
            {
                if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent)
                {
                    _cnt = _proxy.handleExceptionWrapperRelaxed__(_delegate, ex, _cnt);
                }
                else
                {
                    _proxy.handleExceptionWrapper__(_delegate, ex);
                }
                send__();
            }
            catch(Ice.LocalException exc)
            {
                try
                {
                    ice_exception(exc);
                }
                catch(System.Exception exl)
                {
                    warning(exl);
                }
                finally
                {
                    lock(_monitor)
                    {
                        cleanup();
                    }
                }
            }
        }

        protected void prepare__(Ice.ObjectPrx prx, string operation, Ice.OperationMode mode,
                                 Dictionary<string, string> context)
        {
            lock(_monitor)
            {

                try
                {
                    //
                    // We must first wait for other requests to finish.
                    //
                    while(os__ != null)
                    {
                        Monitor.Wait(_monitor);
                    }

                    //
                    // Can't call async via a batch proxy.
                    //
                    _proxy = (Ice.ObjectPrxHelperBase)prx;
                    if(_proxy.ice_isBatchOneway() || _proxy.ice_isBatchDatagram())
                    {
                        throw new Ice.FeatureNotSupportedException("can't send batch requests with AMI");
                    }

                    _delegate = null;
                    _cnt = 0;
                    _mode = mode;
                    _sent = false;
                    _response = false;

                    Reference rf = _proxy.reference__();
                    Debug.Assert(is__ == null);
                    is__ = new BasicStream(rf.getInstance());
                    Debug.Assert(os__ == null);
                    os__ = new BasicStream(rf.getInstance());

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
                catch(Ice.LocalException)
                {
                    cleanup();
                    throw;
                }
            }
        }

        protected void send__()
        {
            //
            // NOTE: no synchronization needed. At this point, no other threads can be calling on this object.
            //

            RequestHandler handler;
            try
            {
                _delegate = _proxy.getDelegate__(true);
                handler = _delegate.getRequestHandler__();
            }
            catch(Ice.LocalException ex)
            {
                finished__(ex);
                return;
            }

            _sent = false;
            _response = false;
            handler.sendAsyncRequest(this);
        }

        protected abstract void response__(bool ok);

        private void runTimerTask__(Ice.ConnectionI connection)
        {
            lock(_monitor)
            {
                Debug.Assert(_timerTask != null && _sent); // Can only be set once the request is sent.

                if(_response) // If the response was just received, don't close the connection.
                {
                    connection = null;
                }
                _timerTask = null;
                Monitor.PulseAll(_monitor);
            }

            if(connection != null)
            {
                connection.exception(new Ice.TimeoutException());
            }
        }

        private void warning(System.Exception ex)
        {
            if(os__ != null) // Don't print anything if cleanup() was already called.
            {
                Reference rf = _proxy.reference__();
                if(rf.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
                                                                                "Ice.Warn.AMICallback", 1) > 0)
                {
                    rf.getInstance().initializationData().logger.warning("exception raised by AMI callback:\n" + ex);
                }
            }
        }

        private void cleanup()
        {
            Debug.Assert(_timerTask == null);

            is__ = null;
            os__ = null;

            Monitor.Pulse(_monitor);
        }

        protected BasicStream is__;
        protected BasicStream os__; // Cannot rename because the generated code assumes this name

        private bool _sent;
        private bool _response;
        private Ice.ObjectPrxHelperBase _proxy;
        private Ice.ObjectDel_ _delegate;
        private int _cnt;
        private Ice.OperationMode _mode;

        private TimerTask _timerTask;

        object _monitor = new object();
    }

    public abstract class BatchOutgoingAsync : OutgoingAsyncMessageCallback
    {
        public BatchOutgoingAsync()
        {
        }

        public abstract void ice_exception(Ice.LocalException ex);

        public BasicStream ostr__()
        {
            return os__;
        }

        public void sent__(Ice.ConnectionI connection)
        {
            lock(_monitor)
            {
                cleanup();
            }
        }

        public void finished__(Ice.LocalException exc)
        {
            try
            {
                ice_exception(exc);
            }
            catch(System.Exception ex)
            {
                warning(ex);
            }
            finally
            {
                lock(_monitor)
                {
                    cleanup();
                }
            }
        }

        protected void prepare__(Instance instance)
        {
            lock(_monitor)
            {
                while(os__ != null)
                {
                    Monitor.Wait(_monitor);
                }

                Debug.Assert(os__ == null);
                os__ = new BasicStream(instance);
            }
        }

        private void warning(System.Exception ex)
        {
            if(os__ != null) // Don't print anything if cleanup() was already called.
            {
                if(os__.instance().initializationData().properties.getPropertyAsIntWithDefault(
                       "Ice.Warn.AMICallback", 1) > 0)
                {
                    os__.instance().initializationData().logger.warning("exception raised by AMI callback:\n" + ex);
                }
            }
        }

        private void cleanup()
        {
            os__ = null;
            Monitor.Pulse(_monitor);
        }

        protected BasicStream os__;
        private object _monitor = new object();
    }

}

namespace Ice
{
    using System.Collections.Generic;

    public abstract class AMI_Object_ice_invoke : IceInternal.OutgoingAsync
    {
        public abstract void ice_response(bool ok, byte[] outParams);
        public abstract override void ice_exception(Ice.Exception ex);

        public void invoke__(Ice.ObjectPrx prx, string operation, OperationMode mode,
            byte[] inParams, Dictionary<string, string> context)
        {
            try
            {
                prepare__(prx, operation, mode, context);
                os__.writeBlob(inParams);
                os__.endWriteEncaps();
            }
            catch(LocalException ex)
            {
                finished__(ex);
                return;
            }
            send__();
        }

        protected override void response__(bool ok) // ok == true means no user exception.
        {
            byte[] outParams;
            try
            {
                int sz = is__.getReadEncapsSize();
                outParams = is__.readBlob(sz);
            }
            catch(LocalException ex)
            {
                finished__(ex);
                return;
            }
            ice_response(ok, outParams);
        }
    }

    public abstract class AMI_Object_ice_flushBatchRequests : IceInternal.BatchOutgoingAsync
    {
        public abstract override void ice_exception(LocalException ex);

        public void invoke__(Ice.ObjectPrx prx)
        {
            Ice.ObjectDel_ @delegate;
            IceInternal.RequestHandler handler;
            try
            {
                Ice.ObjectPrxHelperBase proxy = (Ice.ObjectPrxHelperBase)prx;
                prepare__(proxy.reference__().getInstance());
                @delegate = proxy.getDelegate__(true);
                handler = @delegate.getRequestHandler__();
            }
            catch(Ice.LocalException ex)
            {
                finished__(ex);
                return;
            }

            handler.flushAsyncBatchRequests(this);
        }
    }
}
