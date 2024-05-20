// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed class ProxyFactory
{
    public string proxyToString(Ice.ObjectPrx proxy)
    {
        if (proxy != null)
        {
            Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)proxy;
            return h.iceReference().ToString();
        }
        else
        {
            return "";
        }
    }

    public Ice.ObjectPrx propertyToProxy(string prefix)
    {
        string proxy = _instance.initializationData().properties.getProperty(prefix);
        Reference r = _instance.referenceFactory().create(proxy, prefix);
        return referenceToProxy(r);
    }

    public Dictionary<string, string> proxyToProperty(Ice.ObjectPrx proxy, string prefix)
    {
        if (proxy != null)
        {
            Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)proxy;
            return h.iceReference().toProperty(prefix);
        }
        else
        {
            return new Dictionary<string, string>();
        }
    }

    public Ice.ObjectPrx streamToProxy(Ice.InputStream s)
    {
        var ident = new Ice.Identity(s);

        Reference r = _instance.referenceFactory().create(ident, s);
        return referenceToProxy(r);
    }

    public Ice.ObjectPrx referenceToProxy(Reference r) => r is null ? null : new ObjectPrxHelper(r);

    //
    // Only for use by Instance
    //
    internal ProxyFactory(Instance instance)
    {
        _instance = instance;
    }

    private Instance _instance;
}
