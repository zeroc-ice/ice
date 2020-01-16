//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.operations
{
    class Oneways
    {
        private static void test(bool b)
        {
            if (!b)
            {
                throw new System.SystemException();
            }
        }

        internal static void oneways(global::Test.TestHelper helper, Test.IMyClassPrx p)
        {
            Communicator communicator = helper.communicator();
            p = p.Clone(oneway: true);

            {
                p.IcePing();
            }

            {
                p.opVoid();
            }

            {
                p.opIdempotent();
            }

            {
                p.opNonmutating();
            }

            {
                try
                {
                    var (b1, b2) = p.opByte(0xff, 0x0f);
                    test(false);
                }
                catch (TwowayOnlyException)
                {
                }
            }
        }
    }
}
