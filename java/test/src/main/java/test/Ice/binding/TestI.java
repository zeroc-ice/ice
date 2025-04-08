// Copyright (c) ZeroC, Inc.

package test.Ice.binding;

import com.zeroc.Ice.Current;

import test.Ice.binding.Test.TestIntf;

public class TestI implements TestIntf {
    TestI() {
    }

    @Override
    public String getAdapterName(Current current) {
        return current.adapter.getName();
    }
}
