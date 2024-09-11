// Copyright (c) ZeroC, Inc.

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

    private final String[] _args;
}
