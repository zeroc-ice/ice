// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceDiscovery.simple;

import test.IceDiscovery.simple.Test.*;

public final class TestIntfI implements TestIntf
{
    @Override
    public String getAdapterId(com.zeroc.Ice.Current current)
    {
        return current.adapter.getCommunicator().getProperties().getProperty(current.adapter.getName() + ".AdapterId");
    }
}
