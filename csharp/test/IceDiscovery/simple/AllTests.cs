// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using Test;

namespace ZeroC.Ice.Test.Discovery
{
    public static class AllTests
    {
        public static void Run(TestHelper helper, int num)
        {
            TextWriter output = helper.Output;
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);
            var proxies = new List<IControllerPrx>();
            var facetedProxies = new List<IControllerPrx>();
            var indirectProxies = new List<IControllerPrx>();
            bool ice1 = helper.Protocol == Protocol.Ice1;

            for (int i = 0; i < num; ++i)
            {
                proxies.Add(IControllerPrx.Parse(ice1 ? $"controller{i}" : $"ice:controller{i}", communicator));

                facetedProxies.Add(IControllerPrx.Parse(
                    ice1 ? $"faceted-controller{i} -f abc" : $"ice:faceted-controller{i}#abc",
                    communicator));

                indirectProxies.Add(
                    IControllerPrx.Parse(ice1 ? $"controller{i} @ control{i}" : $"ice:control{i}//controller{i}",
                                         communicator));
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

            output.Write("testing faceted well-known proxies... ");
            output.Flush();
            {
                foreach (IControllerPrx prx in facetedProxies)
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
                    IObjectPrx.Parse(ice1 ? "object @ oa1" : "ice:oa1//object", communicator).IcePing();
                    TestHelper.Assert(false);
                }
                catch (NoEndpointException)
                {
                }

                proxies[0].ActivateObjectAdapter("oa", "oa1", "");

                try
                {
                    IObjectPrx.Parse(ice1 ? "object @ oa1" : "ice:oa1//object", communicator).IcePing();
                    TestHelper.Assert(false);
                }
                catch (ObjectNotExistException)
                {
                }

                proxies[0].DeactivateObjectAdapter("oa");

                try
                {
                    IObjectPrx.Parse(ice1 ? "object @ oa1" : "ice:oa1//object", communicator).IcePing();
                    TestHelper.Assert(false);
                }
                catch (NoEndpointException)
                {
                }
            }
            output.WriteLine("ok");

            output.Write("testing object adapter migration... ");
            output.Flush();
            {
                proxies[0].ActivateObjectAdapter("oa", "oa1", "");
                proxies[0].AddObject("oa", "object");
                IObjectPrx.Parse(ice1 ? "object @ oa1" : "ice:oa1//object", communicator).IcePing();
                proxies[0].RemoveObject("oa", "object");
                proxies[0].DeactivateObjectAdapter("oa");

                proxies[1].ActivateObjectAdapter("oa", "oa1", "");
                proxies[1].AddObject("oa", "object");
                IObjectPrx.Parse(ice1 ? "object @ oa1" : "ice:oa1//object", communicator).IcePing();
                proxies[1].RemoveObject("oa", "object");
                proxies[1].DeactivateObjectAdapter("oa");
            }
            output.WriteLine("ok");

            output.Write("testing object migration... ");
            output.Flush();
            {
                proxies[0].ActivateObjectAdapter("oa", "oa1", "");
                proxies[1].ActivateObjectAdapter("oa", "oa2", "");

                proxies[0].AddObject("oa", "object");
                IObjectPrx.Parse(ice1 ? "object @ oa1" : "ice:oa1//object", communicator).IcePing();
                IObjectPrx.Parse(ice1 ? "object" : "ice:object", communicator).IcePing();
                proxies[0].RemoveObject("oa", "object");

                proxies[1].AddObject("oa", "object");
                IObjectPrx.Parse(ice1 ? "object @ oa2" : "ice:oa2//object", communicator).IcePing();

                if (ice1)
                {
                    // TODO: this currently does not work with ice2 because the previous object (in oa1) is still in
                    // the cache and we don't retry on ONE.
                    IObjectPrx.Parse("object", communicator).IcePing();
                }
                proxies[1].RemoveObject("oa", "object");

                try
                {
                    IObjectPrx.Parse(ice1 ? "object @ oa1" : "ice:oa1//object", communicator).IcePing();
                }
                catch (ObjectNotExistException)
                {
                }
                try
                {
                    IObjectPrx.Parse(ice1 ? "object @ oa2" : "ice:oa2//object", communicator).IcePing();
                }
                catch (ObjectNotExistException)
                {
                }

                proxies[0].DeactivateObjectAdapter("oa");
                proxies[1].DeactivateObjectAdapter("oa");
            }
            output.WriteLine("ok");

