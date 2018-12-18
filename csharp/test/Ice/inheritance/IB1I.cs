// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace inheritance
    {
        public sealed class IB1I : Test.MB.IB1Disp_
        {
            public IB1I()
            {
            }

            public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
            {
                return p;
            }

            public override Test.MB.IB1Prx ib1op(Test.MB.IB1Prx p, Ice.Current current)
            {
                return p;
            }
        }
    }
}
