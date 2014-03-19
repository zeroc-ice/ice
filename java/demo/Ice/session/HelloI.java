// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class HelloI extends _HelloDisp
{
    public
    HelloI(String name, int id)
    {
        _name = name;
        _id = id;
    }

    public void
    sayHello(Ice.Current current)
    {
        System.out.println("Hello object #" + _id + " for session `" + _name + "' says:\n" +
                           "Hello " + _name + "!");
    }

    final private String _name;
    final private int _id;
}
