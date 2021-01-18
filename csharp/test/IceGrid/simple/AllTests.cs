// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.IceGrid.Test.Simple
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);
            Console.Out.Write("testing stringToProxy... ");
            Console.Out.Flush();
            string rf = "test @ TestAdapter";
            var obj = ITestIntfPrx.Parse(rf, communicator);
            Console.Out.WriteLine("ok");

            Console.Out.Write("pinging server... ");
            Console.Out.Flush();
            await obj.IcePingAsync();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing locator finder... ");
            var finderId = new Identity("LocatorFinder", "Ice");
            ILocatorFinderPrx finder = communicator.DefaultLocator!.Clone(ILocatorFinderPrx.Factory,
                                                                          identity: finderId);
            TestHelper.Assert(await finder.GetLocatorAsync() != null);
            Console.Out.WriteLine("ok");

            Console.Out.Write("shutting down server... ");
            Console.Out.Flush();
            await obj.ShutdownAsync();
            Console.Out.WriteLine("ok");
        }

        public static async Task RunWithDeployAsync(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);
            Console.Out.Write("testing stringToProxy... ");
            Console.Out.Flush();
            var obj = ITestIntfPrx.Parse("test @ TestAdapter", communicator);
            var obj2 = ITestIntfPrx.Parse("test", communicator);
            Console.Out.WriteLine("ok");

            Console.Out.Write("pinging server... ");
            Console.Out.Flush();
            obj.IcePing();
            obj2.IcePing();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing encoding versioning... ");
            Console.Out.Flush();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing reference with unknown identity... ");
            Console.Out.Flush();
            try
            {
                IObjectPrx.Parse("unknown/unknown", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
                // expected
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing reference with unknown adapter... ");
            Console.Out.Flush();
            try
            {
                IObjectPrx.Parse("test @ TestAdapterUnknown", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
                // expected
            }
            Console.Out.WriteLine("ok");

            var registry = IRegistryPrx.Parse(
                $"{communicator.DefaultLocator!.Identity.Category}/Registry", communicator);
            IAdminSessionPrx? session = registry.CreateAdminSession("foo", "bar");
            TestHelper.Assert(session != null);
            Connection connection = await session.GetConnectionAsync();
            connection.KeepAlive = true;

            IAdminPrx? admin = session.GetAdmin();
            TestHelper.Assert(admin != null);
            admin.EnableServer("server", false);
            admin.StopServer("server");

            Console.Out.Write("testing whether server is still reachable... ");
            Console.Out.Flush();
            try
            {
                obj.IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
            }
            try
            {
                obj2.IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
            }

            admin.EnableServer("server", true);
            obj.IcePing();
            obj2.IcePing();
            Console.Out.WriteLine("ok");

            admin.StopServer("server");
            session.Destroy();
        }
    }
}
