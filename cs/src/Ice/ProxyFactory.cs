// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

namespace IceInternal
{

public sealed class ProxyFactory
{
    public Ice.ObjectPrx
    stringToProxy(string str)
    {
	Reference r = _instance.referenceFactory().create(str);
	return referenceToProxy(r);
    }
    
    public string
    proxyToString(Ice.ObjectPrx proxy)
    {
	if(proxy != null)
	{
	    Ice.ObjectPrxHelper h = (Ice.ObjectPrxHelper) proxy;
	    return h.__reference().ToString();
	}
	else
	{
	    return "";
	}
    }
    
    public Ice.ObjectPrx
    streamToProxy(BasicStream s)
    {
	Ice.Identity ident = new Ice.Identity();
	ident.__read(s);
	
	Reference r = _instance.referenceFactory().create(ident, s);
	return referenceToProxy(r);
    }
    
    public Ice.ObjectPrx
    referenceToProxy(Reference r)
    {
	if(r != null)
	{
	    Ice.ObjectPrxHelper proxy = new Ice.ObjectPrxHelper();
	    proxy.setup(r);
	    return proxy;
	}
	else
	{
	    return null;
	}
    }
    
    public void
    proxyToStream(Ice.ObjectPrx proxy, BasicStream s)
    {
	if(proxy != null)
	{
	    Ice.ObjectPrxHelper h = (Ice.ObjectPrxHelper)proxy;
	    Reference r = h.__reference();
	    r.identity.__write(s);
	    r.streamWrite(s);
	}
	else
	{
	    Ice.Identity ident = new Ice.Identity();
	    ident.name = "";
	    ident.category = "";
	    ident.__write(s);
	}
    }
    
    public int[] getRetryIntervals()
    {
	return _retryIntervals;
    }

    //
    // Only for use by Instance
    //
    internal
    ProxyFactory(Instance instance)
    {
	_instance = instance;
	
	string str = _instance.properties().getPropertyWithDefault("Ice.RetryIntervals", "0");
	
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
		if(i == 0 && v == - 1)
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
    
    private Instance _instance;
    private int[] _retryIntervals;
}

}
