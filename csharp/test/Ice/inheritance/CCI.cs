//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public sealed class CCI : Test.MA.CCDisp_
        {
            public CCI()
            {
            }

            public override Test.MA.CAPrx caop(Test.MA.CAPrx p, Ice.Current current)
            {
                return p;
            }

            public override Test.MA.CCPrx ccop(Test.MA.CCPrx p, Ice.Current current)
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
