// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;

public class HelloI : HelloDisp_
{
    public HelloI(string name, int id)
    {
        _name =  name;
        _id = id;
    }

    public override void sayHello(Ice.Current c) 
    {
        Console.Out.WriteLine("Hello object #" + _id + " for session `" + _name + "' says:\n" +
                              "Hello " + _name + "!");
    }

    private string _name;
    private int _id;
}
