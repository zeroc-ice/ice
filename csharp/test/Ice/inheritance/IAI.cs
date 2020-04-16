//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.inheritance
{
    public sealed class IA : Test.MA.IIA
    {
        public Test.MA.IIAPrx? iaop(Test.MA.IIAPrx? p, Current current) => p;
    }
}
