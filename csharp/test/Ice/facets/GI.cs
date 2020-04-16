//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.facets
{
    public sealed class G : Test.IG
    {
        public G(Communicator communicator) => _communicator = communicator;

        public string callG(Current current) => "G";

        public void shutdown(Current current) => _communicator.Shutdown();

        private readonly Communicator _communicator;
    }
}
