//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.dispatcher;

import test.Ice.dispatcher.Test.TestIntfController;

class TestControllerI implements TestIntfController
{
    @Override
    public void holdAdapter(com.zeroc.Ice.Current current)
    {
        _adapter.hold();
    }

    @Override
    public void resumeAdapter(com.zeroc.Ice.Current current)
    {
        _adapter.activate();
    }

    public TestControllerI(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    final private com.zeroc.Ice.ObjectAdapter _adapter;
}
