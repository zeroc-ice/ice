// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.hash;

import java.io.PrintWriter;

import test.Ice.hash.Test.*;

public class Client extends test.Util.Application
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }

    @Override
    public int run(String[] args)
    {
        PrintWriter out = getWriter();
        int status = 0;
        try
        {
            java.util.Map<Integer, Ice.ObjectPrx> seenProxy = new java.util.HashMap<Integer, Ice.ObjectPrx>();
            java.util.Map<Integer, Ice.Endpoint> seenEndpoint = new java.util.HashMap<Integer, Ice.Endpoint>();
            int proxyCollisions = 0;
            int endpointCollisions = 0;
            int i = 0;
            int maxCollisions = 10;
            int maxIterations = 10000;

            Ice.InitializationData initData = createInitializationData() ;
            initData.properties = Ice.Util.createProperties(args);
            initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
            Ice.Communicator  communicator = Ice.Util.initialize(args, initData);

            out.print("testing proxy & endpoint hash algorithm collisions... ");
            out.flush();
            {
                java.util.Random rand = new java.util.Random();
                for(i = 0; proxyCollisions < maxCollisions && 
                        endpointCollisions < maxCollisions  && 
                        i < maxIterations; ++i)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    sw.write(Integer.toString(i));
                    sw.write(":tcp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -t 10");
                    sw.write(Integer.toString(rand.nextInt(1000000)));
                    sw.write(":udp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -h ");
                    sw.write(Integer.toString(rand.nextInt(100)));

                    Ice.ObjectPrx obj = communicator.stringToProxy(sw.toString());
                    java.util.List<Ice.Endpoint> endpoints = new java.util.ArrayList<Ice.Endpoint>(java.util.Arrays.asList(obj.ice_getEndpoints()));
                    if(seenProxy.containsKey(obj.hashCode()))
                    {
                        if(obj.equals(seenProxy.get(obj.hashCode())))
                        {
                            continue; // Same object
                        }
                        ++proxyCollisions;
                    }
                    else
                    {
                        seenProxy.put(obj.hashCode(), obj);
                    }
                    
                    java.util.Iterator<Ice.Endpoint> j = endpoints.iterator();
                    while(j.hasNext())
                    {
                        Ice.Endpoint endpoint = j.next();
                        if(seenEndpoint.containsKey(endpoint.hashCode()))
                        {
                            if(endpoint.equals(seenEndpoint.get(endpoint.hashCode())))
                            {
                                continue; // Same endpoint
                            }
                            ++endpointCollisions;
                        }
                        else
                        {
                            seenEndpoint.put(endpoint.hashCode(), endpoint);
                        }
                        //
                        // Check the same endpoint produce always the same hash
                        //
                        test(endpoint.hashCode() == endpoint.hashCode());
                    }
                    //
                    // Check the same proxy produce always the same hash
                    //
                    test(obj.hashCode() == obj.hashCode());
                }
                test(proxyCollisions < maxCollisions);
                test(endpointCollisions < maxCollisions);

                proxyCollisions = 0;
                seenProxy = new java.util.HashMap<Integer, Ice.ObjectPrx>();
                for(i = 0; proxyCollisions < maxCollisions && 
                        endpointCollisions < maxCollisions  && 
                        i < maxIterations; ++i)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    sw.write(Integer.toString(i));
                    sw.write(":tcp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -t 10");
                    sw.write(Integer.toString(rand.nextInt(1000000)));
                    sw.write(":udp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -h ");
                    sw.write(Integer.toString(rand.nextInt(100)));

                    Ice.ProxyIdentityKey obj = new Ice.ProxyIdentityKey(communicator.stringToProxy(sw.toString()));
                    if(seenProxy.containsKey(obj.hashCode()))
                    {
                        ++proxyCollisions;
                    }
                    else
                    {
                        seenProxy.put(obj.hashCode(), obj.getProxy());
                    }
                    //
                    // Check the same proxy produce always the same hash
                    //
                    test(obj.hashCode() == obj.hashCode());
                }
                test(proxyCollisions < maxCollisions);

                proxyCollisions = 0;
                seenProxy = new java.util.HashMap<Integer, Ice.ObjectPrx>();
                for(i = 0; proxyCollisions < maxCollisions && 
                        endpointCollisions < maxCollisions  && 
                        i < maxIterations; ++i)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    sw.write(Integer.toString(i));
                    sw.write(":tcp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -t 10");
                    sw.write(Integer.toString(rand.nextInt(1000000)));
                    sw.write(":udp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -h ");
                    sw.write(Integer.toString(rand.nextInt(100)));

                    Ice.ProxyIdentityFacetKey obj = new Ice.ProxyIdentityFacetKey(communicator.stringToProxy(sw.toString()));
                    if(seenProxy.containsKey(obj.hashCode()))
                    {
                        ++proxyCollisions;
                    }
                    else
                    {
                        seenProxy.put(obj.hashCode(), obj.getProxy());
                    }
                    //
                    // Check the same proxy produce always the same hash
                    //
                    test(obj.hashCode() == obj.hashCode());
                }
                test(proxyCollisions < maxCollisions);

                Ice.ObjectPrx prx1 = communicator.stringToProxy("Glacier2/router:tcp -p 10010");
                Ice.ObjectPrx prx2 = communicator.stringToProxy("Glacier2/router:ssl -p 10011");
                Ice.ObjectPrx prx3 = communicator.stringToProxy("Glacier2/router:udp -p 10012");
                Ice.ObjectPrx prx4 = communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010");
                Ice.ObjectPrx prx5 = communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011");
                Ice.ObjectPrx prx6 = communicator.stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012");
                Ice.ObjectPrx prx7 = communicator.stringToProxy("Glacier2/router:tcp -p 10010 -t 10000");
                Ice.ObjectPrx prx8 = communicator.stringToProxy("Glacier2/router:ssl -p 10011 -t 10000");
                Ice.ObjectPrx prx9 = communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000");
                Ice.ObjectPrx prx10 = communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000");

                java.util.Map<String, Integer> proxyMap = new java.util.HashMap<String, Integer>();
                proxyMap.put("prx1", prx1.hashCode());
                proxyMap.put("prx2", prx2.hashCode());
                proxyMap.put("prx3", prx3.hashCode());
                proxyMap.put("prx4", prx4.hashCode());
                proxyMap.put("prx5", prx5.hashCode());
                proxyMap.put("prx6", prx6.hashCode());
                proxyMap.put("prx7", prx7.hashCode());
                proxyMap.put("prx8", prx8.hashCode());
                proxyMap.put("prx9", prx9.hashCode());
                proxyMap.put("prx10", prx10.hashCode());

                test(communicator.stringToProxy("Glacier2/router:tcp -p 10010").hashCode() == proxyMap.get("prx1"));
                test(communicator.stringToProxy("Glacier2/router:ssl -p 10011").hashCode() == proxyMap.get("prx2"));
                test(communicator.stringToProxy("Glacier2/router:udp -p 10012").hashCode() == proxyMap.get("prx3"));
                test(communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010").hashCode() == proxyMap.get("prx4"));
                test(communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011").hashCode() == proxyMap.get("prx5"));
                test(communicator.stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012").hashCode() == proxyMap.get("prx6"));
                test(communicator.stringToProxy("Glacier2/router:tcp -p 10010 -t 10000").hashCode() == proxyMap.get("prx7"));
                test(communicator.stringToProxy("Glacier2/router:ssl -p 10011 -t 10000").hashCode() == proxyMap.get("prx8"));
                test(communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000").hashCode() == proxyMap.get("prx9"));
                test(communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000").hashCode() == proxyMap.get("prx10"));

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx1).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx1).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx2).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx2).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx3).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx3).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx4).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx4).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx5).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx5).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx6).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx6).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx7).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx7).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx8).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx8).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx9).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx9).hashCode());

                test(new Ice.ProxyIdentityKey(prx1).hashCode() == new Ice.ProxyIdentityKey(prx10).hashCode());
                test(new Ice.ProxyIdentityFacetKey(prx1).hashCode() == new Ice.ProxyIdentityFacetKey(prx10).hashCode());
            }

            out.println("ok");

            out.print("testing struct hash algorithm collisions... ");
            out.flush();
            {
                java.util.Map<Integer, PointF> seenPointF = new java.util.HashMap<Integer, PointF>();
                java.util.Random rand = new java.util.Random();
                int structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    PointF pf = new PointF(rand.nextFloat(), rand.nextFloat(), rand.nextFloat());
                    if(seenPointF.containsKey(pf.hashCode()))
                    {
                        if(pf.equals(seenPointF.get(pf.hashCode())))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenPointF.put(pf.hashCode(), pf);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(pf.hashCode() == pf.hashCode());
                }
                test(structCollisions < maxCollisions);

                java.util.Map<Integer, PointD> seenPointD = new java.util.HashMap<Integer, PointD>();
                rand = new java.util.Random();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    PointD pd = new PointD(rand.nextDouble(), rand.nextDouble(), rand.nextDouble());
                    if(seenPointD.containsKey(pd.hashCode()))
                    {
                        if(pd.equals(seenPointF.get(pd.hashCode())))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenPointD.put(pd.hashCode(), pd);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(pd.hashCode() == pd.hashCode());
                }
                test(structCollisions < maxCollisions);

                java.util.Map<Integer, Polyline> seenPolyline = new java.util.HashMap<Integer, Polyline>();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Polyline polyline = new Polyline();
                    java.util.List<Point> vertices = new java.util.ArrayList<Point>();
                    for(int j = 0; j < 100; ++j)
                    {
                        vertices.add(new Point(rand.nextInt(100), rand.nextInt(100)));
                    }
                    polyline.vertices = new Point[vertices.size()];
                    vertices.toArray(polyline.vertices);

                    if(seenPolyline.containsKey(polyline.hashCode()))
                    {
                        if(polyline.equals(seenPolyline.get(polyline.hashCode())))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenPolyline.put(polyline.hashCode(), polyline);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(polyline.hashCode() == polyline.hashCode());
                }
                test(structCollisions < maxCollisions);

                java.util.Map<Integer, ColorPalette> seenColorPalette = new java.util.HashMap<Integer, ColorPalette>();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    ColorPalette colorPalette = new ColorPalette();
                    colorPalette.colors = new java.util.HashMap<Integer, Color>();
                    for(int j = 0; j < 100; ++j)
                    {
                        colorPalette.colors.put(j, new Color(rand.nextInt(255), rand.nextInt(255), rand.nextInt(255), rand.nextInt(255)));
                    }

                    if(seenColorPalette.containsKey(colorPalette.hashCode()))
                    {
                        if(colorPalette.equals(seenColorPalette.get(colorPalette.hashCode())))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenColorPalette.put(colorPalette.hashCode(), colorPalette);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(colorPalette.hashCode() == colorPalette.hashCode());
                }
                test(structCollisions < maxCollisions);

                java.util.Map<Integer, Color> seenColor = new java.util.HashMap<Integer, Color>();
                rand = new java.util.Random();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Color c = new Color(rand.nextInt(255), rand.nextInt(255), rand.nextInt(255), rand.nextInt(255));
                    if(seenColor.containsKey(c.hashCode()))
                    {
                        if(c.equals(seenColor.get(c.hashCode())))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenColor.put(c.hashCode(), c);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(c.hashCode() == c.hashCode());
                }
                test(structCollisions < maxCollisions);
            
                structCollisions = 0;
                java.util.Map<Integer, Draw> seenDraw = new java.util.HashMap<Integer, Draw>();
                structCollisions = 0;
                for(i = 0; i < maxIterations && structCollisions < maxCollisions; ++i)
                {
                    Draw draw =  new Draw(
                        new Color(rand.nextInt(255), rand.nextInt(255), rand.nextInt(255), rand.nextInt(255)),
                        new Pen(rand.nextInt(10), 
                                     new Color(rand.nextInt(255), rand.nextInt(255), rand.nextInt(255), rand.nextInt(255))),
                                     false);

                    if(seenDraw.containsKey(draw.hashCode()))
                    {
                        if(draw.equals(seenDraw.get(draw.hashCode())))
                        {
                            continue; // same object
                        }
                        structCollisions++;
                    }
                    else
                    {
                        seenDraw.put(draw.hashCode(), draw);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(draw.hashCode() == draw.hashCode());
                }
                test(structCollisions < maxCollisions);
            }
            out.println("ok");
            
            if(communicator != null)
            {
                try
                {
                    communicator.destroy();
                }
                catch(Ice.LocalException ex)
                {
                    System.out.println(ex.toString());
                    status = 1;
                }
            }
        }
        catch(Exception ex)
        {
            System.out.println(ex.toString());
            status = 1;
        }
        return status;
    }
}

