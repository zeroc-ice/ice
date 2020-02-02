//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using Ice.defaultServant.Test;

namespace Ice.defaultServant
{
    public class AllTests : global::Test.AllTests
    {
        public static void
        allTests(global::Test.TestHelper helper)
        {
            var output = helper.getWriter();
            Communicator communicator = helper.communicator();
            ObjectAdapter oa = communicator.CreateObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
            oa.Activate();

            var servantI = new MyObject();

            //
            // Register default servant with category "foo"
            //
            oa.AddDefaultServant(servantI, "foo");

            //
            // Start test
            //
            output.Write("testing single category... ");
            output.Flush();

            IObject r = oa.FindDefaultServant("foo");
            test(r == servantI);

            r = oa.FindDefaultServant("bar");
            test(r == null);

            Identity identity = new Identity("", "foo");

            string[] names = new string[] { "foo", "bar", "x", "y", "abcdefg" };

            IMyObjectPrx prx = null;
            for (int idx = 0; idx < 5; ++idx)
            {
                identity = new Identity(names[idx], identity.Category);
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
                prx.IcePing();
                test(prx.getName() == names[idx]);
            }

            identity = new Identity("ObjectNotExist", identity.Category);
            prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
            try
            {
                prx.IcePing();
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

            identity = new Identity("FacetNotExist", identity.Category);
            prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
            try
            {
                prx.IcePing();
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

            identity = new Identity(identity.Name, "bar");
            for (int idx = 0; idx < 5; idx++)
            {
                identity = new Identity(names[idx], identity.Category);
                prx = oa.CreateProxy(identity, Test.IMyObjectPrx.Factory);

                try
                {
                    prx.IcePing();
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

            oa.RemoveDefaultServant("foo");
            identity =  new Identity(identity.Name, "foo");
            prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
            try
            {
                prx.IcePing();
            }
            catch (ObjectNotExistException)
            {
                // Expected
            }

            output.WriteLine("ok");

            output.Write("testing default category... ");
            output.Flush();

            oa.AddDefaultServant(servantI, "");

            r = oa.FindDefaultServant("bar");
            test(r == null);

            r = oa.FindDefaultServant("");
            test(r == servantI);

            for (int idx = 0; idx < 5; ++idx)
            {
                identity = new Identity(names[idx], identity.Category);
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
                prx.IcePing();
                test(prx.getName() == names[idx]);
            }

            output.WriteLine("ok");
        }
    }
}
