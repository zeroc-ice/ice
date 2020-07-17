//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using ZeroC.Ice;
using Test;

namespace ZeroC.IceDiscovery.Test.Simple
{
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

                proxies[0].ActivateObjectAdapter("oa", "oa1", "");

                try
                {
                    IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }

                proxies[0].DeactivateObjectAdapter("oa");

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
                proxies[0].ActivateObjectAdapter("oa", "oa1", "");
                proxies[0].AddObject("oa", "object");
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                proxies[0].RemoveObject("oa", "object");
                proxies[0].DeactivateObjectAdapter("oa");

                proxies[1].ActivateObjectAdapter("oa", "oa1", "");
                proxies[1].AddObject("oa", "object");
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                proxies[1].RemoveObject("oa", "object");
                proxies[1].DeactivateObjectAdapter("oa");
            }
            output.WriteLine("ok");

            output.Write("testing object migration...");
            output.Flush();
            {
                proxies[0].ActivateObjectAdapter("oa", "oa1", "");
                proxies[1].ActivateObjectAdapter("oa", "oa2", "");

                proxies[0].AddObject("oa", "object");
                IObjectPrx.Parse("object @ oa1", communicator).IcePing();
                IObjectPrx.Parse("object", communicator).IcePing();
                proxies[0].RemoveObject("oa", "object");

                proxies[1].AddObject("oa", "object");
                IObjectPrx.Parse("object @ oa2", communicator).IcePing();
                IObjectPrx.Parse("object", communicator).IcePing();
                proxies[1].RemoveObject("oa", "object");

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

                proxies[0].DeactivateObjectAdapter("oa");
                proxies[1].DeactivateObjectAdapter("oa");
            }
            output.WriteLine("ok");

            output.Write("testing replica groups...");
            output.Flush();
            {
                proxies[0].ActivateObjectAdapter("oa", "oa1", "rg");
                proxies[1].ActivateObjectAdapter("oa", "oa2", "rg");
                proxies[2].ActivateObjectAdapter("oa", "oa3", "rg");

                proxies[0].AddObject("oa", "object");
                proxies[1].AddObject("oa", "object");
                proxies[2].AddObject("oa", "object");

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
                    locatorCacheTimeout: TimeSpan.Zero);
                while (adapterIds.Count > 0)
                {
                    adapterIds.Remove(intf.GetAdapterId());
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
                        adapterIds.Remove(intf.GetAdapterId());
                    }
                    if (nRetry > 0)
                    {
                        break;
                    }

                    // The previous locator lookup probably didn't return all the replicas... try again.
                    IObjectPrx.Parse("object @ rg", communicator).Clone(locatorCacheTimeout: TimeSpan.Zero).IcePing();
                }

                proxies[0].DeactivateObjectAdapter("oa");
                proxies[1].DeactivateObjectAdapter("oa");
                TestHelper.Assert(ITestIntfPrx.Parse("object @ rg", communicator).GetAdapterId().Equals("oa3"));
                proxies[2].DeactivateObjectAdapter("oa");

                proxies[0].ActivateObjectAdapter("oa", "oa1", "rg");
                proxies[0].AddObject("oa", "object");
                TestHelper.Assert(ITestIntfPrx.Parse("object @ rg", communicator).GetAdapterId().Equals("oa1"));
                proxies[0].DeactivateObjectAdapter("oa");
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
                    using var comm = new Communicator(properties);
                    TestHelper.Assert(comm.DefaultLocator != null);
                    try
                    {
                        IObjectPrx.Parse("controller0@control0", comm).IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (Exception)
                    {
                    }
                }
                {
                    var properties = communicator.GetProperties();
                    string intf = communicator.GetProperty("IceDiscovery.Interface") ?? "";
                    if (intf.Length > 0)
                    {
                        intf = $" --interface \"{intf}\"";
                    }
                    string port = communicator.GetProperty("IceDiscovery.Port") ?? "";
                    properties["IceDiscovery.Lookup"] =
                        $"udp -h {multicast} --interface unknown:udp -h {multicast} -p {port}{intf}";
                    using var comm = new Communicator(properties);
                    TestHelper.Assert(comm.DefaultLocator != null);
                    IObjectPrx.Parse("controller0@control0", comm).IcePing();
                }
            }
            output.WriteLine("ok");

            output.Write("shutting down... ");
            output.Flush();
            foreach (IControllerPrx prx in proxies)
            {
                prx.Shutdown();
            }
            output.WriteLine("ok");
        }
    }
}
