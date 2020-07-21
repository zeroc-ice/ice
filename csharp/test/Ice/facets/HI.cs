//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Facets
{
    public sealed class H : IH
    {
        public H(Communicator communicator) => _communicator = communicator;

        public string CallG(Current current) => "G";

        public string CallH(Current current) => "H";

        public void Shutdown(Current current) => _communicator.ShutdownAsync();

        private readonly Communicator _communicator;
    }
}
