// Copyright (c) ZeroC, Inc.

package test.IceDiscovery.simple;

import test.IceDiscovery.simple.Test.*;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.TestHelper helper, int num) {
        com.zeroc.Ice.Communicator communicator = helper.communicator();

        List<ControllerPrx> proxies = new ArrayList<>();
        List<ControllerPrx> indirectProxies = new ArrayList<>();
        for (int i = 0; i < num; i++) {
            String id = "controller" + i;
            proxies.add(ControllerPrx.createProxy(communicator, id));
            indirectProxies.add(ControllerPrx.createProxy(communicator, id + "@control" + i));
        }

        System.out.print("testing indirect proxies... ");
        System.out.flush();
        {
            for (ControllerPrx prx : indirectProxies) {
                prx.ice_ping();
            }
        }
        System.out.println("ok");

        System.out.print("testing well-known proxies... ");
        System.out.flush();
        {
            for (ControllerPrx prx : proxies) {
                prx.ice_ping();
            }
        }
        System.out.println("ok");

        System.out.print("testing object adapter registration... ");
        System.out.flush();
        {
            try {
                communicator.stringToProxy("object @ oa1").ice_ping();
                test(false);
            } catch (com.zeroc.Ice.NoEndpointException ex) {
            }

            proxies.get(0).activateObjectAdapter("oa", "oa1", "");

            try {
                communicator.stringToProxy("object @ oa1").ice_ping();
                test(false);
            } catch (com.zeroc.Ice.ObjectNotExistException ex) {
            }

            proxies.get(0).deactivateObjectAdapter("oa");

            try {
                communicator.stringToProxy("object @ oa1").ice_ping();
                test(false);
            } catch (com.zeroc.Ice.NoEndpointException ex) {
            }
        }
        System.out.println("ok");

        System.out.print("testing object adapter migration...");
        System.out.flush();
        {
            proxies.get(0).activateObjectAdapter("oa", "oa1", "");
            proxies.get(0).addObject("oa", "object");
            communicator.stringToProxy("object @ oa1").ice_ping();
            proxies.get(0).removeObject("oa", "object");
            proxies.get(0).deactivateObjectAdapter("oa");

            proxies.get(1).activateObjectAdapter("oa", "oa1", "");
            proxies.get(1).addObject("oa", "object");
            communicator.stringToProxy("object @ oa1").ice_ping();
            proxies.get(1).removeObject("oa", "object");
            proxies.get(1).deactivateObjectAdapter("oa");
        }
        System.out.println("ok");

        System.out.print("testing object migration...");
        System.out.flush();
        {
            proxies.get(0).activateObjectAdapter("oa", "oa1", "");
            proxies.get(1).activateObjectAdapter("oa", "oa2", "");

            proxies.get(0).addObject("oa", "object");
            communicator.stringToProxy("object @ oa1").ice_ping();
            communicator.stringToProxy("object").ice_ping();
            proxies.get(0).removeObject("oa", "object");

            proxies.get(1).addObject("oa", "object");
            communicator.stringToProxy("object @ oa2").ice_ping();
            communicator.stringToProxy("object").ice_ping();
            proxies.get(1).removeObject("oa", "object");

            try {
                communicator.stringToProxy("object @ oa1").ice_ping();
            } catch (com.zeroc.Ice.ObjectNotExistException ex) {
            }
            try {
                communicator.stringToProxy("object @ oa2").ice_ping();
            } catch (com.zeroc.Ice.ObjectNotExistException ex) {
            }

            proxies.get(0).deactivateObjectAdapter("oa");
            proxies.get(1).deactivateObjectAdapter("oa");
        }
        System.out.println("ok");

        System.out.print("testing replica groups...");
        System.out.flush();
        {
            proxies.get(0).activateObjectAdapter("oa", "oa1", "rg");
            proxies.get(1).activateObjectAdapter("oa", "oa2", "rg");
            proxies.get(2).activateObjectAdapter("oa", "oa3", "rg");

            proxies.get(0).addObject("oa", "object");
            proxies.get(1).addObject("oa", "object");
            proxies.get(2).addObject("oa", "object");

            communicator.stringToProxy("object @ oa1").ice_ping();
            communicator.stringToProxy("object @ oa2").ice_ping();
            communicator.stringToProxy("object @ oa3").ice_ping();

            communicator.stringToProxy("object @ rg").ice_ping();

            Set<String> adapterIds = new HashSet<>();
            adapterIds.add("oa1");
            adapterIds.add("oa2");
            adapterIds.add("oa3");
            var intf = TestIntfPrx.createProxy(communicator, "object");
            intf = intf.ice_connectionCached(false).ice_locatorCacheTimeout(0);
            while (!adapterIds.isEmpty()) {
                adapterIds.remove(intf.getAdapterId());
            }

            while (true) {
                adapterIds.add("oa1");
                adapterIds.add("oa2");
                adapterIds.add("oa3");
                intf =
                        TestIntfPrx.createProxy(communicator, "object @ rg")
                                .ice_connectionCached(false);
                int nRetry = 100;
                while (!adapterIds.isEmpty() && --nRetry > 0) {
                    adapterIds.remove(intf.getAdapterId());
                }
                if (nRetry > 0) {
                    break;
                }

                // The previous locator lookup probably didn't return all the replicas... try again.
                communicator.stringToProxy("object @ rg").ice_locatorCacheTimeout(0).ice_ping();
            }

            proxies.get(0).deactivateObjectAdapter("oa");
            proxies.get(1).deactivateObjectAdapter("oa");
            test("oa3".equals(TestIntfPrx.createProxy(communicator, "object @ rg").getAdapterId()));
            proxies.get(2).deactivateObjectAdapter("oa");

            proxies.get(0).activateObjectAdapter("oa", "oa1", "rg");
            proxies.get(0).addObject("oa", "object");
            test("oa1".equals(TestIntfPrx.createProxy(communicator, "object @ rg").getAdapterId()));
            proxies.get(0).deactivateObjectAdapter("oa");
        }
        System.out.println("ok");

        System.out.print("testing invalid lookup endpoints... ");
        System.out.flush();
        {
            String multicast;
            if ("1".equals(communicator.getProperties().getIceProperty("Ice.IPv6"))) {
                multicast = "\"ff15::1\"";
            } else {
                multicast = "239.255.0.1";
            }

            {
                com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
                initData.properties = communicator.getProperties()._clone();
                initData.properties.setProperty(
                        "IceDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");
                com.zeroc.Ice.Communicator comm = com.zeroc.Ice.Util.initialize(initData);
                test(comm.getDefaultLocator() != null);
                try {
                    comm.stringToProxy("controller0@control0").ice_ping();
                    test(false);
                } catch (com.zeroc.Ice.LocalException ex) {
                }
                comm.destroy();
            }
            {
                com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
                initData.properties = communicator.getProperties()._clone();
                String intf = initData.properties.getIceProperty("IceDiscovery.Interface");
                if (!intf.isEmpty()) {
                    intf = " --interface \"" + intf + "\"";
                }
                String port = initData.properties.getIceProperty("IceDiscovery.Port");
                initData.properties.setProperty(
                        "IceDiscovery.Lookup",
                        "udp -h " +
                                multicast +
                                " --interface unknown:" +
                                "udp -h " +
                                multicast +
                                " -p " +
                                port +
                                intf);
                com.zeroc.Ice.Communicator comm = com.zeroc.Ice.Util.initialize(initData);
                test(comm.getDefaultLocator() != null);
                comm.stringToProxy("controller0@control0").ice_ping();
                comm.destroy();
            }
        }
        System.out.println("ok");

        System.out.print("shutting down... ");
        System.out.flush();
        for (ControllerPrx prx : proxies) {
            prx.shutdown();
        }
        System.out.println("ok");
    }

    private AllTests() {
    }
}
