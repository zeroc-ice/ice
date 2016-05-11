// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Collections.Generic;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public static void
    allTests(Ice.Communicator communicator, int num)
    {
        List<ControllerPrx> proxies = new List<ControllerPrx>();
        List<ControllerPrx> indirectProxies = new List<ControllerPrx>();
        for(int i = 0; i < num; ++i)
        {
            string id = "controller" + i;
            proxies.Add(ControllerPrxHelper.uncheckedCast(communicator.stringToProxy(id)));
            indirectProxies.Add(ControllerPrxHelper.uncheckedCast(communicator.stringToProxy(id + "@control" + i)));
        }

        Console.Out.Write("testing indirect proxies... ");
        Console.Out.Flush();
        {
            foreach(ControllerPrx prx in indirectProxies)
            {
                prx.ice_ping();
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing well-known proxies... ");
        Console.Out.Flush();
        {
            foreach(ControllerPrx prx in proxies)
            {
                prx.ice_ping();
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing object adapter registration... ");
        Console.Out.Flush();
        {
            try
            {
                communicator.stringToProxy("object @ oa1").ice_ping();
            }
            catch(Ice.NoEndpointException)
            {
            }

            proxies[0].activateObjectAdapter("oa", "oa1", "");

            try
            {
                communicator.stringToProxy("object @ oa1").ice_ping();
            }
            catch(Ice.ObjectNotExistException)
            {
            }

            proxies[0].deactivateObjectAdapter("oa");

            try
            {
                communicator.stringToProxy("object @ oa1").ice_ping();
            }
            catch(Ice.NoEndpointException)
            {
            }
        }
        Console.Out.WriteLine("ok");
    
        Console.Out.Write("testing object adapter migration...");
        Console.Out.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa1", "");
            proxies[0].addObject("oa", "object");
            communicator.stringToProxy("object @ oa1").ice_ping();
            proxies[0].removeObject("oa", "object");
            proxies[0].deactivateObjectAdapter("oa");

            proxies[1].activateObjectAdapter("oa", "oa1", "");
            proxies[1].addObject("oa", "object");
            communicator.stringToProxy("object @ oa1").ice_ping();
            proxies[1].removeObject("oa", "object");
            proxies[1].deactivateObjectAdapter("oa");
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing object migration...");
        Console.Out.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa1", "");
            proxies[1].activateObjectAdapter("oa", "oa2", "");

            proxies[0].addObject("oa", "object");
            communicator.stringToProxy("object @ oa1").ice_ping();
            communicator.stringToProxy("object").ice_ping();
            proxies[0].removeObject("oa", "object");

            proxies[1].addObject("oa", "object");
            communicator.stringToProxy("object @ oa2").ice_ping();
            communicator.stringToProxy("object").ice_ping();
            proxies[1].removeObject("oa", "object");

            try
            {
                communicator.stringToProxy("object @ oa1").ice_ping();
            }
            catch(Ice.ObjectNotExistException)
            {
            }
            try
            {
                communicator.stringToProxy("object @ oa2").ice_ping();
            }
            catch(Ice.ObjectNotExistException)
            {
            }

            proxies[0].deactivateObjectAdapter("oa");
            proxies[1].deactivateObjectAdapter("oa");
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing replica groups...");
        Console.Out.Flush();
        {
            proxies[0].activateObjectAdapter("oa", "oa1", "rg");
            proxies[1].activateObjectAdapter("oa", "oa2", "rg");
            proxies[2].activateObjectAdapter("oa", "oa3", "rg");

            proxies[0].addObject("oa", "object");
            proxies[1].addObject("oa", "object");
            proxies[2].addObject("oa", "object");

            communicator.stringToProxy("object @ oa1").ice_ping();
            communicator.stringToProxy("object @ oa2").ice_ping();
            communicator.stringToProxy("object @ oa3").ice_ping();

            communicator.stringToProxy("object @ rg").ice_ping();

            List<string> adapterIds = new List<string>();
            adapterIds.Add("oa1");
            adapterIds.Add("oa2");
            adapterIds.Add("oa3");
            TestIntfPrx intf = TestIntfPrxHelper.uncheckedCast(communicator.stringToProxy("object"));
            intf = (TestIntfPrx)intf.ice_connectionCached(false).ice_locatorCacheTimeout(0);
            while(adapterIds.Count > 0)
            {
                adapterIds.Remove(intf.getAdapterId());
            }

            while(true)
            {
                adapterIds.Add("oa1");
                adapterIds.Add("oa2");
                adapterIds.Add("oa3");
                intf = TestIntfPrxHelper.uncheckedCast(
                    communicator.stringToProxy("object @ rg").ice_connectionCached(false));
                int nRetry = 100;
                while(adapterIds.Count > 0 && --nRetry > 0)
                {
                    adapterIds.Remove(intf.getAdapterId());
                }
                if(nRetry > 0)
                {
                    break;
                }

                // The previous locator lookup probably didn't return all the replicas... try again.
                communicator.stringToProxy("object @ rg").ice_locatorCacheTimeout(0).ice_ping();
            }

            proxies[0].deactivateObjectAdapter("oa");
            proxies[1].deactivateObjectAdapter("oa");
            test(TestIntfPrxHelper.uncheckedCast(
                     communicator.stringToProxy("object @ rg")).getAdapterId().Equals("oa3"));
            proxies[2].deactivateObjectAdapter("oa");

            proxies[0].activateObjectAdapter("oa", "oa1", "rg");
            proxies[0].addObject("oa", "object");
            test(TestIntfPrxHelper.uncheckedCast(
                     communicator.stringToProxy("object @ rg")).getAdapterId().Equals("oa1"));
            proxies[0].deactivateObjectAdapter("oa");
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("shutting down... ");
        Console.Out.Flush();
        foreach(ControllerPrx prx in proxies)
        {
            prx.shutdown();
        }
        Console.Out.WriteLine("ok");
    }
}
