// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami;

import test.Ice.ami.Test.TestIntfController;

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
