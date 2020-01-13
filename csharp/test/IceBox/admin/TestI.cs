//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

using Test;

public class TestFacet : ITestFacet
{
    public Dictionary<string, string> getChanges(Ice.Current current) => _changes;

    public void updated(Dictionary<string, string> changes)
    {
        lock (this)
        {
            _changes = changes;
        }
    }

    private Dictionary<string, string> _changes;
}
