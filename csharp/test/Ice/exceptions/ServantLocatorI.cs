// Copyright (c) ZeroC, Inc.

namespace Ice.exceptions;

public sealed class ServantLocatorI : Ice.ServantLocator
{
    public Ice.Object locate(Ice.Current curr, out System.Object cookie)
    {
        cookie = null;
        return null;
    }

    public void finished(Ice.Current curr, Ice.Object servant, System.Object cookie)
    {
    }

    public void deactivate(string category)
    {
    }
}
