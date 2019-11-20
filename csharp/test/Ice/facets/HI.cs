//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class HI : Test.H
        {
            public HI(Ice.Communicator communicator)
            {
                _communicator = communicator;
            }

            public string callG(Ice.Current current)
            {
                return "G";
            }

            public string callH(Ice.Current current)
            {
                return "H";
            }

            public void shutdown(Ice.Current current)
            {
                _communicator.shutdown();
            }

            private Ice.Communicator _communicator;
        }
    }
}
