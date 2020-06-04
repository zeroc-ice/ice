//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class IA : MA.IIA
    {
        public MA.IIAPrx? iaop(MA.IIAPrx? p, Current current) => p;
    }
}
