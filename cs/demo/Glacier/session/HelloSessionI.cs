// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

sealed class HelloSessionI : HelloSession
{
    public HelloSessionI(string userId, HelloSessionManagerI manager)
    {
        _userId = userId;
        _manager = manager;
    }
    
    public override void hello(Ice.Current current)
    {
        System.Console.Out.WriteLine("Hello " + _userId);
    }
    
    public override void destroy(Ice.Current current)
    {
        _manager.remove(current.id);
    }
    
    private string _userId;
    private HelloSessionManagerI _manager;
}
