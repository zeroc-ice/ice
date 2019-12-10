//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;

namespace Ice
{
    namespace hold
    {
        public class AllTests : global::Test.AllTests
        {
            private class Condition
            {
                public Condition(bool value)
                {
                    _value = value;
                }

                public void
                set(bool value)
                {
                    lock (this)
                    {
                        _value = value;
                    }
                }

                public bool
                value()
                {
                    lock (this)
                    {
                        return _value;
                    }
                }

                private bool _value;
            }

            private class SetCB
            {
                public
                SetCB(Condition condition, int expected)
                {
                    _condition = condition;
                    _expected = expected;
                }

                public void
                response(int value)
                {
                    if (value != _expected)
                    {
                        _condition.set(false);
                    }
                }

                public void
                exception(Ice.Exception ex)
                {
                }

                private Condition _condition;
                private int _expected;
            }

            public static void allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();

                output.Flush();
                var hold = Test.HoldPrx.Parse($"hold:{helper.getTestEndpoint(0)}", communicator);
                var holdOneway = hold.Clone(oneway: true);

                var holdSerialized = Test.HoldPrx.Parse($"hold:{helper.getTestEndpoint(1)}", communicator);
                var holdSerializedOneway = holdSerialized.Clone(oneway: true);

                output.Write("changing state between active and hold rapidly... ");
                output.Flush();
                for (int i = 0; i < 100; ++i)
                {
                    hold.putOnHold(0);
                }
                for (int i = 0; i < 100; ++i)
                {
                    holdOneway.putOnHold(0);
                }
                for (int i = 0; i < 100; ++i)
                {
                    holdSerialized.putOnHold(0);
                }
                for (int i = 0; i < 100; ++i)
                {
                    holdSerializedOneway.putOnHold(0);
                }
                output.WriteLine("ok");

                output.Write("testing without serialize mode... ");
                output.Flush();
                Random rand = new Random();
                {
                    Condition cond = new Condition(true);
                    int value = 0;
                    Task result = null;
                    while (cond.value())
                    {
                        var expected = value;
                        result = hold.setAsync(++value, value < 500 ? rand.Next(5) : 0).ContinueWith(
                            (Task<int> t) =>
                            {
                                try
                                {
                                    if (t.Result != expected)
                                    {
                                        cond.set(false);
                                    }
                                }
                                catch (Exception)
                                {
                                }
                            });
                        if (value % 100 == 0)
                        {
                            result.Wait();
                        }

                        if (value > 100000)
                        {
                            // Don't continue, it's possible that out-of-order dispatch doesn't occur
                            // after 100000 iterations and we don't want the test to last for too long
                            // when this occurs.
                            break;
                        }
                    }
                    test(value > 100000 || !cond.value());
                    result.Wait();
                }
                output.WriteLine("ok");

                output.Write("testing with serialize mode... ");
                output.Flush();
                {
                    Condition cond = new Condition(true);
                    int value = 0;
                    Task result = null;
                    while (value < 3000 && cond.value())
                    {
                        var expected = value;
                        result = holdSerialized.setAsync(++value, 0).ContinueWith(
                            (Task<int> p) =>
                            {
                                try
                                {
                                    if (p.Result != expected)
                                    {
                                        cond.set(false);
                                    }
                                }
                                catch (Exception)
                                {
                                }
                            });
                        if (value % 100 == 0)
                        {
                            result.Wait();
                        }
                    }
                    result.Wait();
                    test(cond.value());

                    for (int i = 0; i < 10000; ++i)
                    {
                        holdSerializedOneway.setOneway(value + 1, value);
                        ++value;
                        if ((i % 100) == 0)
                        {
                            holdSerializedOneway.putOnHold(1);
                        }
                    }
                }
                output.WriteLine("ok");

                output.Write("testing serialization... ");
                output.Flush();
                {
                    int value = 0;
                    holdSerialized.set(value, 0);
                    Task result = null;
                    for (int i = 0; i < 10000; ++i)
                    {
                        // Create a new proxy for each request
                        result = holdSerialized.Clone(oneway: true).setOnewayAsync(value + 1, value);
                        ++value;
                        if ((i % 100) == 0)
                        {
                            result.Wait();
                            holdSerialized.IcePing(); // Ensure everything's dispatched.
                            holdSerialized.GetConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                        }
                    }
                    result.Wait();
                }
                output.WriteLine("ok");

                output.Write("testing waitForHold... ");
                output.Flush();
                {
                    hold.waitForHold();
                    hold.waitForHold();
                    for (int i = 0; i < 1000; ++i)
                    {
                        holdOneway.IcePing();
                        if ((i % 20) == 0)
                        {
                            hold.putOnHold(0);
                        }
                    }
                    hold.putOnHold(-1);
                    hold.IcePing();
                    hold.putOnHold(-1);
                    hold.IcePing();
                }
                output.WriteLine("ok");

                output.Write("changing state to hold and shutting down server... ");
                output.Flush();
                hold.shutdown();
                output.WriteLine("ok");
            }
        }
    }
}
