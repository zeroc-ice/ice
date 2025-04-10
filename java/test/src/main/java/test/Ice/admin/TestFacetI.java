// Copyright (c) ZeroC, Inc.

package test.Ice.admin;

import com.zeroc.Ice.Current;

import test.Ice.admin.Test.TestFacet;

public class TestFacetI implements TestFacet {
    @Override
    public void op(Current current) {}
}
