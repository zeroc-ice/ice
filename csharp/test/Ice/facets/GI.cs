// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Facets
{
    public sealed class G : IG
    {
        public G(Communicator communicator) => _communicator = communicator;

        public string CallG(Current current, CancellationToken cancel) => "G";

        public void Shutdown(Current current, CancellationToken cancel) => _communicator.ShutdownAsync();

        private readonly Communicator _communicator;
    }
}
