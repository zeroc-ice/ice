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

    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Threading;

    public abstract class OutgoingAsync
    {
        public abstract void ice_exception(Ice.Exception ex);
	
        public void __finished(BasicStream istr)
        {
            lock(_monitor)
            {
                DispatchStatus status;
		
                try
                {
                    __is.swap(istr);
		    
                    status = (DispatchStatus)__is.readByte();
		    
                    switch(status)
                    {
                        case DispatchStatus.DispatchOK:
                        case DispatchStatus.DispatchUserException:
                        {
                            __is.startReadEncaps();
                            break;
                        }
			
                        case DispatchStatus.DispatchObjectNotExist:
                        case DispatchStatus.DispatchFacetNotExist:
                        case DispatchStatus.DispatchOperationNotExist:
                        {
                            Ice.Identity id = new Ice.Identity();
                            id.__read(__is);

                            //
                            // For compatibility with the old FacetPath.
                            //
                            string[] facetPath = __is.readStringSeq();
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

                            string operation = __is.readString();

                            Ice.RequestFailedException ex = null;
                            switch(status)
                            {
                                case DispatchStatus.DispatchObjectNotExist:
                                {
                                    ex = new Ice.ObjectNotExistException();
                                    break;
                                }
				
                                case DispatchStatus.DispatchFacetNotExist:
                                {
                                    ex = new Ice.FacetNotExistException();
                                    break;
                                }
				
                                case DispatchStatus.DispatchOperationNotExist:
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
			
                        case DispatchStatus.DispatchUnknownException:
                        case DispatchStatus.DispatchUnknownLocalException:
                        case DispatchStatus.DispatchUnknownUserException:
                        {
                            string unknown = __is.readString();

                            Ice.UnknownException ex = null;
                            switch(status)
                            {
                                case DispatchStatus.DispatchUnknownException:
                                {
                                    ex = new Ice.UnknownException();
                                    break;
                                }
                                case DispatchStatus.DispatchUnknownLocalException:
                                {
                                    ex = new Ice.UnknownLocalException();
                                    break;
                                }
                                case DispatchStatus.DispatchUnknownUserException:
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
                catch(Ice.LocalException ex)
                {
                    __finished(ex);
                    return;
                }
		    
                Debug.Assert(status == DispatchStatus.DispatchOK || status == DispatchStatus.DispatchUserException);
		
                try
                {
                    __response(status == DispatchStatus.DispatchOK);
                }
                catch(System.Exception ex)
                {
                    warning(ex);
                }
                finally
                {
                    cleanup();
                }
            }
        }
	
        public void __finished(Ice.LocalException exc)
        {
            lock(_monitor)
            {

                if(_reference != null)
                {
                    bool doRetry = false;
		    
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
                    if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent ||
                        exc is Ice.CloseConnectionException || exc is Ice.ObjectNotExistException)
                    {
                        try
                        {
                            ProxyFactory proxyFactory = _reference.getInstance().proxyFactory();
                            if(proxyFactory != null)
                            {
                                _cnt = proxyFactory.checkRetryAfterException(exc, _reference, _cnt);
                            }
                            else
                            {
                                throw exc; // The communicator is already destroyed, so we cannot retry.
                            }
			    
                            doRetry = true;
                        }
                        catch(Ice.LocalException)
                        {
                        }
                    }
		    
                    if(doRetry)
                    {
                        _connection = null;
                        __send();
                        return;
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
                    cleanup();
                }
            }
        }
	
        public bool __timedOut()
        {
	    long absoluteTimeoutMillis;

            lock(_timeoutMutex) // MONO bug: Should be WaitOne(), but that's broken under Mono 1.0 for Linux.
	    {
		absoluteTimeoutMillis = _absoluteTimeoutMillis;
	    }

            if(absoluteTimeoutMillis > 0)
            {
                return System.DateTime.Now.Ticks / 10 >= absoluteTimeoutMillis;
            }
            else
            {
                return false;
            }
        }

        protected void __prepare(Ice.ObjectPrx prx, string operation, Ice.OperationMode mode, Ice.Context context)
        {
            lock(_monitor)
            {

                try
                {
                    //
                    // We must first wait for other requests to finish.
                    //
                    while(_reference != null)
                    {
                        Monitor.Wait(_monitor);
                    }
		    
                    _reference = ((Ice.ObjectPrxHelperBase)prx).__reference();;
                    Debug.Assert(_connection == null);
                    _connection = _reference.getConnection(out _compress);
                    _cnt = 0;
                    _mode = mode;
                    Debug.Assert(__is == null);
                    __is = new BasicStream(_reference.getInstance());
                    Debug.Assert(__os == null);
                    __os = new BasicStream(_reference.getInstance());
		    
                    //
                    // If we are using a router, then add the proxy to the router info object.
                    //
                    try
                    {
                        RoutableReference rr = (RoutableReference)_reference;
                        if(rr != null && rr.getRouterInfo() != null)
                        {
                            rr.getRouterInfo().addProxy(prx);
                        }

                    }
                    catch(InvalidCastException)
                    {
                    }

                    _connection.prepareRequest(__os);
		    
                    _reference.getIdentity().__write(__os);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    string facet = _reference.getFacet();
                    if(facet == null || facet.Length == 0)
                    {
                        __os.writeStringSeq(null);
                    }
                    else
                    {
                        string[] facetPath = { facet };
                        __os.writeStringSeq(facetPath);
                    }

                    __os.writeString(operation);

                    __os.writeByte((byte)mode);

                    if(context == null)
                    {
                        __os.writeSize(0);
                    }
                    else
                    {
                        int sz = context.Count;
                        __os.writeSize(sz);
                        if(sz > 0)
                        {
                            foreach(DictionaryEntry e in context)
                            {
                                __os.writeString((string)e.Key);
                                __os.writeString((string)e.Value);
                            }
                        }
                    }
		    
                    __os.startWriteEncaps();
                }
                catch(Ice.LocalException ex)
                {
                    cleanup();
                    throw ex;
                }
            }
        }
	
        protected void __send()
        {
            lock(_monitor)
            {
                try
                {
                    while(true)
                    {
                        if(_connection == null)
                        {
                            _connection = _reference.getConnection(out _compress);
                        }
			
			lock(_timeoutMutex) // MONO bug: Should be WaitOne(), but that's broken under Mono 1.0 for Linux.
			{
			    if(_connection.timeout() >= 0)
			    {
				_absoluteTimeoutMillis = System.DateTime.Now.Ticks / 10 + _connection.timeout();
			    }
			    else
			    {
				_absoluteTimeoutMillis = 0;
			    }
			}
			
                        try
                        {
                            _connection.sendAsyncRequest(__os, this, _compress);
			    
                            //
                            // Don't do anything after sendAsyncRequest() returned
                            // without an exception.  I such case, there will be
                            // callbacks, i.e., calls to the __finished()
                            // functions. Since there is no mutex protection, we
                            // cannot modify state here and in such callbacks.
                            //
                            return;
                        }
                        catch(Ice.LocalException ex)
                        {
                            ProxyFactory proxyFactory = _reference.getInstance().proxyFactory();
                            if(proxyFactory != null)
                            {
                                _cnt = proxyFactory.checkRetryAfterException(ex, _reference, _cnt);
                            }
                            else
                            {
                                throw ex; // The communicator is already destroyed, so we cannot retry.
                            }
                        }
			
                        _connection = null;
                    }
                }
                catch(Ice.LocalException ex)
                {
                    __finished(ex);
                }
            }
        }

        protected internal abstract void __response(bool ok);
	
        private void warning(System.Exception ex)
        {
	    if(_reference != null) // Don't print anything if cleanup() was already called.
	    {
		if(_reference.getInstance().properties().getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
		{
		    _reference.getInstance().logger().warning("exception raised by AMI callback:\n" + ex);
		}
	    }
        }

        private void cleanup()
        {
	    _reference = null;
	    _connection = null;
            if(__is != null)
            {
                __is.destroy();
                __is = null;
            }
            if(__os != null)
            {
                __os.destroy();
                __os = null;
            }

            Monitor.Pulse(_monitor);
        }
	
        protected BasicStream __is;
        protected BasicStream __os;

        private Reference _reference;
        private Ice.ConnectionI _connection;
        private int _cnt;
        private Ice.OperationMode _mode;
        private bool _compress;

        private long _absoluteTimeoutMillis;
        Mutex _timeoutMutex = new Mutex();

        object _monitor = new object();
    }

}

namespace Ice
{

    public abstract class AMI_Object_ice_invoke : IceInternal.OutgoingAsync
    {
        public abstract void ice_response(bool ok, byte[] outParams);
        public abstract override void ice_exception(Ice.Exception ex);

        public void __invoke(Ice.ObjectPrx prx, string operation, OperationMode mode,
            byte[] inParams, Ice.Context context)
        {
            try
            {
                __prepare(prx, operation, mode, context);
                __os.writeBlob(inParams);
                __os.endWriteEncaps();
            }
            catch(LocalException ex)
            {
                __finished(ex);
                return;
            }
            __send();
        }

        protected internal override void __response(bool ok) // ok == true means no user exception.
        {
            byte[] outParams;
            try
            {
                int sz = __is.getReadEncapsSize();
                outParams = __is.readBlob(sz);
            }
            catch(LocalException ex)
            {
                ice_exception(ex);
                return;
            }
            ice_response(ok, outParams);
        }
    }

}
