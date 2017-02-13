// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.dispatcher;

import test.Ice.dispatcher.Test._TestIntfControllerDisp;

class TestControllerI extends _TestIntfControllerDisp
{
    @Override
    public void
    holdAdapter(Ice.Current current)
    {
        _adapter.hold();
    }
    
    @Override
    public void
    resumeAdapter(Ice.Current current)
    {
        _adapter.activate();
    }
    
    public
    TestControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    final private Ice.ObjectAdapter _adapter;
}
