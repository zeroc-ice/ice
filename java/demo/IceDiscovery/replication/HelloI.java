// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class HelloI extends _HelloDisp
{
    public HelloI(String name)
    {
        _name = name;
    }
    
    @Override
    public String
    getGreeting(Ice.Current current)
    {
        return _name + " says Hello World!";
    }
    
    @Override
    public void 
    shutdown(Ice.Current current)
    {
        System.out.println("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }

    final private String _name;
};
