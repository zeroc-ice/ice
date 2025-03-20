// Copyright (c) ZeroC, Inc.

using Test;

public class TestFacetI : TestFacetDisp_
{
    public TestFacetI()
    {
    }

    public override Dictionary<string, string> getChanges(Ice.Current current) => _changes;

    public void updated(Dictionary<string, string> changes)
    {
        lock (this)
        {
            _changes = changes;
        }
    }

    private Dictionary<string, string> _changes;
}
