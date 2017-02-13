// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;

using Test;

public class TestFacetI : TestFacetDisp_, Ice.PropertiesAdminUpdateCallback
{
    public TestFacetI()
    {
    }

    override public Dictionary<string, string> getChanges(Ice.Current current)
    {
        return _changes;
    }

    public void updated(Dictionary<string, string> changes)
    {
        lock(this)
        {
            _changes = changes;
        }
    }

    private Dictionary<string, string> _changes;
}
