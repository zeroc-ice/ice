//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
                Ice.Communicator communicator = helper.communicator();
                Ice.ObjectAdapter oa = communicator.createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
                oa.activate();

                Ice.Object servant = new MyObjectI();

                //
                // Register default servant with category "foo"
                //
                oa.addDefaultServant(servant, "foo");

                //
                // Start test
                //
                output.Write("testing single category... ");
                output.Flush();

                Ice.Object r = oa.findDefaultServant("foo");
                test(r == servant);

                r = oa.findDefaultServant("bar");
                test(r == null);

                Ice.Identity identity = new Ice.Identity();
                identity.category = "foo";

                string[] names = new string[] { "foo", "bar", "x", "y", "abcdefg" };

                Test.MyObjectPrx prx = null;
                for(int idx = 0; idx < 5; ++idx)
                {
                    identity.name = names[idx];
                    prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
                    prx.ice_ping();
                    test(prx.getName() == names[idx]);
                }

                identity.name = "ObjectNotExist";
                prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
                try
                {
                    prx.ice_ping();
                    test(false);
                }
                catch(Ice.ObjectNotExistException)
                {
                    // Expected
                }

                try
                {
                    prx.getName();
                    test(false);
                }
                catch(Ice.ObjectNotExistException)
                {
                    // Expected
                }

                identity.name = "FacetNotExist";
                prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));
                try
                {
                    prx.ice_ping();
                    test(false);
                }
                catch(Ice.FacetNotExistException)
                {
                    // Expected
                }

                try
                {
                    prx.getName();
                    test(false);
                }
                catch(Ice.FacetNotExistException)
                {
                    // Expected
                }

                identity.category = "bar";
                for(int idx = 0; idx < 5; idx++)
                {
                    identity.name = names[idx];
                    prx = Test.MyObjectPrxHelper.uncheckedCast(oa.createProxy(identity));

                    try
                    {
                        prx.ice_ping();
                        test(false);
                    }
                    catch(Ice.ObjectNotExistException)
                    {
                        // Expected
                    }

                    try
                    {
                        prx.getName();
                        test(false);
                    }
                    catch(Ice.ObjectNotExistException)
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
                catch(Ice.ObjectNotExistException)
                {
                    // Expected
                }

                output.WriteLine("ok");

                output.Write("testing default category... ");
                output.Flush();

                oa.addDefaultServant(servant, "");

                r = oa.findDefaultServant("bar");
                test(r == null);

                r = oa.findDefaultServant("");
                test(r == servant);

                for(int idx = 0; idx < 5; ++idx)
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
