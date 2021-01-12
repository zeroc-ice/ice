// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.DefaultServant
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            TextWriter output = helper.Output;
            Communicator communicator = helper.Communicator;

            ObjectAdapter oa = communicator.CreateObjectAdapterWithEndpoints("MyOA",
                helper.GetTestEndpoint(ephemeral: true));
            await oa.ActivateAsync();

            output.Write("testing single category... ");
            output.Flush();

            var servant = new MyObject();

            oa.AddDefaultForCategory("foo", servant);
            try
            {
                oa.AddDefaultForCategory("foo", new MyObject());
                TestHelper.Assert(false); // duplicate registration not allowed
            }
            catch (ArgumentException)
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
            foreach (string name in names)
            {
                identity = new Identity(name, identity.Category);
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
                prx.IcePing();
                TestHelper.Assert(prx.GetName() == name);
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
            foreach (string name in names)
            {
                identity = new Identity(name, identity.Category);
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
            catch (ArgumentException)
            {
                // Expected
            }

            oa.AddDefaultForCategory("", servant); // associated with empty category

            r = oa.Find("bar");
            TestHelper.Assert(r == servant);

            r = oa.Find("x/y");
            TestHelper.Assert(r == defaultServant);

            foreach (string name in names)
            {
                identity = new Identity(name, "");
                prx = oa.CreateProxy(identity, IMyObjectPrx.Factory);
                prx.IcePing();
                TestHelper.Assert(prx.GetName() == name);
            }

            removed = oa.RemoveDefault();
            TestHelper.Assert(removed == defaultServant);
            removed = oa.RemoveDefault();
            TestHelper.Assert(removed == null);

            output.WriteLine("ok");
        }
    }
}
