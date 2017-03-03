// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceDiscovery.simple;

import test.IceDiscovery.simple.Test.*;

import java.util.List;
import java.util.ArrayList;

import java.util.Set;
import java.util.HashSet;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(Ice.Communicator communicator, int num)
    {
        List<ControllerPrx> proxies = new ArrayList<ControllerPrx>();
        List<ControllerPrx> indirectProxies = new ArrayList<ControllerPrx>();
        for(int i = 0; i < num; ++i)
        {
            String id = "controller" + i;
            proxies.add(ControllerPrxHelper.uncheckedCast(communicator.stringToProxy(id)));
            indirectProxies.add(ControllerPrxHelper.uncheckedCast(communicator.stringToProxy(id + "@control" + i)));
        }

        System.out.print("testing indirect proxies... ");
        System.out.flush();
        {
            for(ControllerPrx prx : indirectProxies)
            {
                prx.ice_ping();
            }
        }
        System.out.println("ok");

        System.out.print("testing well-known proxies... ");
        System.out.flush();
        {
            for(ControllerPrx prx : proxies)
            {
                prx.ice_ping();
            }
        }
        System.out.println("ok");

        System.out.print("testing object adapter registration... ");
        System.out.flush();
        {
            try
            {
                communicator.stringToProxy("object @ oa1").ice_ping();
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
            }

            proxies.get(0).activateObjectAdapter("oa", "oa1", "");

            try
            {
                communicator.stringToProxy("object @ oa1").ice_ping();
                test(false);
            }
            catch(Ice.ObjectNotExistException ex)
            {
            }

            proxies.get(0).deactivateObjectAdapter("oa");

            try
            {
                communicator.stringToProxy("object @ oa1").ice_ping();
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
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

            try
            {
                communicator.stringToProxy("object @ oa1").ice_ping();
            }
            catch(Ice.ObjectNotExistException ex)
            {
            }
            try
            {
                communicator.stringToProxy("object @ oa2").ice_ping();
            }
            catch(Ice.ObjectNotExistException ex)
            {
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

            Set<String> adapterIds = new HashSet<String>();
            adapterIds.add("oa1");
            adapterIds.add("oa2");
            adapterIds.add("oa3");
            TestIntfPrx intf = TestIntfPrxHelper.uncheckedCast(communicator.stringToProxy("object"));
            intf = (TestIntfPrx)intf.ice_connectionCached(false).ice_locatorCacheTimeout(0);
            while(!adapterIds.isEmpty())
            {
                adapterIds.remove(intf.getAdapterId());
            }

            while(true)
            {
                adapterIds.add("oa1");
                adapterIds.add("oa2");
                adapterIds.add("oa3");
                intf = TestIntfPrxHelper.uncheckedCast(
                    communicator.stringToProxy("object @ rg").ice_connectionCached(false));
                int nRetry = 100;
                while(!adapterIds.isEmpty() && --nRetry > 0)
                {
                    adapterIds.remove(intf.getAdapterId());
                }
                if(nRetry > 0)
                {
                    break;
                }

                // The previous locator lookup probably didn't return all the replicas... try again.
                communicator.stringToProxy("object @ rg").ice_locatorCacheTimeout(0).ice_ping();
            }

            proxies.get(0).deactivateObjectAdapter("oa");
            proxies.get(1).deactivateObjectAdapter("oa");
            test(TestIntfPrxHelper.uncheckedCast(
                     communicator.stringToProxy("object @ rg")).getAdapterId().equals("oa3"));
            proxies.get(2).deactivateObjectAdapter("oa");

            proxies.get(0).activateObjectAdapter("oa", "oa1", "rg");
            proxies.get(0).addObject("oa", "object");
            test(TestIntfPrxHelper.uncheckedCast(
                     communicator.stringToProxy("object @ rg")).getAdapterId().equals("oa1"));
            proxies.get(0).deactivateObjectAdapter("oa");
        }
        System.out.println("ok");

        System.out.print("shutting down... ");
        System.out.flush();
        for(ControllerPrx prx : proxies)
        {
            prx.shutdown();
        }
        System.out.println("ok");
    }
}
