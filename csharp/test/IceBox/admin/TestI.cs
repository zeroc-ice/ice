//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

using Test;

public class TestFacetI : TestFacetDisp_
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
        lock (this)
        {
            _changes = changes;
        }
    }

    private Dictionary<string, string> _changes;
}
