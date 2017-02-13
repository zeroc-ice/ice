// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceBox.configuration;

import test.IceBox.configuration.Test._TestIntfDisp;


public class TestI extends _TestIntfDisp
{
    public
    TestI(String[] args)
    {
        _args = args;
    }

    @Override
    public String
    getProperty(String name, Ice.Current current)
    {
        return current.adapter.getCommunicator().getProperties().getProperty(name);
    }
    
    @Override
    public String[]
    getArgs(Ice.Current current)
    {
        return _args;
    }
    
    final private String[] _args;
}
