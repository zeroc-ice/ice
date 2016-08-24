// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

import com.zeroc.Ice.ServantLocator;

public final class ServantLocatorI implements ServantLocator
{
    @Override
    public ServantLocator.LocateResult locate(com.zeroc.Ice.Current curr)
    {
        return new ServantLocator.LocateResult();
    }

    @Override
    public void finished(com.zeroc.Ice.Current curr, com.zeroc.Ice.Object servant, java.lang.Object cookie)
    {
    }

    @Override
    public void deactivate(String category)
    {
    }
}
