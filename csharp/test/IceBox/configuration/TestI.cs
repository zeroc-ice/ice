// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class TestI : TestIntfDisp_
{
    public TestI(string[] args)
    {
        _args = args;
    }

    public override string
    getProperty(string name, Ice.Current current)
    {
        return current.adapter.getCommunicator().getProperties().getProperty(name);
    }

    public override string[]
    getArgs(Ice.Current current)
    {
        return _args;
    }

    private string[] _args;
}
