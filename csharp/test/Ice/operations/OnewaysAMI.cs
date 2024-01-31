//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice
{
    namespace operations
    {
        public class OnewaysAMI
        {
            private static void test(bool b)
            {
                if (!b)
                {
                    throw new System.Exception();
                }
            }

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
                    await p.ice_idsAsync();
                    test(false);
                }
                catch (TwowayOnlyException)
                {
                }

                await p.opVoidAsync();
                await p.opIdempotentAsync();
                await p.opNonmutatingAsync();
                
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
}
