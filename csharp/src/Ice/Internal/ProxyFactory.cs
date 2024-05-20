// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed class ProxyFactory
{
    public Ice.ObjectPrx propertyToProxy(string prefix)
    {
        string proxy = _instance.initializationData().properties.getProperty(prefix);
        Reference r = _instance.referenceFactory().create(proxy, prefix);
        return referenceToProxy(r);
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
