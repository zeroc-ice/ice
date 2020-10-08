// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;

namespace ZeroC.IceBox.Test.Admin
{
    public class TestFacet : ITestFacet
    {
        private volatile IReadOnlyDictionary<string, string>? _changes;

        public IReadOnlyDictionary<string, string> GetChanges(Ice.Current current, CancellationToken cancel) =>
            new Dictionary<string, string>(_changes!);

        internal void Updated(IReadOnlyDictionary<string, string> changes) => _changes = changes;
    }
}
