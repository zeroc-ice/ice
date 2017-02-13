// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceBox.admin;

import test.IceBox.admin.Test.*;

public class TestFacetI implements TestFacet, com.zeroc.Ice.PropertiesAdminUpdateCallback
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
    public synchronized void updated(java.util.Map<String, String> changes)
    {
        _changes = changes;
    }

    private java.util.Map<String, String> _changes;
}
