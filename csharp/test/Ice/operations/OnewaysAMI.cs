// Copyright (c) ZeroC, Inc.

namespace Ice.operations;

public static class OnewaysAMI
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    internal static async Task onewaysAMI(Test.MyInterfacePrx proxy)
    {
        Test.MyInterfacePrx p = Test.MyInterfacePrxHelper.uncheckedCast(proxy.ice_oneway());

        await p.ice_pingAsync();

        try
        {
            _ = p.ice_isAAsync("::Test::MyInterface");
            test(false);
        }
        catch (TwowayOnlyException)
        {
        }

        try
        {
            _ = p.ice_idAsync();
            test(false);
        }
        catch (TwowayOnlyException)
        {
        }

        try
        {
            _ = p.ice_idsAsync();
            test(false);
        }
        catch (TwowayOnlyException)
        {
        }

        await p.opVoidAsync();
        await p.opIdempotentAsync();

        // Calling a ["oneway"] operation on a oneway proxy succeeds.
        await p.opOnewayAsync();

        // Calling a ["oneway"] operation on a twoway proxy throws OnewayOnlyException.
        try
        {
            _ = Test.MyInterfacePrxHelper.uncheckedCast(proxy.ice_twoway()).opOnewayAsync();
            test(false);
        }
        catch (OnewayOnlyException)
        {
        }

        try
        {
            _ = p.opByteAsync(0xff, 0x0f);
            test(false);
        }
        catch (TwowayOnlyException)
        {
        }
    }
}
