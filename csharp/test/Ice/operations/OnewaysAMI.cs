// Copyright (c) ZeroC, Inc.

namespace Ice.operations
{
        public class OnewaysAMI
        {
            private static void test(bool b) => global::Test.TestHelper.test(b);

            internal static async Task onewaysAMI(Test.MyClassPrx proxy)
            {
                Test.MyClassPrx p = Test.MyClassPrxHelper.uncheckedCast(proxy.ice_oneway());

                await p.ice_pingAsync();

                try
                {
                    _ = p.ice_isAAsync("::Test::MyClass");
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
    }

