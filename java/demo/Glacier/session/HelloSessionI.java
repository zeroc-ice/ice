// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

final class HelloSessionI extends HelloSession
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
