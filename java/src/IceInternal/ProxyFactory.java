// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class ProxyFactory
{
    public Ice.ObjectPrx
    stringToProxy(String str)
    {
        if(str.length() == 0)
        {
            return null;
        }
        else
        {
            Reference ref = _instance.referenceFactory().create(str);
            return referenceToProxy(ref);
        }
    }

    public String
    proxyToString(Ice.ObjectPrx proxy)
    {
        if(proxy != null)
        {
            Ice.ObjectPrxHelper h = (Ice.ObjectPrxHelper)proxy;
            return h.__reference().toString();
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

        if(ident.name.length() == 0)
        {
            return null;
        }
        else
        {
            Reference ref = _instance.referenceFactory().create(ident, s);
            return referenceToProxy(ref);
        }
    }

    public Ice.ObjectPrx
    referenceToProxy(Reference reference)
    {
        Ice.ObjectPrxHelper proxy = new Ice.ObjectPrxHelper();
        proxy.setup(reference);
        return proxy;
    }

    public void
    proxyToStream(Ice.ObjectPrx proxy, BasicStream s)
    {
        if(proxy != null)
        {
            Ice.ObjectPrxHelper h = (Ice.ObjectPrxHelper)proxy;
            Reference ref = h.__reference();
            ref.identity.__write(s);
            ref.streamWrite(s);
        }
        else
        {
            Ice.Identity ident = new Ice.Identity();
            ident.name = "";
            ident.category = "";
            ident.__write(s);
        }
    }

    public int[]
    getRetryIntervals()
    {  
	return _retryIntervals;
    }

    //
    // Only for use by Instance
    //
    ProxyFactory(Instance instance)
    {
        _instance = instance;

	String str = _instance.properties().getPropertyWithDefault("Ice.RetryIntervals", "0");

        String[] arr = str.trim().split("[ \t\n\r]+");

	if(arr.length > 0)
	{
	    _retryIntervals = new int[arr.length];

	    for(int i = 0; i < arr.length; i++)
	    {
		int v = Integer.parseInt(arr[i]);

		//
		// If -1 is the first value, no retry and wait intervals.
		// 
		if(i == 0 && v == -1)
		{
		    _retryIntervals = new int[0];
		    break;
		}

		_retryIntervals[i] = v > 0 ? v : 0;
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
