// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
