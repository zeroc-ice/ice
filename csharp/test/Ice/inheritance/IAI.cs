//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Inheritance
{
    public sealed class IA : MA.IIA
    {
        public MA.IIAPrx? Iaop(MA.IIAPrx? p, Current current) => p;
    }
}
