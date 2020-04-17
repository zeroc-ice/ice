//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

using Test;

public class TestFacet : ITestFacet
{
    private volatile IReadOnlyDictionary<string, string>? _changes;

    public IReadOnlyDictionary<string, string> getChanges(Ice.Current current) =>
        new Dictionary<string, string>(_changes!);

    internal void Updated(IReadOnlyDictionary<string, string> changes) => _changes = changes;
}
