//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.operations
{
    class Oneways
    {
        internal static void oneways(TestHelper helper, Test.IMyClassPrx p)
        {
            p = p.Clone(oneway: true);
            p.IcePing();
            p.opVoid();
            p.opIdempotent();
            p.opOneway();
            p.opByte(0xff, 0x0f);
        }
    }
}
