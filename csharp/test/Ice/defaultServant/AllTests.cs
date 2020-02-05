//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using Ice.DefaultServant.Test;

namespace Ice.DefaultServant
{
    public class AllTests : global::Test.AllTests
    {
        public static void
        Run(global::Test.TestHelper helper)
        {
            var output = helper.getWriter();
            Communicator communicator = helper.communicator();
            ObjectAdapter oa = communicator.CreateObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
            oa.Activate();

            output.Write("testing single category... ");
            output.Flush();

            var servant = new MyObject();

            oa.AddDefaultForCategory("foo", servant);
            try
            {
                oa.AddDefaultForCategory("foo", new MyObject());
                test(false); // duplicate registration not allowed
            }
            catch (System.ArgumentException)
            {
                // Expected
            }

            IObject? r = oa.Find("foo");
            test(r == null);
            r = oa.Find("foo/someId");
            test(r == servant);
            r = oa.Find("bar/someId");
            test(r == null);

            Identity identity = new Identity("", "foo");
            string[] names = new string[] { "foo", "bar", "x", "y", "abcdefg" };

            IMyObjectPrx? prx = null;
            for (int idx = 0; idx < 5; ++idx)
            {
                identity = new Identity(names[idx], identity.Category);
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
                prx.IcePing();
                test(prx.GetName() == names[idx]);
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
                prx.GetName();
                test(false);
            }
            catch (ObjectNotExistException)
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
                    prx.GetName();
                    test(false);
                }
                catch (Ice.ObjectNotExistException)
                {
                    // Expected
                }
            }

            IObject? removed = oa.RemoveDefaultForCategory("foo");
            test(removed == servant);
            removed = oa.RemoveDefaultForCategory("foo");
            test(removed == null);
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

            output.Write("testing default servant... ");
            output.Flush();

            var defaultServant = new MyObject();

            oa.AddDefault(defaultServant);
            try
            {
                oa.AddDefault(servant);
                test(false);
            }
            catch (System.ArgumentException)
            {
                // Expected
            }

            oa.AddDefaultForCategory("", servant); // associated with empty category

            r = oa.Find("bar");
            test(r == servant);

            r = oa.Find("x/y");
            test(r == defaultServant);

            for (int idx = 0; idx < 5; ++idx)
            {
                identity = new Identity(names[idx], "");
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
                prx.IcePing();
                test(prx.GetName() == names[idx]);
            }

            removed = oa.RemoveDefault();
            test(removed == defaultServant);
            removed = oa.RemoveDefault();
            test(removed == null);

            output.WriteLine("ok");
        }
    }
}
