//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.invoke;

public class ServantLocatorI implements Ice.ServantLocator
{
    public ServantLocatorI( boolean async)
    {
        if(async)
        {
            _blobject = new BlobjectAsyncI();
        }
        else
        {
            _blobject = new BlobjectI();
        }
    }

    @Override
    public Ice.Object
    locate(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
        return _blobject;
    }

    @Override
    public void
    finished(Ice.Current current, Ice.Object servant, java.lang.Object cookie)
    {
    }

    @Override
    public void
    deactivate(String category)
    {
    }

    private Ice.Object _blobject;
}
