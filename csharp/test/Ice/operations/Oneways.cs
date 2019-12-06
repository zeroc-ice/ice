//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace operations
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

            internal static void oneways(global::Test.TestHelper helper, Test.MyClassPrx p)
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
                    byte b;
                    try
                    {
                        p.opByte(0xff, 0x0f, out b);
                        test(false);
                    }
                    catch (TwowayOnlyException)
                    {
                    }
                }
            }
        }
    }
}
