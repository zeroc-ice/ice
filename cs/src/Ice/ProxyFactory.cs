// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace IceInternal
{

    public sealed class ProxyFactory
    {
	public Ice.ObjectPrx stringToProxy(string str)
	{
	    Reference r = instance_.referenceFactory().create(str);
	    return referenceToProxy(r);
	}
	
	public string proxyToString(Ice.ObjectPrx proxy)
	{
	    if(proxy != null)
	    {
		Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase) proxy;
		return h.reference__().ToString();
	    }
	    else
	    {
		return "";
	    }
	}
	
	public Ice.ObjectPrx streamToProxy(BasicStream s)
	{
	    Ice.Identity ident = new Ice.Identity();
	    ident.read__(s);
	    
	    Reference r = instance_.referenceFactory().create(ident, s);
	    return referenceToProxy(r);
	}
	
	public Ice.ObjectPrx referenceToProxy(Reference r)
	{
	    if(r != null)
	    {
		Ice.ObjectPrxHelperBase proxy = new Ice.ObjectPrxHelperBase();
		proxy.setup(r);
		return proxy;
	    }
	    else
	    {
		return null;
	    }
	}
	
	public void proxyToStream(Ice.ObjectPrx proxy, BasicStream s)
	{
	    if(proxy != null)
	    {
		Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)proxy;
		Reference r = h.reference__();
		r.getIdentity().write__(s);
		r.streamWrite(s);
	    }
	    else
	    {
		Ice.Identity ident = new Ice.Identity();
		ident.name = "";
		ident.category = "";
		ident.write__(s);
	    }
	}
	
	public int checkRetryAfterException(Ice.LocalException ex, Reference @ref, int cnt)
	{
	    //
	    // We retry ObjectNotExistException if the reference is
	    // indirect. Otherwise, we don't retry other *NotExistException,
	    // which are all derived from RequestFailedException.
	    //
	    if(ex is Ice.ObjectNotExistException)
	    {
		try
		{
		    IndirectReference ir = (IndirectReference)@ref;
		    if(ir.getLocatorInfo() == null)
		    {
			throw ex;
		    }
		    ir.getLocatorInfo().clearObjectCache(ir);
		}
		catch(System.InvalidCastException)
		{
		    throw ex;
		}
	    }
	    else if(ex is Ice.RequestFailedException)
	    {
		throw ex;
	    }

            //
            // There is no point in retrying an operation that resulted in a
            // MarshalException. This must have been raised locally (because if
            // it happened in a server it would result in an UnknownLocalException
            // instead), which means there was a problem in this process that will
            // not change if we try again.
            //
            // The most likely cause for a MarshalException is exceeding the
            // maximum message size, which is represented by the the subclass
            // MemoryLimitException. For example, a client can attempt to send a
            // message that exceeds the maximum memory size, or accumulate enough
            // batch requests without flushing that the maximum size is reached.
            //
            // This latter case is especially problematic, because if we were to
            // retry a batch request after a MarshalException, we would in fact
            // silently discard the accumulated requests and allow new batch
            // requests to accumulate. If the subsequent batched requests do not
            // exceed the maximum message size, it appears to the client that all
            // of the batched requests were accepted, when in reality only the
            // last few are actually sent.
            //
	    if(ex is Ice.MarshalException)
	    {
		throw ex;
	    }

	    ++cnt;
	    Debug.Assert(cnt > 0);

	    TraceLevels traceLevels = instance_.traceLevels();
	    Ice.Logger logger = instance_.logger();

	    if(cnt > _retryIntervals.Length)
	    {
	        if(traceLevels.retry >= 1)
	        {
	    	    string s = "cannot retry operation call because retry limit has been exceeded\n" + ex;
	    	    logger.trace(traceLevels.retryCat, s);
	        }
	        throw ex;
	    }

	    int interval = _retryIntervals[cnt - 1];

	    if(traceLevels.retry >= 1)
	    {
	        string s = "re-trying operation call";
	        if(interval > 0)
	        {
	    	    s += " in " + interval + "ms";
	        }
	        s += " because of exception\n" + ex;
	        logger.trace(traceLevels.retryCat, s);
	    }

	    if(interval > 0)
	    {
	        //
	        // Sleep before retrying.
	        //
	        System.Threading.Thread.Sleep(interval);
	    }

	    return cnt;
	}

	//
	// Only for use by Instance
	//
	internal ProxyFactory(Instance instance)
	{
	    instance_ = instance;
	    
	    string str = instance_.properties().getPropertyWithDefault("Ice.RetryIntervals", "0");
	    
	    char[] separators = { ' ', '\t', '\n', '\r' };
	    string[] arr = str.Trim().Split(separators);
	    
	    if(arr.Length > 0)
	    {
		_retryIntervals = new int[arr.Length];
		
		for (int i = 0; i < arr.Length; i++)
		{
		    int v;
		    
		    try
		    {
			v = System.Int32.Parse(arr[i]);
		    }
		    catch(System.FormatException)
		    {
			v = 0;
		    }
		    
		    //
		    // If -1 is the first value, no retry and wait intervals.
		    // 
		    if(i == 0 && v == -1)
		    {
			_retryIntervals = new int[0];
			break;
		    }
		    
		    _retryIntervals[i] = v > 0?v:0;
		}
	    }
	    else
	    {
		_retryIntervals = new int[1];
		_retryIntervals[0] = 0;
	    }
	}
	
	private Instance instance_;
	private int[] _retryIntervals;
    }

}
