// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.Facets
{
    public sealed class H : IH
    {
        public H(Communicator communicator) => _communicator = communicator;

        public string CallG(Current current, CancellationToken cancel) => "G";

        public string CallH(Current current, CancellationToken cancel) => "H";

        public void Shutdown(Current current, CancellationToken cancel) => _communicator.ShutdownAsync();

        private readonly Communicator _communicator;
    }
}
