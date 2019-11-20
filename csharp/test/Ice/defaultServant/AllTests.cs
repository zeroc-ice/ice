//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using Ice.defaultServant.Test;

namespace Ice
{
    namespace defaultServant
    {
        public class AllTests : global::Test.AllTests
        {
            public static void
            allTests(global::Test.TestHelper helper)
            {
                var output = helper.getWriter();
                Communicator communicator = helper.communicator();
                ObjectAdapter oa = communicator.createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
                oa.activate();

                var servantI = new MyObjectI();
                var servantT = default(MyObjectTraits);
                Disp servantD = (incoming, current) => servantT.Dispatch(servantI, incoming, current);

                //
                // Register default servant with category "foo"
                //
                oa.addDefaultServant(servantD, "foo");

                //
                // Start test
                //
                output.Write("testing single category... ");
                output.Flush();

                Disp r = oa.findDefaultServant("foo");
                test(r == servantD);

                r = oa.findDefaultServant("bar");
                test(r == null);

                Ice.Identity identity = new Ice.Identity();
                identity.category = "foo";

                string[] names = new string[] { "foo", "bar", "x", "y", "abcdefg" };

                MyObjectPrx prx = null;
                for (int idx = 0; idx < 5; ++idx)
                {
                    identity.name = names[idx];
                    prx = MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
                    prx.ice_ping();
                    test(prx.getName() == names[idx]);
                }

                identity.name = "ObjectNotExist";
                prx = MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
                try
                {
                    prx.ice_ping();
                    test(false);
                }
                catch (ObjectNotExistException)
                {
                    // Expected
                }

                try
                {
                    prx.getName();
                    test(false);
                }
                catch (ObjectNotExistException)
                {
                    // Expected
                }

                identity.name = "FacetNotExist";
                prx = MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
                try
                {
                    prx.ice_ping();
                    test(false);
                }
                catch (FacetNotExistException)
                {
                    // Expected
                }

                try
                {
                    prx.getName();
                    test(false);
                }
                catch (FacetNotExistException)
                {
                    // Expected
                }

                identity.category = "bar";
                for (int idx = 0; idx < 5; idx++)
                {
                    identity.name = names[idx];
                    prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));

                    try
                    {
                        prx.ice_ping();
                        test(false);
                    }
                    catch (Ice.ObjectNotExistException)
                    {
                        // Expected
                    }

                    try
                    {
                        prx.getName();
                        test(false);
                    }
                    catch (Ice.ObjectNotExistException)
                    {
                        // Expected
                    }
                }

                oa.removeDefaultServant("foo");
                identity.category = "foo";
                prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
                try
                {
                    prx.ice_ping();
                }
                catch (Ice.ObjectNotExistException)
                {
                    // Expected
                }

                output.WriteLine("ok");

                output.Write("testing default category... ");
                output.Flush();

                oa.addDefaultServant(servantD, "");

                r = oa.findDefaultServant("bar");
                test(r == null);

                r = oa.findDefaultServant("");
                test(r == servantD);

                for (int idx = 0; idx < 5; ++idx)
                {
                    identity.name = names[idx];
                    prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
                    prx.ice_ping();
                    test(prx.getName() == names[idx]);
                }

                output.WriteLine("ok");
            }
        }
    }
}
