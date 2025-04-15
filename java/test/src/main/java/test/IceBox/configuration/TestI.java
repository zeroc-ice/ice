// Copyright (c) ZeroC, Inc.

package test.IceBox.configuration;

import com.zeroc.Ice.Current;

import test.IceBox.configuration.Test.TestIntf;

public class TestI implements TestIntf {
    public TestI(String[] args) {
        _args = args;
    }

    @Override
    public String getProperty(String name, Current current) {
        return current.adapter.getCommunicator().getProperties().getProperty(name);
    }

    @Override
    public String[] getArgs(Current current) {
        return _args;
    }

    private final String[] _args;
}
