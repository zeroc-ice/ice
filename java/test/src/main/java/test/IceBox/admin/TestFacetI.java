// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceBox.admin;

import test.IceBox.admin.Test.*;

public class TestFacetI implements TestFacet, java.util.function.Consumer<java.util.Map<String, String>>
{
    public TestFacetI()
    {
    }

    @Override
    public synchronized java.util.Map<String, String> getChanges(com.zeroc.Ice.Current current)
    {
       return _changes;
    }

    @Override
    public synchronized void accept(java.util.Map<String, String> changes)
    {
        _changes = changes;
    }

    private java.util.Map<String, String> _changes;
}
