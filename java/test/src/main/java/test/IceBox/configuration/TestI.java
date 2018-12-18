// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceBox.configuration;

import test.IceBox.configuration.Test.TestIntf;

public class TestI implements TestIntf
{
    public TestI(String[] args)
    {
        _args = args;
    }

    @Override
    public String getProperty(String name, com.zeroc.Ice.Current current)
    {
        return current.adapter.getCommunicator().getProperties().getProperty(name);
    }

    @Override
    public String[] getArgs(com.zeroc.Ice.Current current)
    {
        return _args;
    }

    final private String[] _args;
}