            output.Write("testing replica groups... ");
            output.Flush();
            {
                proxies[0].ActivateObjectAdapter("oa", "oa1", "rg");
                proxies[1].ActivateObjectAdapter("oa", "oa2", "rg");
                proxies[2].ActivateObjectAdapter("oa", "oa3", "rg");

                proxies[0].AddObject("oa", "object");
                proxies[1].AddObject("oa", "object");
                proxies[2].AddObject("oa", "object");

                IObjectPrx.Parse(ice1 ? "object @ oa1" : "ice:oa1//object", communicator).IcePing();
                IObjectPrx.Parse(ice1 ? "object @ oa2" : "ice:oa2//object", communicator).IcePing();
                IObjectPrx.Parse(ice1 ? "object @ oa3" : "ice:oa3//object", communicator).IcePing();

                IObjectPrx.Parse(ice1 ? "object @ rg" : "ice:rg//object", communicator).IcePing();

                var adapterIds = new List<string>
                {
                    "oa1",
                    "oa2",
                    "oa3"
                };

                ITestIntfPrx intf = ITestIntfPrx.Parse(ice1 ? "object" : "ice:object", communicator).Clone(
                    cacheConnection: false,
                    locatorCacheTimeout: TimeSpan.Zero);
                while (adapterIds.Count > 0)
                {
                    adapterIds.Remove(intf.GetAdapterId());
                }

                // TODO: describe what this test is testing and make sure it does not hang forever!
                while (true)
                {
                    adapterIds.Add("oa1");
                    adapterIds.Add("oa2");
                    adapterIds.Add("oa3");
                    intf = ITestIntfPrx.Parse(ice1 ? "object @ rg" : "ice:rg//object", communicator).Clone(
                        cacheConnection: false,
                        locatorCacheTimeout: TimeSpan.Zero);
                    int nRetry = 100;
                    while (adapterIds.Count > 0 && --nRetry > 0)
                    {
                        adapterIds.Remove(intf.GetAdapterId());
                    }
                    if (nRetry > 0)
                    {
                        break;
                    }

                    adapterIds.Clear();
                }

                proxies[0].DeactivateObjectAdapter("oa");
                proxies[1].DeactivateObjectAdapter("oa");
                TestHelper.Assert(
                    ITestIntfPrx.Parse(
                        ice1 ? "object @ rg" : "ice:rg//object", communicator).GetAdapterId().Equals("oa3"));
                proxies[2].DeactivateObjectAdapter("oa");

                proxies[0].ActivateObjectAdapter("oa", "oa1", "rg");
                proxies[0].AddObject("oa", "object");
                TestHelper.Assert(
                    ITestIntfPrx.Parse(
                        ice1 ? "object @ rg" : "ice:rg//object", communicator).GetAdapterId().Equals("oa1"));
                proxies[0].DeactivateObjectAdapter("oa");
            }
            output.WriteLine("ok");

            output.Write("testing invalid lookup endpoints... ");
            output.Flush();
            {
                string multicast;
                if (helper.Host.Contains(":"))
                {
                    multicast = "\"ff15::1\"";
                }
                else
                {
                    multicast = "239.255.0.1";
                }

                {
                    Dictionary<string, string> properties = communicator.GetProperties();
                    properties["IceDiscovery.Lookup"] = $"udp -h {multicast} --interface unknown";
                    using var comm = new Communicator(properties);
                    TestHelper.Assert(comm.DefaultLocator != null);
                    try
                    {
                        IObjectPrx.Parse(ice1 ? "controller0@control0" : "ice:control0//controller0", comm).IcePing();
                        TestHelper.Assert(false);
                    }
                    catch
                    {
                    }
                }
                {
                    Dictionary<string, string> properties = communicator.GetProperties();
                    string port = $"{helper.BasePort + 10}";
                    string intf = helper.Host.Contains(":") ? $"\"{helper.Host}\"" : helper.Host;
                    properties["IceDiscovery.Lookup"] =
                        $"udp -h {multicast} --interface unknown:udp -h {multicast} -p {port} --interface {intf}";
                    using var comm = new Communicator(properties);
                    TestHelper.Assert(comm.DefaultLocator != null);
                    IObjectPrx.Parse(ice1 ? "controller0@control0" : "ice:control0//controller0", comm).IcePing();
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
