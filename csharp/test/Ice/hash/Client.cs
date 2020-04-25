//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Ice;
using Test;

public class Client : TestHelper
{
    public override void Run(string[] args)
    {
        using Communicator communicator = Initialize(ref args);
        Console.Error.Write("testing proxy & endpoint hash algorithm collisions... ");
        Console.Error.Flush();
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
                var sw = new System.IO.StringWriter();
                sw.Write(i);
                sw.Write(":tcp -p ");
                sw.Write(rand.Next(65536));
                sw.Write(" -t 10");
                sw.Write(rand.Next(1000000));
                sw.Write(":udp -p ");
                sw.Write(rand.Next(65536));
                sw.Write(" -h ");
                sw.Write(rand.Next(100));

                var obj = IObjectPrx.Parse(sw.ToString(), communicator);
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
                    //
                    // Check the same endpoint produce always the same hash
                    //
                    TestHelper.Assert(endpoint.GetHashCode() == endpoint.GetHashCode());
                }
                //
                // Check the same proxy produce always the same hash
                //
                TestHelper.Assert(obj.GetHashCode() == obj.GetHashCode());
            }
            TestHelper.Assert(proxyCollisions < maxCollisions);
            TestHelper.Assert(endpointCollisions < maxCollisions);
            {
                var comparer = new ProxyIdentityComparer();
                proxyCollisions = 0;
                seenProxy = new Dictionary<int, IObjectPrx>();
                for (i = 0; proxyCollisions < maxCollisions && i < maxIterations; ++i)
                {
                    var sw = new System.IO.StringWriter();
                    sw.Write(i);
                    sw.Write(":tcp -p ");
                    sw.Write(rand.Next(65536));
                    sw.Write(" -t 10");
                    sw.Write(rand.Next(1000000));
                    sw.Write(":udp -p ");
                    sw.Write(rand.Next(65536));
                    sw.Write(" -h ");
                    sw.Write(rand.Next(100));

                    var obj = IObjectPrx.Parse(sw.ToString(), communicator);

                    if (seenProxy.ContainsKey(comparer.GetHashCode(obj)))
                    {
                        ++proxyCollisions;
                    }
                    else
                    {
                        seenProxy[comparer.GetHashCode(obj)] = obj;
                    }
                    //
                    // Check the same proxy produce always the same hash
                    //
                    TestHelper.Assert(comparer.GetHashCode(obj) == comparer.GetHashCode(obj));
                }
                TestHelper.Assert(proxyCollisions < maxCollisions);
            }
        }

        {
            var rand = new Random();
            var comparer = new ProxyIdentityFacetComparer();
            proxyCollisions = 0;
            seenProxy = new Dictionary<int, IObjectPrx>();
            for (i = 0; proxyCollisions < maxCollisions && i < maxIterations; ++i)
            {
                var sw = new System.IO.StringWriter();
                sw.Write(i);
                sw.Write(" -f demo:tcp -p ");
                sw.Write(rand.Next(65536));
                sw.Write(" -t 10");
                sw.Write(rand.Next(1000000));
                sw.Write(":udp -p ");
                sw.Write(rand.Next(65536));
                sw.Write(" -h ");
                sw.Write(rand.Next(100));

                var obj = IObjectPrx.Parse(sw.ToString(), communicator);

                if (seenProxy.ContainsKey(comparer.GetHashCode(obj)))
                {
                    ++proxyCollisions;
                }
                else
                {
                    seenProxy[comparer.GetHashCode(obj)] = obj;
                }
                //
                // Check the same proxy produce always the same hash
                //
                TestHelper.Assert(comparer.GetHashCode(obj) == comparer.GetHashCode(obj));
            }
            TestHelper.Assert(proxyCollisions < maxCollisions);
        }

        var iComparer = new ProxyIdentityComparer();
        var ifComparer = new ProxyIdentityFacetComparer();

        var prx1 = IObjectPrx.Parse("Glacier2/router:tcp -p 10010", communicator);
        //Ice.ObjectPrx prx2 = communicator.stringToProxy("Glacier2/router:ssl -p 10011");
        var prx3 = IObjectPrx.Parse("Glacier2/router:udp -p 10012", communicator);
        var prx4 = IObjectPrx.Parse("Glacier2/router:tcp -h zeroc.com -p 10010", communicator);
        //Ice.ObjectPrx prx5 = communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011");
        var prx6 = IObjectPrx.Parse("Glacier2/router:udp -h zeroc.com -p 10012", communicator);
        var prx7 = IObjectPrx.Parse("Glacier2/router:tcp -p 10010 -t 10000", communicator);
        //Ice.ObjectPrx prx8 = communicator.stringToProxy("Glacier2/router:ssl -p 10011 -t 10000");
        var prx9 = IObjectPrx.Parse("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000", communicator);
        //Ice.ObjectPrx prx10 = communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000");

        var proxyMap = new Dictionary<string, int>
        {
            ["prx1"] = prx1.GetHashCode(),
            //proxyMap["prx2"] = prx2.GetHashCode();
            ["prx3"] = prx3.GetHashCode(),
            ["prx4"] = prx4.GetHashCode(),
            //proxyMap["prx5"] = prx5.GetHashCode();
            ["prx6"] = prx6.GetHashCode(),
            ["prx7"] = prx7.GetHashCode(),
            //proxyMap["prx8"] = prx8.GetHashCode();
            ["prx9"] = prx9.GetHashCode()
        };
        //proxyMap["prx10"] = prx10.GetHashCode();

        TestHelper.Assert(IObjectPrx.Parse("Glacier2/router:tcp -p 10010", communicator).GetHashCode() == proxyMap["prx1"]);
        //TestHelper.Assert(communicator.stringToProxy("Glacier2/router:ssl -p 10011").GetHashCode() == proxyMap["prx2"]);
        TestHelper.Assert(IObjectPrx.Parse("Glacier2/router:udp -p 10012", communicator).GetHashCode() == proxyMap["prx3"]);
        TestHelper.Assert(IObjectPrx.Parse("Glacier2/router:tcp -h zeroc.com -p 10010", communicator).GetHashCode() == proxyMap["prx4"]);
        //TestHelper.Assert(communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011").GetHashCode() == proxyMap["prx5"]);
        TestHelper.Assert(IObjectPrx.Parse("Glacier2/router:udp -h zeroc.com -p 10012", communicator).GetHashCode() == proxyMap["prx6"]);
        TestHelper.Assert(IObjectPrx.Parse("Glacier2/router:tcp -p 10010 -t 10000", communicator).GetHashCode() == proxyMap["prx7"]);
        //TestHelper.Assert(communicator.stringToProxy("Glacier2/router:ssl -p 10011 -t 10000").GetHashCode() == proxyMap["prx8"]);
        TestHelper.Assert(IObjectPrx.Parse("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000", communicator).GetHashCode() == proxyMap["prx9"]);
        //TestHelper.Assert(communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000").GetHashCode() == proxyMap["prx10"]);

        TestHelper.Assert(iComparer.GetHashCode(prx1) == iComparer.GetHashCode(prx1));
        TestHelper.Assert(ifComparer.GetHashCode(prx1) == ifComparer.GetHashCode(prx1));

        TestHelper.Assert(iComparer.GetHashCode(prx3) == iComparer.GetHashCode(prx3));
        TestHelper.Assert(ifComparer.GetHashCode(prx3) == ifComparer.GetHashCode(prx3));

        TestHelper.Assert(iComparer.GetHashCode(prx4) == iComparer.GetHashCode(prx4));
        TestHelper.Assert(ifComparer.GetHashCode(prx4) == ifComparer.GetHashCode(prx4));

        TestHelper.Assert(iComparer.GetHashCode(prx6) == iComparer.GetHashCode(prx6));
        TestHelper.Assert(ifComparer.GetHashCode(prx6) == ifComparer.GetHashCode(prx6));

        TestHelper.Assert(iComparer.GetHashCode(prx7) == iComparer.GetHashCode(prx7));
        TestHelper.Assert(ifComparer.GetHashCode(prx7) == ifComparer.GetHashCode(prx7));

        TestHelper.Assert(iComparer.GetHashCode(prx9) == iComparer.GetHashCode(prx9));
        TestHelper.Assert(ifComparer.GetHashCode(prx9) == ifComparer.GetHashCode(prx9));

        Console.Error.WriteLine("ok");

        Console.Error.Write("testing exceptions hash algorithm collisions... ");

        {
            var seenException = new Dictionary<int, Test.OtherException>();
            var rand = new Random();

            int exceptionCollisions = 0;
            for (i = 0; i < maxIterations && exceptionCollisions < maxCollisions; ++i)
            {
                var ex = new Test.OtherException(rand.Next(100), rand.Next(100), 0, false);
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
                //
                // Check the same exception produce always the same hash
                //
                TestHelper.Assert(ex.GetHashCode() == ex.GetHashCode());
            }
            TestHelper.Assert(exceptionCollisions < maxCollisions);
        }

        //
        // Same as above but with numbers in high ranges
        //
        {
            var seenException = new Dictionary<int, Test.OtherException>();
            var rand = new Random();

            int exceptionCollisions = 0;
            for (i = 0; i < maxIterations && exceptionCollisions < maxCollisions; ++i)
            {
                var ex = new Test.OtherException((rand.Next(100) * 2) ^ 30,
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
                //
                // Check the same exception produce always the same hash
                //
                TestHelper.Assert(ex.GetHashCode() == ex.GetHashCode());
            }
            TestHelper.Assert(exceptionCollisions < maxCollisions);
        }

        {
            var seenException = new Dictionary<int, Test.BaseException>();
            var rand = new Random();

            int exceptionCollisions = 0;
            for (i = 0; i < maxIterations && exceptionCollisions < maxCollisions; ++i)
            {
                int v = rand.Next(1000);
                Test.BaseException ex = new Test.InvalidPointException(v);
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

                //
                // Check the same exception produce always the same hash
                //
                TestHelper.Assert(ex.GetHashCode() == ex.GetHashCode());

                ex = new Test.InvalidLengthException(v);
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

                //
                // Check the same exception produce always the same hash
                //
                TestHelper.Assert(ex.GetHashCode() == ex.GetHashCode());
            }
            TestHelper.Assert(exceptionCollisions < maxCollisions);
        }
        Console.Error.WriteLine("ok");

        Console.Error.Write("testing struct hash algorithm collisions... ");
        {
            var seenPointF = new Dictionary<int, Test.PointF>();
            var rand = new Random();
            int structCollisions = 0;
            for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
            {
                var pf = new Test.PointF((float)rand.NextDouble(),
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
                //
                // Check the same struct produce always the same hash
                //
                TestHelper.Assert(pf.GetHashCode() == pf.GetHashCode());
            }
            TestHelper.Assert(structCollisions < maxCollisions);

            var seenPointD = new Dictionary<int, Test.PointD>();
            structCollisions = 0;
            for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
            {
                var pd = new Test.PointD(rand.NextDouble(), rand.NextDouble(), rand.NextDouble());
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
                //
                // Check the same struct produce always the same hash
                //
                TestHelper.Assert(pd.GetHashCode() == pd.GetHashCode());
            }
            TestHelper.Assert(structCollisions < maxCollisions);

            var seenPolyline = new Dictionary<int, Test.Polyline>();
            structCollisions = 0;
            for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
            {
                var polyline = new Test.Polyline();
                var vertices = new List<Test.Point>();
                for (int j = 0; j < 100; ++j)
                {
                    vertices.Add(new Test.Point(rand.Next(100), rand.Next(100)));
                }
                polyline.vertices = vertices.ToArray();

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
                //
                // Check the same struct produce always the same hash
                //
                TestHelper.Assert(polyline.GetHashCode() == polyline.GetHashCode());
            }
            TestHelper.Assert(structCollisions < maxCollisions);

            var seenColorPalette = new Dictionary<int, Test.ColorPalette>();
            structCollisions = 0;
            for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
            {
                var colorPalette = new Test.ColorPalette();
                colorPalette.colors = new Dictionary<int, Test.Color>();
                for (int j = 0; j < 100; ++j)
                {
                    colorPalette.colors[j] = new Test.Color(rand.Next(255),
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
                //
                // Check the same struct produce always the same hash
                //
                TestHelper.Assert(colorPalette.GetHashCode() == colorPalette.GetHashCode());
            }
            TestHelper.Assert(structCollisions < maxCollisions);

            var seenColor = new Dictionary<int, Test.Color>();
            structCollisions = 0;
            for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
            {
                var c = new Test.Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255));
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
                //
                // Check the same struct produce always the same hash
                //
                TestHelper.Assert(c.GetHashCode() == c.GetHashCode());
            }
            TestHelper.Assert(structCollisions < maxCollisions);

            var seenDraw = new Dictionary<int, Test.Draw>();
            structCollisions = 0;
            for (i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
            {
                var draw = new Test.Draw(
                    new Test.Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255)),
                    new Test.Pen(rand.Next(10),
                                    new Test.Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255))),
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
                //
                // Check the same struct produce always the same hash
                //
                TestHelper.Assert(draw.GetHashCode() == draw.GetHashCode());
            }
            TestHelper.Assert(structCollisions < maxCollisions);
        }
        Console.Error.WriteLine("ok");
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
