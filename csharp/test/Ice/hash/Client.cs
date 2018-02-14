// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;
using System.Collections.Generic;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    private static void
    test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Console.Error.Write("testing proxy & endpoint hash algorithm collisions... ");
            Console.Error.Flush();
            Dictionary<int, Ice.ObjectPrx> seenProxy = new Dictionary<int, Ice.ObjectPrx>();
            Dictionary<int, Ice.Endpoint> seenEndpoint = new Dictionary<int, Ice.Endpoint>();
            int proxyCollisions = 0;
            int endpointCollisions = 0;
            int i = 0;
            int maxCollisions = 10;
            int maxIterations = 10000;

            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            //initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL:IceSSL.PluginFactory");
            communicator = Ice.Util.initialize(ref args, initData);
            {
                Random rand = new Random();
                for(i = 0; proxyCollisions < maxCollisions &&
                        endpointCollisions < maxCollisions  &&
                        i < maxIterations; ++i)
                {
                    System.IO.StringWriter sw = new System.IO.StringWriter();
                    sw.Write(i);
                    sw.Write(":tcp -p ");
                    sw.Write(rand.Next(65536));
                    sw.Write(" -t 10");
                    sw.Write(rand.Next(1000000));
                    sw.Write(":udp -p ");
                    sw.Write(rand.Next(65536));
                    sw.Write(" -h ");
                    sw.Write(rand.Next(100));

                    Ice.ObjectPrx obj = communicator.stringToProxy(sw.ToString());
                    List<Ice.Endpoint> endpoints = new List<Ice.Endpoint>(obj.ice_getEndpoints());

                    if(seenProxy.ContainsKey(obj.GetHashCode()))
                    {
                        if(obj.Equals(seenProxy[obj.GetHashCode()]))
                        {
                            continue;
                        }
                        ++proxyCollisions;
                    }
                    else
                    {
                        seenProxy[obj.GetHashCode()] = obj;
                    }

                    foreach(Ice.Endpoint endpoint in endpoints)
                    {
                        if(seenEndpoint.ContainsKey(endpoint.GetHashCode()))
                        {
                            if(endpoint.Equals(seenEndpoint[endpoint.GetHashCode()]))
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
                        test(endpoint.GetHashCode() == endpoint.GetHashCode());
                    }
                    //
                    // Check the same proxy produce always the same hash
                    //
                    test(obj.GetHashCode() == obj.GetHashCode());
                }
                test(proxyCollisions < maxCollisions);
                test(endpointCollisions < maxCollisions);
                {
                    Ice.ProxyIdentityKey comparer = new Ice.ProxyIdentityKey();
                    proxyCollisions = 0;
                    seenProxy = new Dictionary<int, Ice.ObjectPrx>();
                    for(i = 0; proxyCollisions < maxCollisions && i < maxIterations; ++i)
                    {
                        System.IO.StringWriter sw = new System.IO.StringWriter();
                        sw.Write(i);
                        sw.Write(":tcp -p ");
                        sw.Write(rand.Next(65536));
                        sw.Write(" -t 10");
                        sw.Write(rand.Next(1000000));
                        sw.Write(":udp -p ");
                        sw.Write(rand.Next(65536));
                        sw.Write(" -h ");
                        sw.Write(rand.Next(100));

                        Ice.ObjectPrx obj = communicator.stringToProxy(sw.ToString());

                        if(seenProxy.ContainsKey(comparer.GetHashCode(obj)))
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
                        test(comparer.GetHashCode(obj) == comparer.GetHashCode(obj));
                    }
                    test(proxyCollisions < maxCollisions);
                }
            }

            {
                Random rand = new Random();
                Ice.ProxyIdentityFacetKey comparer = new Ice.ProxyIdentityFacetKey();
                proxyCollisions = 0;
                seenProxy = new Dictionary<int, Ice.ObjectPrx>();
                for(i = 0; proxyCollisions < maxCollisions && i < maxIterations; ++i)
                {
                    System.IO.StringWriter sw = new System.IO.StringWriter();
                    sw.Write(i);
                    sw.Write(" -f demo:tcp -p ");
                    sw.Write(rand.Next(65536));
                    sw.Write(" -t 10");
                    sw.Write(rand.Next(1000000));
                    sw.Write(":udp -p ");
                    sw.Write(rand.Next(65536));
                    sw.Write(" -h ");
                    sw.Write(rand.Next(100));

                    Ice.ObjectPrx obj = communicator.stringToProxy(sw.ToString());

                    if(seenProxy.ContainsKey(comparer.GetHashCode(obj)))
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
                    test(comparer.GetHashCode(obj) == comparer.GetHashCode(obj));
                }
                test(proxyCollisions < maxCollisions);
            }

            Ice.ProxyIdentityFacetKey iComparer = new Ice.ProxyIdentityFacetKey();
            Ice.ProxyIdentityFacetKey ifComparer = new Ice.ProxyIdentityFacetKey();

            Ice.ObjectPrx prx1 = communicator.stringToProxy("Glacier2/router:tcp -p 10010");
            //Ice.ObjectPrx prx2 = communicator.stringToProxy("Glacier2/router:ssl -p 10011");
            Ice.ObjectPrx prx3 = communicator.stringToProxy("Glacier2/router:udp -p 10012");
            Ice.ObjectPrx prx4 = communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010");
            //Ice.ObjectPrx prx5 = communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011");
            Ice.ObjectPrx prx6 = communicator.stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012");
            Ice.ObjectPrx prx7 = communicator.stringToProxy("Glacier2/router:tcp -p 10010 -t 10000");
            //Ice.ObjectPrx prx8 = communicator.stringToProxy("Glacier2/router:ssl -p 10011 -t 10000");
            Ice.ObjectPrx prx9 = communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000");
            //Ice.ObjectPrx prx10 = communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000");

            Dictionary<string, int> proxyMap = new Dictionary<string, int>();
            proxyMap["prx1"] = prx1.GetHashCode();
            //proxyMap["prx2"] = prx2.GetHashCode();
            proxyMap["prx3"] = prx3.GetHashCode();
            proxyMap["prx4"] = prx4.GetHashCode();
            //proxyMap["prx5"] = prx5.GetHashCode();
            proxyMap["prx6"] = prx6.GetHashCode();
            proxyMap["prx7"] = prx7.GetHashCode();
            //proxyMap["prx8"] = prx8.GetHashCode();
            proxyMap["prx9"] = prx9.GetHashCode();
            //proxyMap["prx10"] = prx10.GetHashCode();

            test(communicator.stringToProxy("Glacier2/router:tcp -p 10010").GetHashCode() == proxyMap["prx1"]);
            //test(communicator.stringToProxy("Glacier2/router:ssl -p 10011").GetHashCode() == proxyMap["prx2"]);
            test(communicator.stringToProxy("Glacier2/router:udp -p 10012").GetHashCode() == proxyMap["prx3"]);
            test(communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010").GetHashCode() == proxyMap["prx4"]);
            //test(communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011").GetHashCode() == proxyMap["prx5"]);
            test(communicator.stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012").GetHashCode() == proxyMap["prx6"]);
            test(communicator.stringToProxy("Glacier2/router:tcp -p 10010 -t 10000").GetHashCode() == proxyMap["prx7"]);
            //test(communicator.stringToProxy("Glacier2/router:ssl -p 10011 -t 10000").GetHashCode() == proxyMap["prx8"]);
            test(communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000").GetHashCode() == proxyMap["prx9"]);
            //test(communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000").GetHashCode() == proxyMap["prx10"]);

            test(iComparer.GetHashCode(prx1) == iComparer.GetHashCode(prx1));
            test(ifComparer.GetHashCode(prx1) == ifComparer.GetHashCode(prx1));

            test(iComparer.GetHashCode(prx3) == iComparer.GetHashCode(prx3));
            test(ifComparer.GetHashCode(prx3) == ifComparer.GetHashCode(prx3));

            test(iComparer.GetHashCode(prx4) == iComparer.GetHashCode(prx4));
            test(ifComparer.GetHashCode(prx4) == ifComparer.GetHashCode(prx4));

            test(iComparer.GetHashCode(prx6) == iComparer.GetHashCode(prx6));
            test(ifComparer.GetHashCode(prx6) == ifComparer.GetHashCode(prx6));

            test(iComparer.GetHashCode(prx7) == iComparer.GetHashCode(prx7));
            test(ifComparer.GetHashCode(prx7) == ifComparer.GetHashCode(prx7));

            test(iComparer.GetHashCode(prx9) == iComparer.GetHashCode(prx9));
            test(ifComparer.GetHashCode(prx9) == ifComparer.GetHashCode(prx9));

            Console.Error.WriteLine("ok");

            Console.Error.Write("testing exceptions hash algorithm collisions... ");

            {
                Dictionary<int,Test.OtherException> seenException = new Dictionary<int, Test.OtherException>();
                Random rand = new Random();

                int exceptionCollisions = 0;
                for(i = 0; i < maxIterations &&
                        exceptionCollisions < maxCollisions; ++i)
                {
                    Test.OtherException ex = new Test.OtherException(rand.Next(100), rand.Next(100), 0, false);
                    if(seenException.ContainsKey(ex.GetHashCode()))
                    {
                        if(ex.Equals(seenException[ex.GetHashCode()]))
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
                    test(ex.GetHashCode() == ex.GetHashCode());
                }
                test(exceptionCollisions < maxCollisions);
            }

            //
            // Same as above but with numbers in high ranges
            //
            {
                Dictionary<int,Test.OtherException> seenException = new Dictionary<int, Test.OtherException>();
                Random rand = new Random();

                int exceptionCollisions = 0;
                for(i = 0; i < maxIterations &&
                        exceptionCollisions < maxCollisions; ++i)
                {
                    Test.OtherException ex = new Test.OtherException(rand.Next(100) * 2^30, rand.Next(100) * 2^30, rand.Next(100) * 2^30, false);
                    if(seenException.ContainsKey(ex.GetHashCode()))
                    {
                        if(ex.Equals(seenException[ex.GetHashCode()]))
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
                    test(ex.GetHashCode() == ex.GetHashCode());
                }
                test(exceptionCollisions < maxCollisions);
            }

            {
                Dictionary<int,Test.BaseException> seenException = new Dictionary<int, Test.BaseException>();
                Random rand = new Random();

                int exceptionCollisions = 0;
                for(i = 0; i < maxIterations &&
                        exceptionCollisions < maxCollisions; ++i)
                {
                    int v = rand.Next(1000);
                    Test.BaseException ex = new Test.InvalidPointException(v);
                    if(seenException.ContainsKey(ex.GetHashCode()))
                    {
                        if(ex.Equals(seenException[ex.GetHashCode()]))
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
                    test(ex.GetHashCode() == ex.GetHashCode());

                    ex = new Test.InvalidLengthException(v);
                    if(seenException.ContainsKey(ex.GetHashCode()))
                    {
                        if(ex.Equals(seenException[ex.GetHashCode()]))
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
                    test(ex.GetHashCode() == ex.GetHashCode());
                }
                test(exceptionCollisions < maxCollisions);
            }
            Console.Error.WriteLine("ok");

            Console.Error.Write("testing struct hash algorithm collisions... ");
            {
                Dictionary<int,Test.PointF> seenPointF = new Dictionary<int, Test.PointF>();
                Random rand = new Random();
                int structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Test.PointF pf = new Test.PointF((float)rand.NextDouble(), (float)rand.NextDouble(),
                                                     (float)rand.NextDouble());
                    if(seenPointF.ContainsKey(pf.GetHashCode()))
                    {
                        if(pf.Equals(seenPointF[pf.GetHashCode()]))
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
                    test(pf.GetHashCode() == pf.GetHashCode());
                }
                test(structCollisions < maxCollisions);

                Dictionary<int,Test.PointD> seenPointD = new Dictionary<int, Test.PointD>();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Test.PointD pd = new Test.PointD(rand.NextDouble(), rand.NextDouble(),
                                                     rand.NextDouble());
                    if(seenPointD.ContainsKey(pd.GetHashCode()))
                    {
                        if(pd.Equals(seenPointD[pd.GetHashCode()]))
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
                    test(pd.GetHashCode() == pd.GetHashCode());
                }
                test(structCollisions < maxCollisions);

                Dictionary<int,Test.Polyline> seenPolyline = new Dictionary<int, Test.Polyline>();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Test.Polyline polyline = new Test.Polyline();
                    List<Test.Point> vertices = new List<Test.Point>();
                    for(int j = 0; j < 100; ++j)
                    {
                        vertices.Add(new Test.Point(rand.Next(100), rand.Next(100)));
                    }
                    polyline.vertices = vertices.ToArray();

                    if(seenPolyline.ContainsKey(polyline.GetHashCode()))
                    {
                        if(polyline.Equals(seenPolyline[polyline.GetHashCode()]))
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
                    test(polyline.GetHashCode() == polyline.GetHashCode());
                }
                test(structCollisions < maxCollisions);

                Dictionary<int,Test.ColorPalette> seenColorPalette = new Dictionary<int, Test.ColorPalette>();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Test.ColorPalette colorPalette = new Test.ColorPalette();
                    colorPalette.colors = new Dictionary<int, Test.Color>();
                    for(int j = 0; j < 100; ++j)
                    {
                        colorPalette.colors[j] = new Test.Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255));
                    }

                    if(seenColorPalette.ContainsKey(colorPalette.GetHashCode()))
                    {
                        if(colorPalette.Equals(seenColorPalette[colorPalette.GetHashCode()]))
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
                    test(colorPalette.GetHashCode() == colorPalette.GetHashCode());
                }
                test(structCollisions < maxCollisions);

                Dictionary<int,Test.Color> seenColor = new Dictionary<int, Test.Color>();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Test.Color c = new Test.Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255));
                    if(seenColor.ContainsKey(c.GetHashCode()))
                    {
                        if(c.Equals(seenColor[c.GetHashCode()]))
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
                    test(c.GetHashCode() == c.GetHashCode());
                }
                test(structCollisions < maxCollisions);

                structCollisions = 0;
                Dictionary<int,Test.Draw> seenDraw = new Dictionary<int, Test.Draw>();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Test.Draw draw =  new Test.Draw(
                        new Test.Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255)),
                        new Test.Pen(rand.Next(10),
                                     new Test.Color(rand.Next(255), rand.Next(255), rand.Next(255), rand.Next(255))),
                        false);

                    if(seenDraw.ContainsKey(draw.GetHashCode()))
                    {
                        if(draw.Equals(seenDraw[draw.GetHashCode()]))
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
                    test(draw.GetHashCode() == draw.GetHashCode());
                }
                test(structCollisions < maxCollisions);
            }
            Console.Error.WriteLine("ok");

            if(communicator != null)
            {
                communicator.destroy();
            }
        }
        catch(Exception ex)
        {
            Console.WriteLine(ex.ToString());
            status = 1;
        }

        return status;
    }
}
