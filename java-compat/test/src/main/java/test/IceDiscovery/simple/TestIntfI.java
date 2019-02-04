//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceDiscovery.simple;

import test.IceDiscovery.simple.Test.*;

public final class TestIntfI extends _TestIntfDisp
{
    @Override
    public String
    getAdapterId(Ice.Current current)
    {
        return current.adapter.getCommunicator().getProperties().getProperty(current.adapter.getName() + ".AdapterId");
    }
}
