// Copyright (c) ZeroC, Inc.

namespace Ice.operations;

internal class Oneways
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    internal static void oneways(global::Test.TestHelper helper, Test.MyClassPrx p)
    {
        _ = helper.communicator();
        p = Test.MyClassPrxHelper.uncheckedCast(p.ice_oneway());

        {
            p.ice_ping();
        }

        {
            p.opVoid();
        }

        {
            p.opIdempotent();
        }

        {
            try
            {
                p.opByte((byte)0xff, (byte)0x0f, out byte b);
                test(false);
            }
            catch (Ice.TwowayOnlyException)
            {
            }
        }
    }
}
