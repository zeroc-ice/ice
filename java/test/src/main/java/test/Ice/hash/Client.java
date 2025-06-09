// Copyright (c) ZeroC, Inc.

package test.Ice.hash;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.ProxyIdentityFacetKey;
import com.zeroc.Ice.ProxyIdentityKey;

import test.Ice.hash.Test.Color;
import test.Ice.hash.Test.ColorPalette;
import test.Ice.hash.Test.Draw;
import test.Ice.hash.Test.Pen;
import test.Ice.hash.Test.Point;
import test.Ice.hash.Test.PointD;
import test.Ice.hash.Test.PointF;
import test.Ice.hash.Test.Polyline;
import test.TestHelper;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class Client extends TestHelper {
    public void run(String[] args) {
        PrintWriter out = getWriter();
        Properties properties = createTestProperties(args);
        try (Communicator communicator = initialize(properties)) {
            Map<Integer, ObjectPrx> seenProxy = new HashMap<>();
            Map<Integer, Endpoint> seenEndpoint = new HashMap<>();
            int proxyCollisions = 0;
            int endpointCollisions = 0;
            int i = 0;
            int maxCollisions = 10;
            int maxIterations = 10000;

            out.print("testing proxy & endpoint hash algorithm collisions... ");
            out.flush();
            {
                Random rand = new Random();
                for (i = 0;
                        proxyCollisions < maxCollisions
                                && endpointCollisions < maxCollisions
                                && i < maxIterations;
                        i++) {
                    StringWriter sw = new StringWriter();
                    sw.write(Integer.toString(i));
                    sw.write(":tcp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -t 10");
                    sw.write(Integer.toString(rand.nextInt(1000000)));
                    sw.write(":udp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -h ");
                    sw.write(Integer.toString(rand.nextInt(100)));

                    ObjectPrx obj = communicator.stringToProxy(sw.toString());
                    List<Endpoint> endpoints =
                        new ArrayList<>(
                            Arrays.asList(obj.ice_getEndpoints()));
                    if (seenProxy.containsKey(obj.hashCode())) {
                        if (obj.equals(seenProxy.get(obj.hashCode()))) {
                            continue; // Same object
                        }
                        ++proxyCollisions;
                    } else {
                        seenProxy.put(obj.hashCode(), obj);
                    }

                    Iterator<Endpoint> j = endpoints.iterator();
                    while (j.hasNext()) {
                        Endpoint endpoint = j.next();
                        if (seenEndpoint.containsKey(endpoint.hashCode())) {
                            if (endpoint.equals(seenEndpoint.get(endpoint.hashCode()))) {
                                continue; // Same endpoint
                            }
                            ++endpointCollisions;
                        } else {
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
                seenProxy = new HashMap<>();
                for (i = 0;
                        proxyCollisions < maxCollisions
                                && endpointCollisions < maxCollisions
                                && i < maxIterations;
                        i++) {
                    StringWriter sw = new StringWriter();
                    sw.write(Integer.toString(i));
                    sw.write(":tcp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -t 10");
                    sw.write(Integer.toString(rand.nextInt(1000000)));
                    sw.write(":udp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -h ");
                    sw.write(Integer.toString(rand.nextInt(100)));

                    ProxyIdentityKey obj =
                        new ProxyIdentityKey(communicator.stringToProxy(sw.toString()));
                    if (seenProxy.containsKey(obj.hashCode())) {
                        ++proxyCollisions;
                    } else {
                        seenProxy.put(obj.hashCode(), obj.getProxy());
                    }
                    //
                    // Check the same proxy produce always the same hash
                    //
                    test(obj.hashCode() == obj.hashCode());
                }
                test(proxyCollisions < maxCollisions);

                proxyCollisions = 0;
                seenProxy = new HashMap<>();
                for (i = 0;
                        proxyCollisions < maxCollisions
                                && endpointCollisions < maxCollisions
                                && i < maxIterations;
                        i++) {
                    StringWriter sw = new StringWriter();
                    sw.write(Integer.toString(i));
                    sw.write(":tcp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -t 10");
                    sw.write(Integer.toString(rand.nextInt(1000000)));
                    sw.write(":udp -p ");
                    sw.write(Integer.toString(rand.nextInt(65536)));
                    sw.write(" -h ");
                    sw.write(Integer.toString(rand.nextInt(100)));

                    ProxyIdentityFacetKey obj =
                        new ProxyIdentityFacetKey(
                            communicator.stringToProxy(sw.toString()));
                    if (seenProxy.containsKey(obj.hashCode())) {
                        ++proxyCollisions;
                    } else {
                        seenProxy.put(obj.hashCode(), obj.getProxy());
                    }
                    //
                    // Check the same proxy produce always the same hash
                    //
                    test(obj.hashCode() == obj.hashCode());
                }
                test(proxyCollisions < maxCollisions);

                ObjectPrx prx1 = communicator.stringToProxy("Glacier2/router:tcp -p 10010");
                ObjectPrx prx2 = communicator.stringToProxy("Glacier2/router:ssl -p 10011");
                ObjectPrx prx3 = communicator.stringToProxy("Glacier2/router:udp -p 10012");
                ObjectPrx prx4 =
                    communicator.stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010");
                ObjectPrx prx5 =
                    communicator.stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011");
                ObjectPrx prx6 =
                    communicator.stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012");
                ObjectPrx prx7 =
                    communicator.stringToProxy("Glacier2/router:tcp -p 10010 -t 10000");
                ObjectPrx prx8 =
                    communicator.stringToProxy("Glacier2/router:ssl -p 10011 -t 10000");
                ObjectPrx prx9 =
                    communicator.stringToProxy(
                        "Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000");
                ObjectPrx prx10 =
                    communicator.stringToProxy(
                        "Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000");

                Map<String, Integer> proxyMap = new HashMap<>();
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

                test(
                    communicator.stringToProxy("Glacier2/router:tcp -p 10010").hashCode()
                        == proxyMap.get("prx1"));
                test(
                    communicator.stringToProxy("Glacier2/router:ssl -p 10011").hashCode()
                        == proxyMap.get("prx2"));
                test(
                    communicator.stringToProxy("Glacier2/router:udp -p 10012").hashCode()
                        == proxyMap.get("prx3"));
                test(
                    communicator
                        .stringToProxy("Glacier2/router:tcp -h zeroc.com -p 10010")
                        .hashCode()
                        == proxyMap.get("prx4"));
                test(
                    communicator
                        .stringToProxy("Glacier2/router:ssl -h zeroc.com -p 10011")
                        .hashCode()
                        == proxyMap.get("prx5"));
                test(
                    communicator
                        .stringToProxy("Glacier2/router:udp -h zeroc.com -p 10012")
                        .hashCode()
                        == proxyMap.get("prx6"));
                test(
                    communicator
                        .stringToProxy("Glacier2/router:tcp -p 10010 -t 10000")
                        .hashCode()
                        == proxyMap.get("prx7"));
                test(
                    communicator
                        .stringToProxy("Glacier2/router:ssl -p 10011 -t 10000")
                        .hashCode()
                        == proxyMap.get("prx8"));
                test(
                    communicator
                        .stringToProxy(
                            "Glacier2/router:tcp -h zeroc.com -p 10010 -t 10000")
                        .hashCode()
                        == proxyMap.get("prx9"));
                test(
                    communicator
                        .stringToProxy(
                            "Glacier2/router:ssl -h zeroc.com -p 10011 -t 10000")
                        .hashCode()
                        == proxyMap.get("prx10"));

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx1).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx1).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx2).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx2).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx3).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx3).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx4).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx4).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx5).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx5).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx6).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx6).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx7).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx7).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx8).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx8).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx9).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx9).hashCode());

                test(
                    new ProxyIdentityKey(prx1).hashCode()
                        == new ProxyIdentityKey(prx10).hashCode());
                test(
                    new ProxyIdentityFacetKey(prx1).hashCode()
                        == new ProxyIdentityFacetKey(prx10).hashCode());
            }
            out.println("ok");

            out.print("testing proxy hash of slightly different proxies... ");
            out.flush();
            {
                final String[] proxyString = {
                    "test:tcp -p 10001 -h hello.zeroc.com",
                    "test:udp -p 10001 -h hello.zeroc.com",
                    "test:ssl -p 10001 -h hello.zeroc.com",
                    "test:tcp -p 10001 -h hello.zeroc.com -t 10000",
                    "test -f fa:tcp -p 10001 -h hello.zeroc.com",
                    "test @ adapt",
                    "test @ adapt2",
                    "test:opaque -t 12 -v abcd",
                    "test:opaque -t 13 -v abcd",
                    "test:opaque -t 13 -v abce",
                };

                var hashes = new HashSet<Integer>();

                for (String s : proxyString) {
                    boolean inserted = hashes.add(communicator.stringToProxy(s).hashCode());
                    test(inserted);
                }
            }
            out.println("ok");

            out.print("testing struct hash algorithm collisions... ");
            out.flush();
            {
                Map<Integer, PointF> seenPointF = new HashMap<>();
                Random rand = new Random();
                int structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; i++) {
                    PointF pf = new PointF(rand.nextFloat(), rand.nextFloat(), rand.nextFloat());
                    if (seenPointF.containsKey(pf.hashCode())) {
                        if (pf.equals(seenPointF.get(pf.hashCode()))) {
                            continue; // same object
                        }
                        structCollisions++;
                    } else {
                        seenPointF.put(pf.hashCode(), pf);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(pf.hashCode() == pf.hashCode());
                }
                test(structCollisions < maxCollisions);

                Map<Integer, PointD> seenPointD = new HashMap<>();
                rand = new Random();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; i++) {
                    PointD pd = new PointD(rand.nextDouble(), rand.nextDouble(), rand.nextDouble());
                    if (seenPointD.containsKey(pd.hashCode())) {
                        if (pd.equals(seenPointF.get(pd.hashCode()))) {
                            continue; // same object
                        }
                        structCollisions++;
                    } else {
                        seenPointD.put(pd.hashCode(), pd);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(pd.hashCode() == pd.hashCode());
                }
                test(structCollisions < maxCollisions);

                Map<Integer, Polyline> seenPolyline = new HashMap<>();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; i++) {
                    Polyline polyline = new Polyline();
                    List<Point> vertices = new ArrayList<>();
                    for (int j = 0; j < 100; j++) {
                        vertices.add(new Point(rand.nextInt(100), rand.nextInt(100)));
                    }
                    polyline.vertices = new Point[vertices.size()];
                    vertices.toArray(polyline.vertices);

                    if (seenPolyline.containsKey(polyline.hashCode())) {
                        if (polyline.equals(seenPolyline.get(polyline.hashCode()))) {
                            continue; // same object
                        }
                        structCollisions++;
                    } else {
                        seenPolyline.put(polyline.hashCode(), polyline);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(polyline.hashCode() == polyline.hashCode());
                }
                test(structCollisions < maxCollisions);

                Map<Integer, ColorPalette> seenColorPalette = new HashMap<>();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; i++) {
                    ColorPalette colorPalette = new ColorPalette();
                    colorPalette.colors = new HashMap<>();
                    for (int j = 0; j < 100; j++) {
                        colorPalette.colors.put(
                            j,
                            new Color(
                                rand.nextInt(255),
                                rand.nextInt(255),
                                rand.nextInt(255),
                                rand.nextInt(255)));
                    }

                    if (seenColorPalette.containsKey(colorPalette.hashCode())) {
                        if (colorPalette.equals(seenColorPalette.get(colorPalette.hashCode()))) {
                            continue; // same object
                        }
                        structCollisions++;
                    } else {
                        seenColorPalette.put(colorPalette.hashCode(), colorPalette);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(colorPalette.hashCode() == colorPalette.hashCode());
                }
                test(structCollisions < maxCollisions);

                Map<Integer, Color> seenColor = new HashMap<>();
                rand = new Random();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; i++) {
                    Color c =
                        new Color(
                            rand.nextInt(255),
                            rand.nextInt(255),
                            rand.nextInt(255),
                            rand.nextInt(255));
                    if (seenColor.containsKey(c.hashCode())) {
                        if (c.equals(seenColor.get(c.hashCode()))) {
                            continue; // same object
                        }
                        structCollisions++;
                    } else {
                        seenColor.put(c.hashCode(), c);
                    }
                    //
                    // Check the same struct produce always the same hash
                    //
                    test(c.hashCode() == c.hashCode());
                }
                test(structCollisions < maxCollisions);

                structCollisions = 0;
                Map<Integer, Draw> seenDraw = new HashMap<>();
                structCollisions = 0;
                for (i = 0; i < maxIterations && structCollisions < maxCollisions; i++) {
                    Draw draw =
                        new Draw(
                            new Color(
                                rand.nextInt(255),
                                rand.nextInt(255),
                                rand.nextInt(255),
                                rand.nextInt(255)),
                            new Pen(
                                rand.nextInt(10),
                                new Color(
                                    rand.nextInt(255),
                                    rand.nextInt(255),
                                    rand.nextInt(255),
                                    rand.nextInt(255))),
                            false);

                    if (seenDraw.containsKey(draw.hashCode())) {
                        if (draw.equals(seenDraw.get(draw.hashCode()))) {
                            continue; // same object
                        }
                        structCollisions++;
                    } else {
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
        }
    }
}
