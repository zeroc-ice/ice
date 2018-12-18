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
