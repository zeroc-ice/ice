// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class HelloI : HelloDisp_
{
    public HelloI(string name)
    {
        _name = name;
    }

    public override void sayHello(Ice.Current current)
    {
        Console.WriteLine(_name + " says Hello World!");
    }

    private string _name;
}
