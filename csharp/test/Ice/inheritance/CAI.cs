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
