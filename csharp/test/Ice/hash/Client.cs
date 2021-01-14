// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Hash
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            Output.Write("testing proxy & endpoint hash algorithm collisions... ");
            Output.Flush();
            var seenProxy = new Dictionary<int, IObjectPrx>();
            var seenEndpoint = new Dictionary<int, Endpoint>();
            int proxyCollisions = 0;
            int endpointCollisions = 0;
            int i = 0;
            int maxCollisions = 10;
            int maxIterations = 10000;

            {
                var rand = new Random();
                for (i = 0; proxyCollisions < maxCollisions &&
                     endpointCollisions < maxCollisions &&
                     i < maxIterations; ++i)
                {
                    string proxyString = $"ice+tcp://host-{rand.Next(100)}:{rand.Next(65536)}/{i}";

                    var obj = IObjectPrx.Parse(proxyString, Communicator);
                    var endpoints = new List<Endpoint>(obj.Endpoints);

                    if (seenProxy.ContainsKey(obj.GetHashCode()))
                    {
                        if (obj.Equals(seenProxy[obj.GetHashCode()]))
                        {
                            continue;
                        }
                        ++proxyCollisions;
                    }
                    else
                    {
                        seenProxy[obj.GetHashCode()] = obj;
                    }

                    foreach (Endpoint endpoint in endpoints)
                    {
                        if (seenEndpoint.ContainsKey(endpoint.GetHashCode()))
                        {
                            if (endpoint.Equals(seenEndpoint[endpoint.GetHashCode()]))
                            {
                                continue;
                            }
                            ++endpointCollisions;
                        }
                        else
                        {
                            seenEndpoint[endpoint.GetHashCode()] = endpoint;
                        }
                        // Check the same endpoint produce always the same hash
                        Assert(endpoint.GetHashCode() == endpoint.GetHashCode());
                    }
                    // Check the same proxy produce always the same hash
                    Assert(obj.GetHashCode() == obj.GetHashCode());
                }
                Assert(proxyCollisions < maxCollisions);
                Assert(endpointCollisions < maxCollisions);
                {
                    proxyCollisions = 0;
                    seenProxy = new Dictionary<int, IObjectPrx>();
                    for (i = 0; proxyCollisions < maxCollisions && i < maxIterations; ++i)
                    {
                        string proxyString = $"ice+tcp://host-{rand.Next(100)}:{rand.Next(65536)}/{i}";

                        var obj = IObjectPrx.Parse(proxyString, Communicator);

                        if (seenProxy.ContainsKey(ProxyComparer.Identity.GetHashCode(obj)))
                        {
                            ++proxyCollisions;
                        }
                        else
                        {
                            seenProxy[ProxyComparer.Identity.GetHashCode(obj)] = obj;
                        }
                        // Check the same proxy produce always the same hash
                        Assert(ProxyComparer.Identity.GetHashCode(obj) == ProxyComparer.Identity.GetHashCode(obj));
                    }
                    Assert(proxyCollisions < maxCollisions);
                }
            }

            {
                var rand = new Random();
                proxyCollisions = 0;
                seenProxy = new Dictionary<int, IObjectPrx>();
                for (i = 0; proxyCollisions < maxCollisions && i < maxIterations; ++i)
                {
                    string proxyString = $"ice+tcp://host-{rand.Next(100)}:{rand.Next(65536)}/{i}#facet";

                    var obj = IObjectPrx.Parse(proxyString, Communicator);

                    if (seenProxy.ContainsKey(ProxyComparer.IdentityAndFacet.GetHashCode(obj)))
                    {
                        ++proxyCollisions;
                    }
                    else
                    {
                        seenProxy[ProxyComparer.IdentityAndFacet.GetHashCode(obj)] = obj;
                    }
                    // Check the same proxy produce always the same hash
                    Assert(ProxyComparer.IdentityAndFacet.GetHashCode(obj) ==
                           ProxyComparer.IdentityAndFacet.GetHashCode(obj));
                }
                Assert(proxyCollisions < maxCollisions);
            }

            var prx1 = IObjectPrx.Parse("Glacier2/router:tcp -h localhost -p 10010", Communicator);
            var prx2 = IObjectPrx.Parse("Glacier2/router:udp -h localhost -p 10012", Communicator);
            var prx3 = IObjectPrx.Parse("Glacier2/router:tcp -h zeroc.com -p 10010", Communicator);
            var prx4 = IObjectPrx.Parse("Glacier2/router:udp -h zeroc.com -p 10012", Communicator);
            var prx5 = IObjectPrx.Parse("Glacier2/router:tcp -h localhost -p 10010 -t 10000", Communicator);
            var prx6 = IObjectPrx.Parse("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000", Communicator);

            var proxyMap = new Dictionary<string, int>
            {
                ["prx1"] = prx1.GetHashCode(),
                ["prx2"] = prx2.GetHashCode(),
                ["prx3"] = prx3.GetHashCode(),
                ["prx4"] = prx4.GetHashCode(),
                ["prx5"] = prx5.GetHashCode(),
                ["prx6"] = prx6.GetHashCode()
            };

            Assert(IObjectPrx.Parse("Glacier2/router:tcp -h localhost -p 10010", Communicator).GetHashCode() == proxyMap["prx1"]);
            Assert(IObjectPrx.Parse("Glacier2/router:udp -h localhost -p 10012", Communicator).GetHashCode() == proxyMap["prx2"]);
            Assert(IObjectPrx.Parse("Glacier2/router:tcp -h zeroc.com -p 10010", Communicator).GetHashCode() ==
                   proxyMap["prx3"]);
            Assert(IObjectPrx.Parse("Glacier2/router:udp -h zeroc.com -p 10012", Communicator).GetHashCode() ==
                   proxyMap["prx4"]);
            Assert(IObjectPrx.Parse("Glacier2/router:tcp -h localhost -p 10010 -t 10000", Communicator).GetHashCode() ==
                   proxyMap["prx5"]);
            Assert(
                IObjectPrx.Parse("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000", Communicator).GetHashCode() ==
                proxyMap["prx6"]);

            Assert(ProxyComparer.Identity.GetHashCode(prx1) == ProxyComparer.Identity.GetHashCode(prx1));
            Assert(ProxyComparer.IdentityAndFacet.GetHashCode(prx1) ==
                   ProxyComparer.IdentityAndFacet.GetHashCode(prx1));

            Assert(ProxyComparer.Identity.GetHashCode(prx2) == ProxyComparer.Identity.GetHashCode(prx2));
            Assert(ProxyComparer.IdentityAndFacet.GetHashCode(prx2) ==
                   ProxyComparer.IdentityAndFacet.GetHashCode(prx2));

            Assert(ProxyComparer.Identity.GetHashCode(prx3) == ProxyComparer.Identity.GetHashCode(prx3));
            Assert(ProxyComparer.IdentityAndFacet.GetHashCode(prx3) ==
                   ProxyComparer.IdentityAndFacet.GetHashCode(prx3));

            Assert(ProxyComparer.Identity.GetHashCode(prx4) == ProxyComparer.Identity.GetHashCode(prx4));
            Assert(ProxyComparer.IdentityAndFacet.GetHashCode(prx4) ==
                   ProxyComparer.IdentityAndFacet.GetHashCode(prx4));

            Assert(ProxyComparer.Identity.GetHashCode(prx5) == ProxyComparer.Identity.GetHashCode(prx5));
            Assert(ProxyComparer.IdentityAndFacet.GetHashCode(prx5) ==
                   ProxyComparer.IdentityAndFacet.GetHashCode(prx5));

            Assert(ProxyComparer.Identity.GetHashCode(prx6) == ProxyComparer.Identity.GetHashCode(prx6));
            Assert(ProxyComparer.IdentityAndFacet.GetHashCode(prx6) ==
                   ProxyComparer.IdentityAndFacet.GetHashCode(prx6));

            Output.WriteLine("ok");

            Output.Write("testing exceptions hash algorithm collisions... ");

            {
                var seenException = new Dictionary<int, OtherException>();
                var rand = new Random();

                int exceptionCollisions = 0;
                for (i = 0; i < maxIterations && exceptionCollisions < maxCollisions; ++i)
                {
                    var ex = new OtherException(rand.Next(100), rand.Next(100), 0, false);
                    if (seenException.ContainsKey(ex.GetHashCode()))
                    {
                        if (ex.Equals(seenException[ex.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        exceptionCollisions++;
                    }
                    else
                    {
                        seenException[ex.GetHashCode()] = ex;
                    }
                    // Check the same exception produce always the same hash
                    Assert(ex.GetHashCode() == ex.GetHashCode());
                }
                Assert(exceptionCollisions < maxCollisions);
            }

            // Same as above but with numbers in high ranges
            {
                var seenException = new Dictionary<int, OtherException>();
                var rand = new Random();

                int exceptionCollisions = 0;
                for (i = 0; i < maxIterations && exceptionCollisions < maxCollisions; ++i)
                {
                    var ex = new OtherException((rand.Next(100) * 2) ^ 30,
                                                (rand.Next(100) * 2) ^ 30,
                                                (rand.Next(100) * 2) ^ 30,
                                                false);

                    if (seenException.ContainsKey(ex.GetHashCode()))
                    {
                        if (ex.Equals(seenException[ex.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        exceptionCollisions++;
                    }
                    else
                    {
                        seenException[ex.GetHashCode()] = ex;
                    }
                    // Check the same exception produce always the same hash
                    Assert(ex.GetHashCode() == ex.GetHashCode());
                }
                Assert(exceptionCollisions < maxCollisions);
            }

            {
                var seenException = new Dictionary<int, BaseException>();
                var rand = new Random();

                int exceptionCollisions = 0;
                for (i = 0; i < maxIterations && exceptionCollisions < maxCollisions; ++i)
                {
                    int v = rand.Next(1000);
                    BaseException ex = new InvalidPointException(v);
                    if (seenException.ContainsKey(ex.GetHashCode()))
                    {
                        if (ex.Equals(seenException[ex.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        exceptionCollisions++;
                    }
                    else
                    {
                        seenException[ex.GetHashCode()] = ex;
                    }

                    // Check the same exception produce always the same hash
                    Assert(ex.GetHashCode() == ex.GetHashCode());

                    ex = new InvalidLengthException(v);
                    if (seenException.ContainsKey(ex.GetHashCode()))
                    {
                        if (ex.Equals(seenException[ex.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        exceptionCollisions++;
                    }
                    else
                    {
                        seenException[ex.GetHashCode()] = ex;
                    }

                    // Check the same exception produce always the same hash
                    Assert(ex.GetHashCode() == ex.GetHashCode());
                }
                Assert(exceptionCollisions < maxCollisions);
            }
            Output.WriteLine("ok");

            Output.Write("testing struct hash algorithm collisions... ");
            {
                var seenPointF = new Dictionary<int, PointF>();
                var rand = new Random();
                int structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    var pf = new PointF((float)rand.NextDouble(),
                                            (float)rand.NextDouble(),
                                            (float)rand.NextDouble());
                    if (seenPointF.ContainsKey(pf.GetHashCode()))
                    {
                        if (pf.Equals(seenPointF[pf.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenPointF[pf.GetHashCode()] = pf;
                    }
                    // Check the same struct produce always the same hash
                    Assert(pf.GetHashCode() == pf.GetHashCode());
                }
                Assert(structCollisions < maxCollisions);

                var seenPointD = new Dictionary<int, PointD>();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    var pd = new PointD(rand.NextDouble(), rand.NextDouble(), rand.NextDouble());
                    if (seenPointD.ContainsKey(pd.GetHashCode()))
                    {
                        if (pd.Equals(seenPointD[pd.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenPointD[pd.GetHashCode()] = pd;
                    }
                    // Check the same struct produce always the same hash
                    Assert(pd.GetHashCode() == pd.GetHashCode());
                }
                Assert(structCollisions < maxCollisions);

                var seenPolyline = new Dictionary<int, Polyline>();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    var polyline = new Polyline();
                    var vertices = new List<Point>();
                    for (int j = 0; j < 100; ++j)
                    {
                        vertices.Add(new Point(rand.Next(100), rand.Next(100)));
                    }
                    polyline.Vertices = vertices.ToArray();

                    if (seenPolyline.ContainsKey(polyline.GetHashCode()))
                    {
                        if (polyline.Equals(seenPolyline[polyline.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenPolyline[polyline.GetHashCode()] = polyline;
                    }
                    // Check the same struct produce always the same hash
                    Assert(polyline.GetHashCode() == polyline.GetHashCode());
                }
                Assert(structCollisions < maxCollisions);

                var seenColorPalette = new Dictionary<int, ColorPalette>();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    var colorPalette = new ColorPalette();
                    colorPalette.Colors = new Dictionary<int, Color>();
                    for (int j = 0; j < 100; ++j)
                    {
                        colorPalette.Colors[j] = new Color(
                            rand.Next(255),
                            rand.Next(255),
                            rand.Next(255),
                            rand.Next(255));
                    }

                    if (seenColorPalette.ContainsKey(colorPalette.GetHashCode()))
                    {
                        if (colorPalette.Equals(seenColorPalette[colorPalette.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenColorPalette[colorPalette.GetHashCode()] = colorPalette;
                    }

                    // Check the same struct produce always the same hash
                    Assert(colorPalette.GetHashCode() == colorPalette.GetHashCode());
                }
                Assert(structCollisions < maxCollisions);

                var seenColor = new Dictionary<int, Color>();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    var c = new Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255));
                    if (seenColor.ContainsKey(c.GetHashCode()))
                    {
                        if (c.Equals(seenColor[c.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenColor[c.GetHashCode()] = c;
                    }
                    // Check the same struct produce always the same hash
                    Assert(c.GetHashCode() == c.GetHashCode());
                }
                Assert(structCollisions < maxCollisions);

                var seenDraw = new Dictionary<int, Draw>();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    var draw = new Draw(
                        new Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255)),
                        new Pen(rand.Next(10),
                            new Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255))),
                        false);

                    if (seenDraw.ContainsKey(draw.GetHashCode()))
                    {
                        if (draw.Equals(seenDraw[draw.GetHashCode()]))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenDraw[draw.GetHashCode()] = draw;
                    }
                    // Check the same struct produce always the same hash
                    Assert(draw.GetHashCode() == draw.GetHashCode());
                }
                Assert(structCollisions < maxCollisions);
            }
            Output.WriteLine("ok");
            return Task.CompletedTask;
        }

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
