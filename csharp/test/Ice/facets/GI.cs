//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class GI : Test.GDisp_
        {
            public GI(Ice.Communicator communicator)
            {
                _communicator = communicator;
            }

            public override string callG(Ice.Current current)
            {
                return "G";
            }

            public override void shutdown(Ice.Current current)
            {
                _communicator.shutdown();
            }

            private Ice.Communicator _communicator;
        }
    }
}
