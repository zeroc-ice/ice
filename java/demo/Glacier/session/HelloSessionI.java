// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

final class HelloSessionI extends _HelloSessionDisp
{
    public
    HelloSessionI(String userId, HelloSessionManagerI manager)
    {
        _userId = userId;
        _manager = manager;
    }

    public void
    hello(Ice.Current current)
    {
        System.out.println("Hello " + _userId);
    }

    public void
    destroy(Ice.Current current)
    {
        _manager.remove(current.id);
    }

    private String _userId;
    private HelloSessionManagerI _manager;
}
