// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Discovery
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper, int num)
        {
            TextWriter output = helper.Output;
            Communicator communicator = helper.Communicator;

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
                IObjectPrx.Parse(ice1 ? "object" : "ice:object", communicator).IcePing();
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

                // Check that the well known object is reachable with all replica group members
                ITestIntfPrx intf = ITestIntfPrx.Parse(ice1 ? "object" : "ice:object", communicator).Clone(
                    cacheConnection: false,
                    preferExistingConnection: false,
                    locatorCacheTimeout: TimeSpan.Zero);
                while (adapterIds.Count > 0)
                {
                    string id = intf.GetAdapterId();
                    adapterIds.Remove(id);
                    switch (id)
                    {
                        case "oa1":
                        {
                            proxies[0].DeactivateObjectAdapter("oa");
                            break;
                        }
                        case "oa2":
                        {
                            proxies[1].DeactivateObjectAdapter("oa");
                            break;
                        }
                        case "oa3":
                        {
                            proxies[2].DeactivateObjectAdapter("oa");
                            break;
                        }
                    }
                }

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

                adapterIds = new List<string>
                {
                    "oa1",
                    "oa2",
                    "oa3"
                };

                // Check that the indirect reference is reachable with all replica group members
                intf = ITestIntfPrx.Parse(ice1 ? "object @ rg" : "ice:rg//object", communicator).Clone(
                    cacheConnection: false,
                    preferExistingConnection: false,
                    locatorCacheTimeout: TimeSpan.Zero);
                while (adapterIds.Count > 0)
                {
                    var id = intf.GetAdapterId();
                    adapterIds.Remove(id);
                    switch (id)
                    {
                        case "oa1":
                        {
                            proxies[0].DeactivateObjectAdapter("oa");
                            break;
                        }
                        case "oa2":
                        {
                            proxies[1].DeactivateObjectAdapter("oa");
                            break;
                        }
                        case "oa3":
                        {
                            proxies[2].DeactivateObjectAdapter("oa");
                            break;
                        }
                    }
                }
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
                    properties["Ice.Discovery.Lookup"] = $"udp -h {multicast} --interface unknown";
                    await using var comm = new Communicator(properties);
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
                    string lookup = $"udp -h {multicast} --interface unknown:udp -h {multicast} -p {port}";
                    if (!OperatingSystem.IsLinux())
                    {
                        lookup += " --interface {intf}";
                    }

                    await using var comm = new Communicator(properties);
                    TestHelper.Assert(comm.DefaultLocator != null);
                    IObjectPrx.Parse(ice1 ? "controller0@control0" : "ice:control0//controller0", comm).IcePing();
                }
            }
            output.WriteLine("ok");

            output.Write("shutting down... ");
            output.Flush();
            foreach (IControllerPrx prx in proxies)
            {
                await prx.ShutdownAsync();
            }
            output.WriteLine("ok");
        }
    }
}
