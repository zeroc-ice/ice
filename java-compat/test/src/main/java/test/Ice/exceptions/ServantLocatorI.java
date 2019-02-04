//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.exceptions;

public final class ServantLocatorI implements Ice.ServantLocator
{
    @Override
    public Ice.Object locate(Ice.Current curr, Ice.LocalObjectHolder cookie)
    {
        return null;
    }

    @Override
    public void finished(Ice.Current curr, Ice.Object servant, java.lang.Object cookie)
    {
    }

    @Override
    public void deactivate(String category)
    {
    }
}
