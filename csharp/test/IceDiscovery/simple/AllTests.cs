// Copyright (c) ZeroC, Inc.

using Test;

public class AllTests : Test.AllTests
{
    public static void
    allTests(Test.TestHelper helper, int num)
    {
        var output = helper.getWriter();
        Ice.Communicator communicator = helper.communicator();
        var proxies = new List<ControllerPrx>();
        var indirectProxies = new List<ControllerPrx>();
        for (int i = 0; i < num; ++i)
        {
            string id = "controller" + i;
            proxies.Add(ControllerPrxHelper.createProxy(communicator, id));
            indirectProxies.Add(ControllerPrxHelper.createProxy(communicator, id + "@control" + i));
        }

        output.Write("testing indirect proxies... ");
        output.Flush();
        {
            foreach (ControllerPrx prx in indirectProxies)
            {
                prx.ice_ping();
            }
        }
        output.WriteLine("ok");

        output.Write("testing well-known proxies... ");
        output.Flush();
        {
            foreach (ControllerPrx prx in proxies)
            {
                prx.ice_ping();
            }
        }
        output.WriteLine("ok");

        output.Write("testing object adapter registration... ");
        output.Flush();
        {
            try
            {
                communicator.stringToProxy("object @ oa10").ice_ping();
                test(false);
            }
            catch (Ice.NoEndpointException)
            {
            }

            proxies[0].activateObjectAdapter("oa", "oa10", "");

            try
            {
                communicator.stringToProxy("object @ oa10").ice_ping();
                test(false);
            }
            catch (Ice.ObjectNotExistException)
            {
            }

            proxies[0].deactivateObjectAdapter("oa");

            try
            {
                communicator.stringToProxy("object @ oa10").ice_ping();
                test(false);
            }
            catch (Ice.NoEndpointException)
            {
            }
        }
        output.WriteLine("ok");

        output.Write("testing object adapter migration...");
        output.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa21", "");
            proxies[0].addObject("oa", "object");
            communicator.stringToProxy("object @ oa21").ice_ping();
            proxies[0].removeObject("oa", "object");
            proxies[0].deactivateObjectAdapter("oa");

            proxies[1].activateObjectAdapter("oa", "oa21", "");
            proxies[1].addObject("oa", "object");
            communicator.stringToProxy("object @ oa21").ice_ping();
            proxies[1].removeObject("oa", "object");
            proxies[1].deactivateObjectAdapter("oa");
        }
        output.WriteLine("ok");

        output.Write("testing object migration...");
        output.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa31", "");
            proxies[1].activateObjectAdapter("oa", "oa32", "");

            proxies[0].addObject("oa", "object");
            communicator.stringToProxy("object @ oa31").ice_ping();
            communicator.stringToProxy("object").ice_ping();
            proxies[0].removeObject("oa", "object");

            proxies[1].addObject("oa", "object");
            communicator.stringToProxy("object @ oa32").ice_ping();
            communicator.stringToProxy("object").ice_ping();
            proxies[1].removeObject("oa", "object");

            try
            {
                communicator.stringToProxy("object @ oa31").ice_ping();
            }
            catch (Ice.ObjectNotExistException)
            {
            }
            try
            {
                communicator.stringToProxy("object @ oa32").ice_ping();
            }
            catch (Ice.ObjectNotExistException)
            {
            }

            proxies[0].deactivateObjectAdapter("oa");
            proxies[1].deactivateObjectAdapter("oa");
        }
        output.WriteLine("ok");

        output.Write("testing replica groups...");
        output.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa41", "rg");
            proxies[1].activateObjectAdapter("oa", "oa42", "rg");
            proxies[2].activateObjectAdapter("oa", "oa43", "rg");

            proxies[0].addObject("oa", "object");
            proxies[1].addObject("oa", "object");
            proxies[2].addObject("oa", "object");

            communicator.stringToProxy("object @ oa41").ice_ping();
            communicator.stringToProxy("object @ oa42").ice_ping();
            communicator.stringToProxy("object @ oa43").ice_ping();

            communicator.stringToProxy("object @ rg").ice_ping();

            var adapterIds = new List<string>
            {
                "oa41",
                "oa42",
                "oa43"
            };
            TestIntfPrx intf = TestIntfPrxHelper.uncheckedCast(communicator.stringToProxy("object"));
            intf = (TestIntfPrx)intf.ice_connectionCached(false).ice_locatorCacheTimeout(0);
            while (adapterIds.Count > 0)
            {
                adapterIds.Remove(intf.getAdapterId());
            }

            while (true)
            {
                adapterIds.Add("oa41");
                adapterIds.Add("oa42");
                adapterIds.Add("oa43");
                intf = TestIntfPrxHelper.uncheckedCast(
                    communicator.stringToProxy("object @ rg").ice_connectionCached(false));
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
                communicator.stringToProxy("object @ rg").ice_locatorCacheTimeout(0).ice_ping();
            }

            proxies[0].deactivateObjectAdapter("oa");
            proxies[1].deactivateObjectAdapter("oa");
            test(TestIntfPrxHelper.uncheckedCast(
                     communicator.stringToProxy("object @ rg")).getAdapterId() == "oa43");
            proxies[2].deactivateObjectAdapter("oa");

            proxies[0].activateObjectAdapter("oa", "oa41", "rg");
            proxies[0].addObject("oa", "object");
            test(TestIntfPrxHelper.uncheckedCast(
                     communicator.stringToProxy("object @ rg")).getAdapterId() == "oa41");
            proxies[0].deactivateObjectAdapter("oa");
        }
        output.WriteLine("ok");

        output.Write("testing invalid lookup endpoints... ");
        output.Flush();
        {
            string multicast;
            if (communicator.getProperties().getIceProperty("Ice.IPv6") == "1")
            {
                multicast = "\"ff15::1\"";
            }
            else
            {
                multicast = "239.255.0.1";
            }

            {
                var initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().Clone();
                initData.properties.setProperty("IceDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");
                using var comm = new Ice.Communicator(initData);
                test(comm.getDefaultLocator() != null);
                try
                {
                    comm.stringToProxy("controller0@control0").ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
            }
            {
                var initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties().Clone();
                string intf = initData.properties.getIceProperty("IceDiscovery.Interface");
                if (intf.Length > 0)
                {
                    intf = " --interface \"" + intf + "\"";
                }
                string port = initData.properties.getIceProperty("IceDiscovery.Port");
                initData.properties.setProperty("IceDiscovery.Lookup",
                                                 "udp -h " + multicast + " --interface unknown:" +
                                                 "udp -h " + multicast + " -p " + port + intf);
                using var comm = new Ice.Communicator(initData);
                test(comm.getDefaultLocator() != null);
                comm.stringToProxy("controller0@control0").ice_ping();
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
