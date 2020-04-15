//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Collections.Generic;
using Ice;

public class AllTests
{
    public static void
    allTests(TestHelper helper, int num)
    {
        var output = helper.GetWriter();
        Communicator? communicator = helper.Communicator();
        TestHelper.Assert(communicator != null);
        var proxies = new List<IControllerPrx>();
        var indirectProxies = new List<IControllerPrx>();
        for (int i = 0; i < num; ++i)
        {
            string id = "controller" + i;
            proxies.Add(IControllerPrx.Parse(id, communicator));
            indirectProxies.Add(IControllerPrx.Parse($"{id}@control{i}", communicator));
        }

        output.Write("testing indirect proxies... ");
        output.Flush();
        {
            foreach (IControllerPrx prx in indirectProxies)
            {
                prx.IcePing();
            }
        }
        output.WriteLine("ok");

        output.Write("testing well-known proxies... ");
        output.Flush();
        {
            foreach (IControllerPrx prx in proxies)
            {
                prx.IcePing();
            }
        }
        output.WriteLine("ok");

        output.Write("testing object adapter registration... ");
        output.Flush();
        {
            try
            {
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
            }

            proxies[0].activateObjectAdapter("oa", "oa1", "");

            try
            {
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
            }

            proxies[0].deactivateObjectAdapter("oa");

            try
            {
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
            }
        }
        output.WriteLine("ok");

        output.Write("testing object adapter migration...");
        output.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa1", "");
            proxies[0].addObject("oa", "object");
            IObjectPrx.Parse("object @ oa1", communicator).IcePing();
            proxies[0].removeObject("oa", "object");
            proxies[0].deactivateObjectAdapter("oa");

            proxies[1].activateObjectAdapter("oa", "oa1", "");
            proxies[1].addObject("oa", "object");
            IObjectPrx.Parse("object @ oa1", communicator).IcePing();
            proxies[1].removeObject("oa", "object");
            proxies[1].deactivateObjectAdapter("oa");
        }
        output.WriteLine("ok");

        output.Write("testing object migration...");
        output.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa1", "");
            proxies[1].activateObjectAdapter("oa", "oa2", "");

            proxies[0].addObject("oa", "object");
            IObjectPrx.Parse("object @ oa1", communicator).IcePing();
            IObjectPrx.Parse("object", communicator).IcePing();
            proxies[0].removeObject("oa", "object");

            proxies[1].addObject("oa", "object");
            IObjectPrx.Parse("object @ oa2", communicator).IcePing();
            IObjectPrx.Parse("object", communicator).IcePing();
            proxies[1].removeObject("oa", "object");

            try
            {
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
            }
            catch (ObjectNotExistException)
            {
            }
            try
            {
                IObjectPrx.Parse("object @ oa2", communicator).IcePing();
            }
            catch (ObjectNotExistException)
            {
            }

            proxies[0].deactivateObjectAdapter("oa");
            proxies[1].deactivateObjectAdapter("oa");
        }
        output.WriteLine("ok");

        output.Write("testing replica groups...");
        output.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa1", "rg");
            proxies[1].activateObjectAdapter("oa", "oa2", "rg");
            proxies[2].activateObjectAdapter("oa", "oa3", "rg");

            proxies[0].addObject("oa", "object");
            proxies[1].addObject("oa", "object");
            proxies[2].addObject("oa", "object");

            IObjectPrx.Parse("object @ oa1", communicator).IcePing();
            IObjectPrx.Parse("object @ oa2", communicator).IcePing();
            IObjectPrx.Parse("object @ oa3", communicator).IcePing();

            IObjectPrx.Parse("object @ rg", communicator).IcePing();

            List<string> adapterIds = new List<string>();
            adapterIds.Add("oa1");
            adapterIds.Add("oa2");
            adapterIds.Add("oa3");
            ITestIntfPrx intf = ITestIntfPrx.Parse("object", communicator).Clone(
                cacheConnection: false,
                locatorCacheTimeout: 0);
            while (adapterIds.Count > 0)
            {
                adapterIds.Remove(intf.getAdapterId());
            }

            while (true)
            {
                adapterIds.Add("oa1");
                adapterIds.Add("oa2");
                adapterIds.Add("oa3");
                intf = ITestIntfPrx.Parse("object @ rg", communicator).Clone(cacheConnection: false);
                int nRetry = 100;
                while (adapterIds.Count > 0 && --nRetry > 0)
                {
                    adapterIds.Remove(intf.getAdapterId());
                }
                if (nRetry > 0)
                {
                    break;
                }

                // The previous locator lookup probably didn't return all the replicas... try again.
                IObjectPrx.Parse("object @ rg", communicator).Clone(locatorCacheTimeout: 0).IcePing();
            }

            proxies[0].deactivateObjectAdapter("oa");
            proxies[1].deactivateObjectAdapter("oa");
            TestHelper.Assert(ITestIntfPrx.Parse("object @ rg", communicator).getAdapterId().Equals("oa3"));
            proxies[2].deactivateObjectAdapter("oa");

            proxies[0].activateObjectAdapter("oa", "oa1", "rg");
            proxies[0].addObject("oa", "object");
            TestHelper.Assert(ITestIntfPrx.Parse("object @ rg", communicator).getAdapterId().Equals("oa1"));
            proxies[0].deactivateObjectAdapter("oa");
        }
        output.WriteLine("ok");

        output.Write("testing invalid lookup endpoints... ");
        output.Flush();
        {
            string multicast;
            if (communicator.GetProperty("Ice.IPv6") == "1")
            {
                multicast = "\"ff15::1\"";
            }
            else
            {
                multicast = "239.255.0.1";
            }

            {
                var properties = communicator.GetProperties();
                properties["IceDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
                Communicator comm = new Communicator(properties);
                TestHelper.Assert(comm.DefaultLocator != null);
                try
                {
                    IObjectPrx.Parse("controller0@control0", comm).IcePing();
                    TestHelper.Assert(false);
                }
                catch (System.Exception)
                {
                }
                comm.Destroy();
            }
            {
                var properties = communicator.GetProperties();
                string intf = communicator.GetProperty("IceDiscovery.Interface") ?? "";
                if (intf != "")
                {
                    intf = $" --interface \"{intf}\"";
                }
                string port = communicator.GetProperty("IceDiscovery.Port") ?? "";
                properties["IceDiscovery.Lookup"] =
                    $"udp -h {multicast} --interface unknown:udp -h {multicast} -p {port}{intf}";
                var comm = new Communicator(properties);
                TestHelper.Assert(comm.DefaultLocator != null);
                try
                {
                    IObjectPrx.Parse("controller0@control0", comm).IcePing();
                    TestHelper.Assert(false);
                }
                catch (NoEndpointException)
                {
                }
                comm.Destroy();
            }
        }
        output.WriteLine("ok");

        output.Write("shutting down... ");
        output.Flush();
        foreach (IControllerPrx prx in proxies)
        {
            prx.shutdown();
        }
        output.WriteLine("ok");
    }
}
