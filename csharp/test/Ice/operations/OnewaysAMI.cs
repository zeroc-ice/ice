//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Threading;

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

            private class CallbackBase
            {
                internal CallbackBase()
                {
                    _called = false;
                }

                public virtual void check()
                {
                    lock (this)
                    {
                        while (!_called)
                        {
                            Monitor.Wait(this);
                        }
                        _called = false;
                    }
                }

                public virtual void called()
                {
                    lock (this)
                    {
                        Debug.Assert(!_called);
                        _called = true;
                        Monitor.Pulse(this);
                    }
                }

                private bool _called;
            }

            private class Callback : CallbackBase
            {
                public Callback()
                {
                }

                public void
                sent(bool sentSynchronously)
                {
                    called();
                }

                public void noException(Exception ex)
                {
                    test(false);
                }
            }

            internal static void onewaysAMI(global::Test.TestHelper helper, Test.MyClassPrx proxy)
            {
                Communicator communicator = helper.communicator();
                Test.MyClassPrx p = Test.MyClassPrxHelper.uncheckedCast(proxy.ice_oneway());

                {
                    Callback cb = new Callback();
                    p.ice_pingAsync(progress: new Progress<bool>(
                        sentSynchronously =>
                        {
                            cb.sent(sentSynchronously);
                        }));
                    cb.check();
                }

                {
                    try
                    {
                        p.ice_isAAsync("::Test::MyClass");
                        test(false);
                    }
                    catch (TwowayOnlyException)
                    {
                    }
                }

                {
                    try
                    {
                        p.ice_idAsync();
                        test(false);
                    }
                    catch (TwowayOnlyException)
                    {
                    }
                }

                {
                    try
                    {
                        p.ice_idsAsync();
                        test(false);
                    }
                    catch (TwowayOnlyException)
                    {
                    }
                }

                {
                    Callback cb = new Callback();
                    p.opVoidAsync(progress: new Progress<bool>(
                        sentSynchronously =>
                        {
                            cb.sent(sentSynchronously);
                        }));
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.opIdempotentAsync(progress: new Progress<bool>(
                        sentSynchronously =>
                        {
                            cb.sent(sentSynchronously);
                        }));
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.opNonmutatingAsync(progress: new Progress<bool>(
                        sentSynchronously =>
                        {
                            cb.sent(sentSynchronously);
                        }));
                    cb.check();
                }

                {
                    try
                    {
                        p.opByteAsync(0xff, 0x0f);
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
