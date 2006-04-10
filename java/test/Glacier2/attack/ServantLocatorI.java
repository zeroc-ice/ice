// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public final class ServantLocatorI extends Ice.LocalObjectImpl implements Ice.ServantLocator
{
    ServantLocatorI()
    {
        _backend = new BackendI();
    }

    public Ice.Object locate(Ice.Current curr, Ice.LocalObjectHolder cookie)
    {
        return _backend;
    }

    public void finished(Ice.Current curr, Ice.Object servant, Ice.LocalObject cookie)
    {
    }

    public void deactivate(String category)
    {
    }

    private Backend _backend;
}
