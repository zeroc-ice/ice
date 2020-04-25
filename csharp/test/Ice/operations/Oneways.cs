//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.operations
{
    public class Oneways
    {
        internal static void oneways(Test.IMyClassPrx p)
        {
            p = p.Clone(oneway: true);
            p.IcePing();
            p.opVoid();
            p.opIdempotent();
            p.opOneway();
            p.opOnewayMetadata();
            p.opByte(0xff, 0x0f);
        }
    }
}
