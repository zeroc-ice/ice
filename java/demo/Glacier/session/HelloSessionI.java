// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
