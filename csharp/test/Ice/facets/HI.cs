// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace facets
    {
        public sealed class HI : Test.HDisp_
        {
            public HI(Ice.Communicator communicator)
            {
                _communicator = communicator;
            }

            public override string callG(Ice.Current current)
            {
                return "G";
            }

            public override string callH(Ice.Current current)
            {
                return "H";
            }

            public override void shutdown(Ice.Current current)
            {
                _communicator.shutdown();
            }

            private Ice.Communicator _communicator;
        }
    }
}
