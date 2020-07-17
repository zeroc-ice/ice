//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class G : IG
    {
        public G(Communicator communicator) => _communicator = communicator;

        public string CallG(Current current) => "G";

        public void Shutdown(Current current) => _communicator.ShutdownAsync();

        private readonly Communicator _communicator;
    }
}
