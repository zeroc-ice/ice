//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public sealed class CAI : Test.MA.CADisp_
        {
            public CAI()
            {
            }

            public override Test.MA.CAPrx caop(Test.MA.CAPrx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
