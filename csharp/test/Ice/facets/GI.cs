//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class GI : Test.G
        {
            public GI(Ice.Communicator communicator)
            {
                _communicator = communicator;
            }

            public string callG(Ice.Current current)
            {
                return "G";
            }

            public void shutdown(Ice.Current current)
            {
                _communicator.shutdown();
            }

            private Ice.Communicator _communicator;
        }
    }
}
