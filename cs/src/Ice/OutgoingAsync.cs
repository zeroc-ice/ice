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

    using System.Collections;
    using System.Diagnostics;

    public abstract class OutgoingAsync
    {
	public OutgoingAsync()
	{
	}
	
	public abstract void ice_exception(Ice.LocalException ex);
	
	public void __finished(BasicStream istr)
	{
	    System.Threading.Monitor.Enter(this);

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
	
	public void __finished(Ice.LocalException exc)
	{
	    System.Threading.Monitor.Enter(this);

	    if(_reference != null)
	    {
		if(_reference.locatorInfo != null)
		{
		    _reference.locatorInfo.clearObjectCache(_reference);
		}
		
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
		if(_mode == Ice.OperationMode.Nonmutating || _mode == Ice.OperationMode.Idempotent ||
		   exc is Ice.CloseConnectionException)
		{
		    try
		    {
			ProxyFactory proxyFactory = _reference.instance.proxyFactory();
			if(proxyFactory != null)
			{
			    _cnt = proxyFactory.checkRetryAfterException(exc, _cnt);
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
	
	public bool __timedOut()
	{
	    _timeoutMutex.WaitOne();
	    long absoluteTimeoutMillis = _absoluteTimeoutMillis;
	    _timeoutMutex.ReleaseMutex();

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
	    System.Threading.Monitor.Enter(this);

	    try
	    {
		//
		// We must first wait for other requests to finish.
		//
		while(_reference != null)
		{
		    try
		    {
			System.Threading.Monitor.Wait(this);
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
		}
		
		_reference = ((Ice.ObjectPrxHelperBase)prx).__reference();;
		Debug.Assert(_connection == null);
		_connection = _reference.getConnection();
		_cnt = 0;
		_mode = mode;
		Debug.Assert(__is == null);
		__is = new BasicStream(_reference.instance);
		Debug.Assert(__os == null);
		__os = new BasicStream(_reference.instance);
		
                //
                // If we are using a router, then add the proxy to the router info object.
                //
                if(_reference.routerInfo != null)
                {
                    _reference.routerInfo.addProxy(prx);
                }

		_connection.prepareRequest(__os);
		
		_reference.identity.__write(__os);

                //
                // For compatibility with the old FacetPath.
                //
                if(_reference.facet == null || _reference.facet.Length == 0)
                {
                    __os.writeStringSeq(null);
                }
                else
                {
                    string[] facetPath = { _reference.facet };
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
	
	protected void __send()
	{
	    System.Threading.Monitor.Wait(this);

	    try
	    {
		while(true)
		{
		    if(_connection == null)
		    {
			_connection = _reference.getConnection();
		    }
		    
		    _timeoutMutex.WaitOne();
		    if(_connection.timeout() >= 0)
		    {
			_absoluteTimeoutMillis = System.DateTime.Now.Ticks / 10 + _connection.timeout();
		    }
		    else
		    {
			_absoluteTimeoutMillis = 0;
		    }
		    _timeoutMutex.ReleaseMutex();
		    
		    try
		    {
			_connection.sendAsyncRequest(__os, this);
			
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
			if(_reference.locatorInfo != null)
			{
			    _reference.locatorInfo.clearObjectCache(_reference);
			}
			
			ProxyFactory proxyFactory = _reference.instance.proxyFactory();
			if(proxyFactory != null)
			{
			    _cnt = proxyFactory.checkRetryAfterException(ex, _cnt);
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

	protected internal abstract void __response(bool ok);
	
	private void warning(System.Exception ex)
	{
	    if(__os.instance().properties().getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	    {
	        __os.instance().logger().warning("exception raised by AMI callback:\n" + ex);
	    }
	}

	private void cleanup()
	{
	    if(_reference != null)
	    {
	        _reference = null;
	    }

	    if(_connection != null)
	    {
	        _connection = null;
	    }

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

	    System.Threading.Monitor.Pulse(this);
	    System.Threading.Monitor.Exit(this);
	}
	
	protected BasicStream __is;
	protected BasicStream __os;

	private Reference _reference;
	private Connection _connection;
	private int _cnt;
	private Ice.OperationMode _mode;

	//
	// Must be volatile, because we don't want to lock the monitor
	// below in __timedOut(), to avoid deadlocks.
	// This is true for Java -- for C#, we sadly can't have volatile longs :-(
	//
	private long _absoluteTimeoutMillis;
	System.Threading.Mutex _timeoutMutex = new System.Threading.Mutex();
    }

}
