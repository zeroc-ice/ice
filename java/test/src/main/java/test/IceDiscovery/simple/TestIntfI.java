// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
