// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public class HelloI : HelloDisp_
{
    public HelloI(string serviceName)
    {
        _serviceName = serviceName;
    }

    public override void sayHello(Ice.Current current)
    {
        System.Console.Out.WriteLine("Hello from " + _serviceName);
    }

    private string _serviceName;
}
