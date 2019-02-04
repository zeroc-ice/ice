//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public sealed class CBI : Test.MB.CBDisp_
        {
            public CBI()
            {
            }

            public override Test.MA.CAPrx caop(Test.MA.CAPrx p, Ice.Current current)
            {
                return p;
            }

            public override Test.MB.CBPrx cbop(Test.MB.CBPrx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
