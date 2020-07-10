//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.IO;
using Test;

namespace ZeroC.Ice.Test.DefaultServant
{
    public class AllTests
    {
        public static void
        Run(TestHelper helper)
        {
            TextWriter output = helper.GetWriter();
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            ObjectAdapter oa = communicator.CreateObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
            oa.Activate();

            output.Write("testing single category... ");
            output.Flush();

            var servant = new MyObject();

            oa.AddDefaultForCategory("foo", servant);
            try
            {
                oa.AddDefaultForCategory("foo", new MyObject());
                TestHelper.Assert(false); // duplicate registration not allowed
            }
            catch (System.ArgumentException)
            {
                // Expected
            }

            IObject? r = oa.Find("foo");
            TestHelper.Assert(r == null);
            r = oa.Find("foo/someId");
            TestHelper.Assert(r == servant);
            r = oa.Find("bar/someId");
            TestHelper.Assert(r == null);

            var identity = new Identity("", "foo");
            string[] names = new string[] { "foo", "bar", "x", "y", "abcdefg" };

            IMyObjectPrx? prx = null;
            for (int idx = 0; idx < 5; ++idx)
            {
                identity = new Identity(names[idx], identity.Category);
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
                prx.IcePing();
                TestHelper.Assert(prx.GetName() == names[idx]);
            }

            identity = new Identity("ObjectNotExist", identity.Category);
            prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
            try
            {
                prx.IcePing();
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
                // Expected
            }

            try
            {
                prx.GetName();
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
                // Expected
            }

            identity = new Identity(identity.Name, "bar");
            for (int idx = 0; idx < 5; idx++)
            {
                identity = new Identity(names[idx], identity.Category);
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);

                try
                {
                    prx.IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                    // Expected
                }

                try
                {
                    prx.GetName();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                    // Expected
                }
            }

            IObject? removed = oa.RemoveDefaultForCategory("foo");
            TestHelper.Assert(removed == servant);
            removed = oa.RemoveDefaultForCategory("foo");
            TestHelper.Assert(removed == null);
            identity = new Identity(identity.Name, "foo");
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
                TestHelper.Assert(false);
            }
            catch (System.ArgumentException)
            {
                // Expected
            }

            oa.AddDefaultForCategory("", servant); // associated with empty category

            r = oa.Find("bar");
            TestHelper.Assert(r == servant);

            r = oa.Find("x/y");
            TestHelper.Assert(r == defaultServant);

            for (int idx = 0; idx < 5; ++idx)
            {
                identity = new Identity(names[idx], "");
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
                prx.IcePing();
                TestHelper.Assert(prx.GetName() == names[idx]);
            }

            removed = oa.RemoveDefault();
            TestHelper.Assert(removed == defaultServant);
            removed = oa.RemoveDefault();
            TestHelper.Assert(removed == null);

            output.WriteLine("ok");
        }
    }
}
