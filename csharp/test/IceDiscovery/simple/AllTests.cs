//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Collections.Generic;
using Ice;

public class AllTests : Test.AllTests
{
    public static void
    allTests(Test.TestHelper helper, int num)
    {
        var output = helper.getWriter();
        Ice.Communicator communicator = helper.communicator();
        List<ControllerPrx> proxies = new List<ControllerPrx>();
        List<ControllerPrx> indirectProxies = new List<ControllerPrx>();
        for (int i = 0; i < num; ++i)
        {
            string id = "controller" + i;
            proxies.Add(ControllerPrx.Parse(id, communicator));
            indirectProxies.Add(ControllerPrx.Parse($"{id}@control{i}", communicator));
        }

        output.Write("testing indirect proxies... ");
        output.Flush();
        {
            foreach (ControllerPrx prx in indirectProxies)
            {
                prx.IcePing();
            }
        }
        output.WriteLine("ok");

        output.Write("testing well-known proxies... ");
        output.Flush();
        {
            foreach (ControllerPrx prx in proxies)
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
                test(false);
            }
            catch (NoEndpointException)
            {
            }

            proxies[0].activateObjectAdapter("oa", "oa1", "");

            try
            {
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                test(false);
            }
            catch (ObjectNotExistException)
            {
            }

            proxies[0].deactivateObjectAdapter("oa");

            try
            {
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                test(false);
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
            TestIntfPrx intf = TestIntfPrx.Parse("object", communicator).Clone(
                connectionCached: false,
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
                intf = TestIntfPrx.Parse("object @ rg", communicator).Clone(connectionCached: false);
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
            test(TestIntfPrx.Parse("object @ rg", communicator).getAdapterId().Equals("oa3"));
            proxies[2].deactivateObjectAdapter("oa");

            proxies[0].activateObjectAdapter("oa", "oa1", "rg");
            proxies[0].addObject("oa", "object");
            test(TestIntfPrx.Parse("object @ rg", communicator).getAdapterId().Equals("oa1"));
            proxies[0].deactivateObjectAdapter("oa");
        }
        output.WriteLine("ok");

        output.Write("testing invalid lookup endpoints... ");
        output.Flush();
        {
            String multicast;
            if (communicator.getProperties().getProperty("Ice.IPv6").Equals("1"))
            {
                multicast = "\"ff15::1\"";
            }
            else
            {
                multicast = "239.255.0.1";
            }

            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().ice_clone_();
                initData.properties.setProperty("IceDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");
                Ice.Communicator comm = Ice.Util.initialize(initData);
                test(comm.getDefaultLocator() != null);
                try
                {
                    IObjectPrx.Parse("controller0@control0", comm).IcePing();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                comm.destroy();
            }
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().ice_clone_();
                string intf = initData.properties.getProperty("IceDiscovery.Interface");
                if (!intf.Equals(""))
                {
                    intf = " --interface \"" + intf + "\"";
                }
                string port = initData.properties.getProperty("IceDiscovery.Port");
                initData.properties.setProperty("IceDiscovery.Lookup",
                                                 "udp -h " + multicast + " --interface unknown:" +
                                                 "udp -h " + multicast + " -p " + port + intf);
                Communicator comm = Util.initialize(initData);
                test(comm.getDefaultLocator() != null);
                IObjectPrx.Parse("controller0@control0", comm).IcePing();
                comm.destroy();
            }
        }
        output.WriteLine("ok");

        output.Write("shutting down... ");
        output.Flush();
        foreach (ControllerPrx prx in proxies)
        {
            prx.shutdown();
        }
        output.WriteLine("ok");
    }
}
