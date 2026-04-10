// Copyright (c) ZeroC, Inc.

namespace Ice.operations;

internal class Oneways
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    internal static void oneways(global::Test.TestHelper helper, Test.MyClassPrx p)
    {
        _ = helper.communicator();
        p = Test.MyClassPrxHelper.uncheckedCast(p.ice_oneway());
        p.ice_ping();
        p.opVoid();
        p.opIdempotent();

        // Calling a ["oneway"] operation on a oneway proxy succeeds.
        p.opOneway();

        // Calling a ["oneway"] operation on a twoway proxy throws OnewayOnlyException.
        try
        {
            Test.MyClassPrxHelper.uncheckedCast(p.ice_twoway()).opOneway();
            test(false);
        }
        catch (Ice.OnewayOnlyException)
        {
        }

        try
        {
            p.opByte(0xff, 0x0f, out byte b);
            test(false);
        }
        catch (Ice.TwowayOnlyException)
        {
        }
    }
}
