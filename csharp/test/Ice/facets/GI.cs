//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class G : IG
    {
        public G(Communicator communicator) => _communicator = communicator;

        public string callG(Current current) => "G";

        public void shutdown(Current current) => _communicator.ShutdownAsync();

        private readonly Communicator _communicator;
    }
}
