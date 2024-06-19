// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

public interface ProxyFactoryMethods<TProxy> : ObjectPrx where TProxy : class, ObjectPrx
{
    public new TProxy ice_oneway()
    {
        ObjectPrx obj = this;
        return (TProxy)obj.ice_oneway();
    }
}
