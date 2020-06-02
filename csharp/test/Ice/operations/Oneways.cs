//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Operations
{
    public class Oneways
    {
        internal static void oneways(IMyClassPrx p)
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
