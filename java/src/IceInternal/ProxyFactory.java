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

final class ProxyFactory
{
    public Ice.ObjectPrx
    stringToProxy(String s)
    {
        Reference reference = new Reference(_instance, s);
        return referenceToProxy(reference);
    }

    public String
    proxyToString(Ice.ObjectPrx proxy)
    {
        return proxy.__reference().toString();
    }

    public Ice.ObjectPrx
    streamToProxy(BasicStream s)
    {
        String identity = s.readString();

        if (identity.length() == 0)
        {
            return null;
        }
        else
        {
            Reference reference = new Reference(identity, s);
            return referenceToProxy(reference);
        }
    }

    public Ice.ObjectPrx
    referenceToProxy(Reference reference)
    {
        Ice.ObjectPrx proxy = new Ice.ProxyForObject();
        proxy.setup(reference);
        return proxy;
    }

    public void
    proxyToStream(Ice.ObjectPrx proxy, BasicStream s)
    {
        if (proxy != null)
        {
            s.writeString(proxy.__reference().identity);
            proxy.__reference().streamWrite(s);
        }
        else
        {
            s.writeString("");
        }
    }

    //
    // Only for use by Instance
    //
    ProxyFactory(Instance instance)
    {
        _instance = instance;
    }

    private Instance _instance;
}
