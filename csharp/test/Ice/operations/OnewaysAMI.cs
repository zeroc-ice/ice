// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

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

                public void noException(Ice.Exception ex)
                {
                    test(false);
                }
            }

            internal static void onewaysAMI(global::Test.TestHelper helper, Test.MyClassPrx proxy)
            {
                Ice.Communicator communicator = helper.communicator();
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
                    Callback cb = new Callback();
                    p.begin_ice_ping().whenCompleted(cb.noException).whenSent(cb.sent);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_ice_ping().whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.noException(ex);
                        }
                    ).whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                }

                {
                    try
                    {
                        p.ice_isAAsync("::Test::MyClass");
                        test(false);
                    }
                    catch (Ice.TwowayOnlyException)
                    {
                    }
                }

                {
                    try
                    {
                        p.begin_ice_isA("::Test::MyClass");
                        test(false);
                    }
                    catch (System.ArgumentException)
                    {
                    }
                }

                {
                    try
                    {
                        p.ice_idAsync();
                        test(false);
                    }
                    catch (Ice.TwowayOnlyException)
                    {
                    }
                }

                {
                    try
                    {
                        p.begin_ice_id();
                        test(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                }

                {
                    try
                    {
                        p.ice_idsAsync();
                        test(false);
                    }
                    catch (Ice.TwowayOnlyException)
                    {
                    }
                }

                {
                    try
                    {
                        p.begin_ice_ids();
                        test(false);
                    }
                    catch (ArgumentException)
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
                    p.begin_opVoid().whenCompleted(cb.noException).whenSent(cb.sent);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opVoid().whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.noException(ex);
                        }
                    ).whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
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
                    p.begin_opIdempotent().whenCompleted(cb.noException).whenSent(cb.sent);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opIdempotent().whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.noException(ex);
                        }
                    ).whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
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
                    Callback cb = new Callback();
                    p.begin_opNonmutating().whenCompleted(cb.noException).whenSent(cb.sent);
                    cb.check();
                }

                {
                    Callback cb = new Callback();
                    p.begin_opNonmutating().whenCompleted(
                        (Ice.Exception ex) =>
                        {
                            cb.noException(ex);
                        }
                    ).whenSent(
                        (bool sentSynchronously) =>
                        {
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                }

                {
                    try
                    {
                        p.opByteAsync(0xff, 0x0f);
                        test(false);
                    }
                    catch (Ice.TwowayOnlyException)
                    {
                    }
                }

                {
                    try
                    {
                        p.begin_opByte(0xff, 0x0f);
                        test(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                }
            }
        }
    }
}
