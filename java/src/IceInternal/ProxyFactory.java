// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
        Reference ref = _instance.referenceFactory().create(str);
        return referenceToProxy(ref);
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

        Reference ref = _instance.referenceFactory().create(ident, s);
        return referenceToProxy(ref);
    }

    public Ice.ObjectPrx
    referenceToProxy(Reference ref)
    {
        if(ref != null)
        {
            Ice.ObjectPrxHelper proxy = new Ice.ObjectPrxHelper();
            proxy.setup(ref);
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
		int v;

		try
		{
		    v = Integer.parseInt(arr[i]);
		}
		catch(NumberFormatException ex)
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
