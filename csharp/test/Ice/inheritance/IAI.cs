// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace inheritance
    {
        public sealed class IAI : Test.MA.IADisp_
        {
            public IAI()
            {
            }

            public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
