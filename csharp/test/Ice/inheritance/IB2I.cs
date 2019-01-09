// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace inheritance
    {
        public sealed class IB2I : Test.MB.IB2Disp_
        {
            public IB2I()
            {
            }

            public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
            {
                return p;
            }

            public override Test.MB.IB2Prx ib2op(Test.MB.IB2Prx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
