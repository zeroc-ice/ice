// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

import java.io.PrintWriter;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;

import com.zeroc.IceMX.*;

import test.Ice.metrics.Test.*;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static String getPort(com.zeroc.Ice.PropertiesAdminPrx p)
    {
        return Integer.toString(test.Util.Application.getTestPort(p.ice_getCommunicator().getProperties(), 0));
    }

    static ConnectionMetrics getServerConnectionMetrics(MetricsAdminPrx metrics, long expected)
    {
        try
        {
            ConnectionMetrics s;
            MetricsAdmin.GetMetricsViewResult r = metrics.getMetricsView("View");
            s = (ConnectionMetrics)r.returnValue.get("Connection")[0];
            int nRetry = 30;
            while(s.sentBytes != expected && nRetry-- > 0)
            {
                // On some platforms, it's necessary to wait a little before obtaining the server metrics
                // to get an accurate sentBytes metric. The sentBytes metric is updated before the response
                // to the operation is sent and getMetricsView can be dispatched before the metric is really
                // updated.
                try
                {
                    Thread.sleep(100);
                }
                catch(InterruptedException ex)
                {
                }
                r = metrics.getMetricsView("View");
                s = (ConnectionMetrics)r.returnValue.get("Connection")[0];
            }
            return s;
        }
        catch(UnknownMetricsView ex)
        {
            assert(false);
            return null;
        }
    }

    static void waitForObserverCurrent(ObserverI observer, int value)
    {
        for(int i = 0; i < 10; ++i)
        {
            if(observer.getCurrent() != value)
            {
                try
                {
                    Thread.sleep(10);
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            else
            {
                break;
            }
        }
    }

    static class Callback
    {
        public Callback()
        {
            _wait = true;
        }

        synchronized public void completed()
        {
            _wait = false;
            notify();
        }

        synchronized public void waitForResponse()
        {
            while(_wait)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
            _wait = true;
        }

        private boolean _wait;
    }

    static private Map<String, String> getClientProps(com.zeroc.Ice.PropertiesAdminPrx p,
                                                      Map<String, String> orig, String m)
    {
        Map<String, String> props = p.getPropertiesForPrefix("IceMX.Metrics");
        for(Map.Entry<String, String> e : props.entrySet())
        {
            e.setValue("");
        }
        for(Map.Entry<String, String> e : orig.entrySet())
        {
            props.put(e.getKey(), e.getValue());
        }
        String map = "";
        if(!m.isEmpty())
        {
            map += "Map." + m + '.';
        }
        props.put("IceMX.Metrics.View." + map + "Reject.parent", "Ice\\.Admin");
        props.put("IceMX.Metrics.View." + map + "Accept.endpointPort", getPort(p));
        props.put("IceMX.Metrics.View." + map + "Reject.identity", ".*/admin|controller");
        return props;
    }

    static private Map<String, String> getServerProps(com.zeroc.Ice.PropertiesAdminPrx p,
                                                      Map<String, String> orig, String m)
    {
        Map<String, String> props = p.getPropertiesForPrefix("IceMX.Metrics");
        for(Map.Entry<String, String> e : props.entrySet())
        {
            e.setValue("");
        }
        for(Map.Entry<String, String> e : orig.entrySet())
        {
            props.put(e.getKey(), e.getValue());
        }
        String map = "";
        if(!m.isEmpty())
        {
            map += "Map." + m + '.';
        }
        props.put("IceMX.Metrics.View." + map + "Reject.parent", "Ice\\.Admin|Controller");
        props.put("IceMX.Metrics.View." + map + "Accept.endpointPort", getPort(p));
        return props;
    }

    static void waitForCurrent(MetricsAdminPrx metrics, String viewName, String map, int value)
        throws UnknownMetricsView
    {
        while(true)
        {
            MetricsAdmin.GetMetricsViewResult r = metrics.getMetricsView(viewName);
            test(r.returnValue.containsKey(map));
            boolean ok = true;
            for(com.zeroc.IceMX.Metrics m : r.returnValue.get(map))
            {
                if(m.current != value)
                {
                    ok = false;
                    break;
                }
            }
            if(ok)
            {
                break;
            }
            try
            {
                Thread.sleep(50);
            }
            catch(InterruptedException ex)
            {
            }
        }
    }

    static void testAttribute(MetricsAdminPrx metrics,
                              com.zeroc.Ice.PropertiesAdminPrx props,
                              String map,
                              String attr,
                              String value,
                              Runnable func,
                              PrintWriter out)
        throws UnknownMetricsView
    {
        Map<String, String> dict = new java.util.HashMap<>();
        dict.put("IceMX.Metrics.View.Map." + map + ".GroupBy", attr);
        if(props.ice_getIdentity().category.equals("client"))
        {
            props.setProperties(getClientProps(props, dict, map));
        }
        else
        {
            props.setProperties(getServerProps(props, dict, map));
        }

        func.run();
        MetricsAdmin.GetMetricsViewResult r = metrics.getMetricsView("View");
        if(!r.returnValue.containsKey(map) || r.returnValue.get(map).length == 0)
        {
            if(!value.isEmpty())
            {
                out.println("no map `" + map + "' for group by = `" + attr + "'");
                test(false);
            }
        }
        else if(!r.returnValue.get(map)[0].id.equals(value))
        {
            out.println("invalid attribute value: " + attr + " = " + value + " got " + r.returnValue.get(map)[0].id);
            test(false);
        }

        dict.clear();
        if(props.ice_getIdentity().category.equals("client"))
        {
            props.setProperties(getClientProps(props, dict, map));
        }
        else
        {
            props.setProperties(getServerProps(props, dict, map));
        }
    }

    static class Noop implements Runnable
    {
        @Override
        public void run()
        {
        }
    }

    static class Connect implements Runnable
    {
        public Connect(com.zeroc.Ice.ObjectPrx proxy)
        {
            this.proxy = proxy;
        }

        @Override
        public void run()
        {
            if(proxy.ice_getCachedConnection() != null)
            {
                proxy.ice_getCachedConnection().close(false);
            }

            try
            {
                proxy.ice_ping();
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
            }

            if(proxy.ice_getCachedConnection() != null)
            {
                proxy.ice_getCachedConnection().close(false);
            }
        }

        final private com.zeroc.Ice.ObjectPrx proxy;
    }

    static class InvokeOp implements Runnable
    {
        public InvokeOp(MetricsPrx proxy)
        {
            this.proxy = proxy;
        }

        @Override
        public void run()
        {
            Map<String, String> ctx = new java.util.HashMap<>();
            ctx.put("entry1", "test");
            ctx.put("entry2", "");
            proxy.op(ctx);
        }

        final private MetricsPrx proxy;
    }

    static void testAttribute(MetricsAdminPrx metrics,
                              com.zeroc.Ice.PropertiesAdminPrx props,
                              String map,
                              String attr,
                              String value,
                              PrintWriter out)
        throws UnknownMetricsView
    {
        testAttribute(metrics, props, map, attr, value, new Noop(), out);
    }

    static void updateProps(com.zeroc.Ice.PropertiesAdminPrx cprops,
                            com.zeroc.Ice.PropertiesAdminPrx sprops,
                            Map<String, String> props,
                            String map)
    {
        if(sprops.ice_getConnection() != null)
        {
            cprops.setProperties(getClientProps(cprops, props, map));
            sprops.setProperties(getServerProps(sprops, props, map));
        }
        else
        {
            Map<String, String> clientProps = getClientProps(cprops, props, map);
            Map<String, String> serverProps = getServerProps(sprops, props, map);
            serverProps.putAll(clientProps);
            cprops.setProperties(serverProps);
        }
    }

    static void clearView(com.zeroc.Ice.PropertiesAdminPrx cprops, com.zeroc.Ice.PropertiesAdminPrx sprops)
    {
        Map<String, String> dict;

        dict = cprops.getPropertiesForPrefix("IceMX.Metrics");
        dict.put("IceMX.Metrics.View.Disabled", "1");
        cprops.setProperties(dict);

        dict = sprops.getPropertiesForPrefix("IceMX.Metrics");
        dict.put("IceMX.Metrics.View.Disabled", "1");
        sprops.setProperties(dict);

        dict = cprops.getPropertiesForPrefix("IceMX.Metrics");
        dict.put("IceMX.Metrics.View.Disabled", "");
        cprops.setProperties(dict);

        dict = sprops.getPropertiesForPrefix("IceMX.Metrics");
        dict.put("IceMX.Metrics.View.Disabled", "");
        sprops.setProperties(dict);
    }

    static void checkFailure(MetricsAdminPrx m, String map, String id, String failure, int count, PrintWriter out)
        throws UnknownMetricsView
    {
        MetricsFailures f = m.getMetricsFailures("View", map, id);
        if(!f.failures.containsKey(failure))
        {
            out.println("couldn't find failure `" + failure + "' for `" + id + "'");
            test(false);
        }
        if(count > 0 && f.failures.get(failure) != count)
        {
            out.print("count for failure `" + failure + "' of `" + id + "' is different from expected: ");
            out.println(count + " != " + f.failures.get(failure));
            test(false);
        }
    }

    static Map<String, com.zeroc.IceMX.Metrics> toMap(com.zeroc.IceMX.Metrics[] mmap)
    {
        Map<String, com.zeroc.IceMX.Metrics> m = new java.util.HashMap<>();
        for(com.zeroc.IceMX.Metrics e : mmap)
        {
            m.put(e.id, e);
        }
        return m;
    }

    static MetricsPrx allTests(test.Util.Application app, CommunicatorObserverI obsv)
        throws UnknownMetricsView
    {
        PrintWriter out = app.getWriter();
        com.zeroc.Ice.Communicator communicator = app.communicator();

        String host = app.getTestHost();
        String port = Integer.toString(app.getTestPort(0));
        String hostAndPort = host + ":" + port;
        String protocol = app.getTestProtocol();
        String endpoint = protocol + " -h " + host + " -p " + port;

        MetricsPrx metrics = MetricsPrx.checkedCast(communicator.stringToProxy("metrics:" + endpoint));
        boolean collocated = metrics.ice_getConnection() == null;

        int threadCount = 4;
        if(collocated && communicator.getProperties().getPropertyAsInt("Ice.ThreadInterruptSafe") > 0)
        {
            threadCount = 6;
        }

        out.print("testing metrics admin facet checkedCast... ");
        out.flush();
        com.zeroc.Ice.ObjectPrx admin = communicator.getAdmin();
        com.zeroc.Ice.PropertiesAdminPrx clientProps =
            com.zeroc.Ice.PropertiesAdminPrx.checkedCast(admin, "Properties");
        MetricsAdminPrx clientMetrics = MetricsAdminPrx.checkedCast(admin, "Metrics");
        test(clientProps != null && clientMetrics != null);

        admin = metrics.getAdmin();
        com.zeroc.Ice.PropertiesAdminPrx serverProps =
            com.zeroc.Ice.PropertiesAdminPrx.checkedCast(admin, "Properties");
        MetricsAdminPrx serverMetrics = MetricsAdminPrx.checkedCast(admin, "Metrics");
        test(serverProps != null && serverMetrics != null);

        out.println("ok");

        Map<String, String> props = new java.util.HashMap<>();

        out.print("testing group by none...");
        out.flush();

        props.put("IceMX.Metrics.View.GroupBy", "none");
        updateProps(clientProps, serverProps, props, "");
        MetricsAdmin.GetMetricsViewResult r = clientMetrics.getMetricsView("View");
        if(!collocated)
        {
            test(r.returnValue.get("Connection").length == 1 && r.returnValue.get("Connection")[0].current == 1 &&
                 r.returnValue.get("Connection")[0].total == 1);
        }
        test(r.returnValue.get("Thread").length == 1 && r.returnValue.get("Thread")[0].current == threadCount &&
             r.returnValue.get("Thread")[0].total == threadCount);
        out.println("ok");

        out.print("testing group by id...");
        out.flush();

        props.put("IceMX.Metrics.View.GroupBy", "id");
        updateProps(clientProps, serverProps, props, "");

        metrics.ice_ping();
        metrics.ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();

        waitForCurrent(clientMetrics, "View", "Invocation", 0);

        r = clientMetrics.getMetricsView("View");
        test(r.returnValue.get("Thread").length == threadCount);
        if(!collocated)
        {
            test(r.returnValue.get("Connection").length == 2);
        }
        test(r.returnValue.get("Invocation").length == 1);

        InvocationMetrics invoke = (InvocationMetrics)r.returnValue.get("Invocation")[0];
        test(invoke.id.indexOf("[ice_ping]") > 0 && invoke.current == 0 && invoke.total == 5);
        if(!collocated)
        {
            test(invoke.remotes.length == 2);
            test(invoke.remotes[0].total >= 2 && invoke.remotes[1].total >= 2);
            test((invoke.remotes[0].total + invoke.remotes[1].total) == 5);
        }
        else
        {
            test(invoke.collocated.length == 1);
            test(invoke.collocated[0].total == 5);
        }
        r = serverMetrics.getMetricsView("View");
        if(!collocated)
        {
            test(r.returnValue.get("Thread").length > 3);
            test(r.returnValue.get("Connection").length == 2);
        }
        test(r.returnValue.get("Dispatch").length == 1);
        test(r.returnValue.get("Dispatch")[0].current <= 1 && r.returnValue.get("Dispatch")[0].total == 5);
        test(r.returnValue.get("Dispatch")[0].id.indexOf("[ice_ping]") > 0);

        if(!collocated)
        {
            metrics.ice_getConnection().close(false);
            metrics.ice_connectionId("Con1").ice_getConnection().close(false);

            waitForCurrent(clientMetrics, "View", "Connection", 0);
            waitForCurrent(serverMetrics, "View", "Connection", 0);
        }
        clearView(clientProps, serverProps);

        out.println("ok");

        String type = "";
        String isSecure = "";
        if(!collocated)
        {
            com.zeroc.Ice.EndpointInfo endpointInfo = metrics.ice_getConnection().getEndpoint().getInfo();
            type = Short.toString(endpointInfo.type());
            isSecure = endpointInfo.secure() ? "true": "false";
        }

        Map<String, com.zeroc.IceMX.Metrics> map;

        if(!collocated)
        {
            out.print("testing connection metrics... ");
            out.flush();

            props.put("IceMX.Metrics.View.Map.Connection.GroupBy", "none");
            updateProps(clientProps, serverProps, props, "Connection");

            test(clientMetrics.getMetricsView("View").returnValue.get("Connection").length == 0);
            test(serverMetrics.getMetricsView("View").returnValue.get("Connection").length == 0);

            metrics.ice_ping();

            ConnectionMetrics cm1, sm1, cm2, sm2;
            cm1 = (ConnectionMetrics)clientMetrics.getMetricsView("View").returnValue.get("Connection")[0];
            sm1 = getServerConnectionMetrics(serverMetrics, 25);
            test(cm1.total == 1 && sm1.total == 1);

            metrics.ice_ping();

            cm2 = (ConnectionMetrics)clientMetrics.getMetricsView("View").returnValue.get("Connection")[0];
            sm2 = getServerConnectionMetrics(serverMetrics, 50);

            test(cm2.sentBytes - cm1.sentBytes == 45); // 45 for ice_ping request
            test(cm2.receivedBytes - cm1.receivedBytes == 25); // 25 bytes for ice_ping response
            test(sm2.receivedBytes - sm1.receivedBytes == 45);
            test(sm2.sentBytes - sm1.sentBytes == 25);

            cm1 = cm2;
            sm1 = sm2;

            byte[] bs = new byte[0];
            metrics.opByteS(bs);

            cm2 = (ConnectionMetrics)clientMetrics.getMetricsView("View").returnValue.get("Connection")[0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + cm2.receivedBytes - cm1.receivedBytes);
            long requestSz = cm2.sentBytes - cm1.sentBytes;
            long replySz = cm2.receivedBytes - cm1.receivedBytes;

            cm1 = cm2;
            sm1 = sm2;

            bs = new byte[456];
            metrics.opByteS(bs);

            cm2 = (ConnectionMetrics)clientMetrics.getMetricsView("View").returnValue.get("Connection")[0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + replySz);

            test(cm2.sentBytes - cm1.sentBytes == requestSz + bs.length + 4); // 4 is for the seq variable size
            test(cm2.receivedBytes - cm1.receivedBytes == replySz);
            test(sm2.receivedBytes - sm1.receivedBytes == requestSz + bs.length + 4);
            test(sm2.sentBytes - sm1.sentBytes == replySz);

            cm1 = cm2;
            sm1 = sm2;

            bs = new byte[1024 * 1024 * 10]; // Try with large amount of data which should be sent in several chunks
            metrics.opByteS(bs);

            cm2 = (ConnectionMetrics)clientMetrics.getMetricsView("View").returnValue.get("Connection")[0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + replySz);

            test((cm2.sentBytes - cm1.sentBytes) == (requestSz + bs.length + 4)); // 4 is for the seq variable size
            test((cm2.receivedBytes - cm1.receivedBytes) == replySz);
            test((sm2.receivedBytes - sm1.receivedBytes) == (requestSz + bs.length + 4));
            test((sm2.sentBytes - sm1.sentBytes) == replySz);

            props.put("IceMX.Metrics.View.Map.Connection.GroupBy", "state");
            updateProps(clientProps, serverProps, props, "Connection");

            map = toMap(serverMetrics.getMetricsView("View").returnValue.get("Connection"));

            test(map.get("active").current == 1);

            ControllerPrx controller = ControllerPrx.checkedCast(
                communicator.stringToProxy("controller:" + app.getTestEndpoint(1)));
            controller.hold();

            map = toMap(clientMetrics.getMetricsView("View").returnValue.get("Connection"));
            test(map.get("active").current == 1);
            map = toMap(serverMetrics.getMetricsView("View").returnValue.get("Connection"));
            test(map.get("holding").current == 1);

            metrics.ice_getConnection().close(false);

            map = toMap(clientMetrics.getMetricsView("View").returnValue.get("Connection"));
            test(map.get("closing").current == 1);
            map = toMap(serverMetrics.getMetricsView("View").returnValue.get("Connection"));
            test(map.get("holding").current == 1);

            controller.resume();

            map = toMap(serverMetrics.getMetricsView("View").returnValue.get("Connection"));
            test(map.get("holding").current == 0);

            props.put("IceMX.Metrics.View.Map.Connection.GroupBy", "none");
            updateProps(clientProps, serverProps, props, "Connection");

            metrics.ice_getConnection().close(false);

            metrics.ice_timeout(500).ice_ping();
            controller.hold();
            try
            {
                metrics.ice_timeout(500).opByteS(new byte[10000000]);
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
            }
            controller.resume();

            cm1 = (ConnectionMetrics)clientMetrics.getMetricsView("View").returnValue.get("Connection")[0];
            while(true)
            {
                sm1 = (ConnectionMetrics)serverMetrics.getMetricsView("View").returnValue.get("Connection")[0];
                if(sm1.failures >= 2)
                {
                    break;
                }
                try
                {
                    Thread.sleep(10);
                }
                catch(InterruptedException ex)
                {
                }
            }
            test(cm1.failures == 2 && sm1.failures >= 2);

            checkFailure(clientMetrics, "Connection", cm1.id, "::Ice::TimeoutException", 1, out);
            checkFailure(clientMetrics, "Connection", cm1.id, "::Ice::ConnectTimeoutException", 1, out);
            checkFailure(serverMetrics, "Connection", sm1.id, "::Ice::ConnectionLostException", 0, out);

            MetricsPrx m = metrics.ice_timeout(500).ice_connectionId("Con1");
            m.ice_ping();

            testAttribute(clientMetrics, clientProps, "Connection", "parent", "Communicator", out);
            //testAttribute(clientMetrics, clientProps, "Connection", "id", "");
            testAttribute(clientMetrics, clientProps, "Connection", "endpoint",
                          endpoint + " -t 500", out);

            testAttribute(clientMetrics, clientProps, "Connection", "endpointType", type, out);
            testAttribute(clientMetrics, clientProps, "Connection", "endpointIsDatagram", "false", out);
            testAttribute(clientMetrics, clientProps, "Connection", "endpointIsSecure", isSecure, out);
            testAttribute(clientMetrics, clientProps, "Connection", "endpointTimeout", "500", out);
            testAttribute(clientMetrics, clientProps, "Connection", "endpointCompress", "false", out);
            testAttribute(clientMetrics, clientProps, "Connection", "endpointHost", host, out);
            testAttribute(clientMetrics, clientProps, "Connection", "endpointPort", port, out);

            testAttribute(clientMetrics, clientProps, "Connection", "incoming", "false", out);
            testAttribute(clientMetrics, clientProps, "Connection", "adapterName", "", out);
            testAttribute(clientMetrics, clientProps, "Connection", "connectionId", "Con1", out);
            testAttribute(clientMetrics, clientProps, "Connection", "localHost", host, out);
            //testAttribute(clientMetrics, clientProps, "Connection", "localPort", "", out);
            testAttribute(clientMetrics, clientProps, "Connection", "remoteHost", host, out);
            testAttribute(clientMetrics, clientProps, "Connection", "remotePort", port, out);
            testAttribute(clientMetrics, clientProps, "Connection", "mcastHost", "", out);
            testAttribute(clientMetrics, clientProps, "Connection", "mcastPort", "", out);

            m.ice_getConnection().close(false);

            waitForCurrent(clientMetrics, "View", "Connection", 0);
            waitForCurrent(serverMetrics, "View", "Connection", 0);

            out.println("ok");

            out.print("testing connection establishment metrics... ");
            out.flush();

            props.put("IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy", "id");
            updateProps(clientProps, serverProps, props, "ConnectionEstablishment");
            test(clientMetrics.getMetricsView("View").returnValue.get("ConnectionEstablishment").length == 0);

            metrics.ice_ping();

            test(clientMetrics.getMetricsView("View").returnValue.get("ConnectionEstablishment").length == 1);
            com.zeroc.IceMX.Metrics m1 =
                clientMetrics.getMetricsView("View").returnValue.get("ConnectionEstablishment")[0];
            test(m1.current == 0 && m1.total == 1 && m1.id.equals(hostAndPort));

            metrics.ice_getConnection().close(false);
            controller.hold();
            try
            {
                communicator.stringToProxy("test:" + endpoint).ice_timeout(10).ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectTimeoutException ex)
            {
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                test(false);
            }
            controller.resume();
            test(clientMetrics.getMetricsView("View").returnValue.get("ConnectionEstablishment").length == 1);
            m1 = clientMetrics.getMetricsView("View").returnValue.get("ConnectionEstablishment")[0];
            test(m1.id.equals(hostAndPort) && m1.total == 3 && m1.failures == 2);

            checkFailure(clientMetrics, "ConnectionEstablishment", m1.id, "::Ice::ConnectTimeoutException", 2, out);

            Connect c = new Connect(metrics);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "parent", "Communicator", c,
                          out);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "id", hostAndPort, c,
                          out);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "endpoint",
                          endpoint + " -t 60000", c, out);

            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "endpointType", type, c, out);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "endpointIsDatagram", "false",
                          c, out);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "endpointIsSecure", isSecure, c,
                          out);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "endpointTimeout", "60000", c,
                          out);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "endpointCompress", "false", c,
                          out);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "endpointHost", host, c,
                          out);
            testAttribute(clientMetrics, clientProps, "ConnectionEstablishment", "endpointPort", port, c,
                          out);

            out.println("ok");

            out.print("testing endpoint lookup metrics... ");
            out.flush();

            props.put("IceMX.Metrics.View.Map.EndpointLookup.GroupBy", "id");
            updateProps(clientProps, serverProps, props, "EndpointLookup");
            test(clientMetrics.getMetricsView("View").returnValue.get("EndpointLookup").length == 0);

            com.zeroc.Ice.ObjectPrx prx =
                communicator.stringToProxy("metrics:" + protocol + " -p " + port + " -h localhost -t 500");
            try
            {
                prx.ice_ping();
                prx.ice_getConnection().close(false);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
            }

            test(clientMetrics.getMetricsView("View").returnValue.get("EndpointLookup").length == 1);
            m1 = clientMetrics.getMetricsView("View").returnValue.get("EndpointLookup")[0];
            test(m1.current <= 1 && m1.total == 1);

            boolean dnsException = false;
            try
            {
                communicator.stringToProxy("test:tcp -t 500 -h unknownfoo.zeroc.com -p " + port).ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.DNSException ex)
            {
                dnsException = true;
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                // Some DNS servers don't fail on unknown DNS names.
            }
            test(clientMetrics.getMetricsView("View").returnValue.get("EndpointLookup").length == 2);
            m1 = clientMetrics.getMetricsView("View").returnValue.get("EndpointLookup")[0];
            if(!m1.id.equals("tcp -h unknownfoo.zeroc.com -p " + port + " -t 500"))
            {
                m1 = clientMetrics.getMetricsView("View").returnValue.get("EndpointLookup")[1];
            }
            test(m1.id.equals("tcp -h unknownfoo.zeroc.com -p " + port + " -t 500") && m1.total == 2 &&
                 (!dnsException || m1.failures == 2));
            if(dnsException)
            {
                checkFailure(clientMetrics, "EndpointLookup", m1.id, "::Ice::DNSException", 2, out);
            }

            c = new Connect(prx);

            testAttribute(clientMetrics, clientProps, "EndpointLookup", "parent", "Communicator", c, out);
            testAttribute(clientMetrics, clientProps, "EndpointLookup", "id",
                          prx.ice_getConnection().getEndpoint().toString(), c, out);
            testAttribute(clientMetrics, clientProps, "EndpointLookup", "endpoint",
                          prx.ice_getConnection().getEndpoint().toString(), c, out);

            testAttribute(clientMetrics, clientProps, "EndpointLookup", "endpointType", type, c, out);
            testAttribute(clientMetrics, clientProps, "EndpointLookup", "endpointIsDatagram", "false", c, out);
            testAttribute(clientMetrics, clientProps, "EndpointLookup", "endpointIsSecure", isSecure, c, out);
            testAttribute(clientMetrics, clientProps, "EndpointLookup", "endpointTimeout", "500", c, out);
            testAttribute(clientMetrics, clientProps, "EndpointLookup", "endpointCompress", "false", c, out);
            testAttribute(clientMetrics, clientProps, "EndpointLookup", "endpointHost", "localhost", c, out);
            testAttribute(clientMetrics, clientProps, "EndpointLookup", "endpointPort", port, c, out);

            out.println("ok");
        }

        out.print("testing dispatch metrics... ");
        out.flush();

        props.put("IceMX.Metrics.View.Map.Dispatch.GroupBy", "operation");
        updateProps(clientProps, serverProps, props, "Dispatch");
        test(serverMetrics.getMetricsView("View").returnValue.get("Dispatch").length == 0);

        metrics.op();
        try
        {
            metrics.opWithUserException();
            test(false);
        }
        catch(UserEx ex)
        {
        }
        try
        {
            metrics.opWithRequestFailedException();
            test(false);
        }
        catch(com.zeroc.Ice.RequestFailedException ex)
        {
        }
        try
        {
            metrics.opWithLocalException();
            test(false);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
        }
        try
        {
            metrics.opWithUnknownException();
            test(false);
        }
        catch(com.zeroc.Ice.UnknownException ex)
        {
        }
        if(!collocated)
        {
            try
            {
            metrics.fail();
            test(false);
            }
            catch(com.zeroc.Ice.ConnectionLostException ex)
            {
            }
        }
        map = toMap(serverMetrics.getMetricsView("View").returnValue.get("Dispatch"));
        test(map.size() == (!collocated ? 6 : 5));

        DispatchMetrics dm1 = (DispatchMetrics)map.get("op");
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 0 && dm1.userException == 0);
        test(dm1.size == 21 && dm1.replySize == 7);

        dm1 = (DispatchMetrics)map.get("opWithUserException");
        test(dm1.current <= 1 &dm1.total == 1 && dm1.failures == 0 && dm1.userException == 1);
        test(dm1.size == 38 && dm1.replySize == 23);

        dm1 = (DispatchMetrics)map.get("opWithLocalException");
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "::Ice::SyscallException", 1, out);
        test(dm1.size == 39 && dm1.replySize > 7); // Reply contains the exception stack depending on the OS.

        dm1 = (DispatchMetrics)map.get("opWithRequestFailedException");
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "::Ice::ObjectNotExistException", 1, out);
        test(dm1.size == 47 && dm1.replySize == 40);

        dm1 = (DispatchMetrics)map.get("opWithUnknownException");
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "java.lang.IllegalArgumentException", 1, out);
        test(dm1.size == 41 && dm1.replySize > 7); // Reply contains the exception stack depending on the OS.

        InvokeOp op = new InvokeOp(metrics);

        testAttribute(serverMetrics, serverProps, "Dispatch", "parent", "TestAdapter", op, out);
        testAttribute(serverMetrics, serverProps, "Dispatch", "id", "metrics [op]", op, out);
        if(!collocated)
        {
            testAttribute(serverMetrics, serverProps, "Dispatch", "endpoint",
                          endpoint + " -t 60000", op, out);
            //testAttribute(serverMetrics, serverProps, "Dispatch", "connection", "", op);

            testAttribute(serverMetrics, serverProps, "Dispatch", "endpointType", type, op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "endpointIsDatagram", "false", op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "endpointIsSecure", isSecure, op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "endpointTimeout", "60000", op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "endpointCompress", "false", op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "endpointHost", host, op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "endpointPort", port, op, out);

            testAttribute(serverMetrics, serverProps, "Dispatch", "incoming", "true", op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "adapterName", "TestAdapter", op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "connectionId", "", op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "localHost", host, op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "localPort", port, op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "remoteHost", host, op, out);
            //testAttribute(serverMetrics, serverProps, "Dispatch", "remotePort", port, op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "mcastHost", "", op, out);
            testAttribute(serverMetrics, serverProps, "Dispatch", "mcastPort", "", op, out);
        }

        testAttribute(serverMetrics, serverProps, "Dispatch", "operation", "op", op, out);
        testAttribute(serverMetrics, serverProps, "Dispatch", "identity", "metrics", op, out);
        testAttribute(serverMetrics, serverProps, "Dispatch", "facet", "", op, out);
        testAttribute(serverMetrics, serverProps, "Dispatch", "mode", "twoway", op, out);

        testAttribute(serverMetrics, serverProps, "Dispatch", "context.entry1", "test", op, out);
        testAttribute(serverMetrics, serverProps, "Dispatch", "context.entry2", "", op, out);
        testAttribute(serverMetrics, serverProps, "Dispatch", "context.entry3", "", op, out);

        out.println("ok");

        out.print("testing invocation metrics... ");
        out.flush();

        props.put("IceMX.Metrics.View.Map.Invocation.GroupBy", "operation");
        props.put("IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy", "localPort");
        props.put("IceMX.Metrics.View.Map.Invocation.Map.Collocated.GroupBy", "id");
        updateProps(clientProps, serverProps, props, "Invocation");
        test(serverMetrics.getMetricsView("View").returnValue.get("Invocation").length == 0);

        Callback cb = new Callback();

        //
        // Twoway tests
        //
        metrics.op();
        metrics.opAsync().join();
        metrics.opAsync().whenComplete((response, ex) ->
            {
                cb.completed();
            });
        cb.waitForResponse();

        try
        {
            metrics.opWithUserException();
            test(false);
        }
        catch(UserEx ex)
        {
        }
        try
        {
            metrics.opWithUserExceptionAsync().join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof UserEx);
        }
        metrics.opWithUserExceptionAsync().whenComplete((response, ex) ->
            {
                cb.completed();
            });
        cb.waitForResponse();

        try
        {
            metrics.opWithRequestFailedException();
            test(false);
        }
        catch(com.zeroc.Ice.RequestFailedException ex)
        {
        }
        try
        {
            metrics.opWithRequestFailedExceptionAsync().join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof com.zeroc.Ice.RequestFailedException);
        }
        metrics.opWithRequestFailedExceptionAsync().whenComplete((result, ex) ->
            {
                cb.completed();
            });
        cb.waitForResponse();

        try
        {
            metrics.opWithLocalException();
            test(false);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
        }
        try
        {
            metrics.opWithLocalExceptionAsync().join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof com.zeroc.Ice.LocalException);
        }
        metrics.opWithLocalExceptionAsync().whenComplete((result, ex) ->
            {
                cb.completed();
            });
        cb.waitForResponse();

        try
        {
            metrics.opWithUnknownException();
            test(false);
        }
        catch(com.zeroc.Ice.UnknownException ex)
        {
        }
        try
        {
            metrics.opWithUnknownExceptionAsync().join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof com.zeroc.Ice.UnknownException);
        }
        metrics.opWithUnknownExceptionAsync().whenComplete((result, ex) ->
            {
                cb.completed();
            });
        cb.waitForResponse();

        if(!collocated)
        {
            try
            {
                metrics.fail();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectionLostException ex)
            {
            }
            try
            {
                metrics.failAsync().join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.ConnectionLostException);
            }
            metrics.failAsync().whenComplete((result, ex) ->
                {
                    cb.completed();
                });
            cb.waitForResponse();
        }

        map = toMap(clientMetrics.getMetricsView("View").returnValue.get("Invocation"));
        test(map.size() == (collocated ? 5 : 6));

        InvocationMetrics im1;
        ChildInvocationMetrics rim1;
        im1 = (InvocationMetrics)map.get("op");
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 0 && im1.retry == 0);
        test(collocated ? im1.collocated.length == 1 : im1.remotes.length == 1);
        rim1 = (ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 63 && rim1.replySize == 21);

        im1 = (InvocationMetrics)map.get("opWithUserException");
        test(im1.current == 0 && im1.total == 3 && im1.failures == 0 && im1.retry == 0);
        test(collocated ? im1.collocated.length == 1 : im1.remotes.length == 1);
        rim1 = (ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 114 && rim1.replySize == 69);
        test(im1.userException == 3);

        im1 = (InvocationMetrics)map.get("opWithLocalException");
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0);
        test(collocated ? im1.collocated.length == 1 : im1.remotes.length == 1);
        rim1 = (ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 117 && rim1.replySize > 7);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::UnknownLocalException", 3, out);

        im1 = (InvocationMetrics)map.get("opWithRequestFailedException");
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0);
        test(collocated ? im1.collocated.length == 1 : im1.remotes.length == 1);
        rim1 = (ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 141 && rim1.replySize == 120);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::ObjectNotExistException", 3, out);

        im1 = (InvocationMetrics)map.get("opWithUnknownException");
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0);
        test(collocated ? im1.collocated.length == 1 : im1.remotes.length == 1);
        rim1 = (ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 123 && rim1.replySize > 7);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::UnknownException", 3, out);

        if(!collocated)
        {
            im1 = (InvocationMetrics)map.get("fail");
            test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 3 && im1.remotes.length == 6);
            test(im1.remotes[0].current == 0 && im1.remotes[0].total == 1 && im1.remotes[0].failures == 1);
            test(im1.remotes[1].current == 0 && im1.remotes[1].total == 1 && im1.remotes[1].failures == 1);
            test(im1.remotes[2].current == 0 && im1.remotes[2].total == 1 && im1.remotes[2].failures == 1);
            test(im1.remotes[3].current == 0 && im1.remotes[3].total == 1 && im1.remotes[3].failures == 1);
            test(im1.remotes[4].current == 0 && im1.remotes[4].total == 1 && im1.remotes[4].failures == 1);
            test(im1.remotes[5].current == 0 && im1.remotes[5].total == 1 && im1.remotes[5].failures == 1);
            checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::ConnectionLostException", 3, out);
        }

        testAttribute(clientMetrics, clientProps, "Invocation", "parent", "Communicator", op, out);
        testAttribute(clientMetrics, clientProps, "Invocation", "id", "metrics -t -e 1.1 [op]", op, out);

        testAttribute(clientMetrics, clientProps, "Invocation", "operation", "op", op, out);
        testAttribute(clientMetrics, clientProps, "Invocation", "identity", "metrics", op, out);
        testAttribute(clientMetrics, clientProps, "Invocation", "facet", "", op, out);
        testAttribute(clientMetrics, clientProps, "Invocation", "encoding", "1.1", op, out);
        testAttribute(clientMetrics, clientProps, "Invocation", "mode", "twoway", op, out);
        testAttribute(clientMetrics, clientProps, "Invocation", "proxy",
                      "metrics -t -e 1.1:" + endpoint + " -t 60000", op, out);

        testAttribute(clientMetrics, clientProps, "Invocation", "context.entry1", "test", op, out);
        testAttribute(clientMetrics, clientProps, "Invocation", "context.entry2", "", op, out);
        testAttribute(clientMetrics, clientProps, "Invocation", "context.entry3", "", op, out);

        //
        // Oneway tests
        //
        clearView(clientProps, serverProps);
        props.put("IceMX.Metrics.View.Map.Invocation.GroupBy", "operation");
        props.put("IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy", "localPort");
        updateProps(clientProps, serverProps, props, "Invocation");

        MetricsPrx metricsOneway = metrics.ice_oneway();
        metricsOneway.op();
        metricsOneway.opAsync().join();
        {
            CompletableFuture<Void> f = metricsOneway.opAsync();
            com.zeroc.Ice.Util.getInvocationFuture(f).waitForSent();
        }

        map = toMap(clientMetrics.getMetricsView("View").returnValue.get("Invocation"));
        test(map.size() == 1);

        im1 = (InvocationMetrics)map.get("op");
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 0 && im1.retry == 0);
        test(collocated ? (im1.collocated.length == 1) : (im1.remotes.length == 1));
        rim1 = (ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current <= 1 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 63 && rim1.replySize == 0);

        testAttribute(clientMetrics, clientProps, "Invocation", "mode", "oneway", new InvokeOp(metricsOneway),
                      out);

        //
        // Batch oneway tests
        //
        props.put("IceMX.Metrics.View.Map.Invocation.GroupBy", "operation");
        props.put("IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy", "localPort");
        updateProps(clientProps, serverProps, props, "Invocation");

        MetricsPrx metricsBatchOneway = metrics.ice_batchOneway();
        metricsBatchOneway.op();
        metricsBatchOneway.opAsync();
        //metricsBatchOneway.opAsync().waitForSent();

        map = toMap(clientMetrics.getMetricsView("View").returnValue.get("Invocation"));
        test(map.size() == 1);

        im1 = (InvocationMetrics)map.get("op");
        test(im1.current == 0 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
        test(im1.remotes.length == 0);

        testAttribute(clientMetrics, clientProps, "Invocation", "mode", "batch-oneway",
                      new InvokeOp(metricsBatchOneway), out);

        out.println("ok");

        out.print("testing metrics view enable/disable...");
        out.flush();

        MetricsAdmin.GetMetricsViewNamesResult n;
        props.put("IceMX.Metrics.View.GroupBy", "none");
        props.put("IceMX.Metrics.View.Disabled", "0");
        updateProps(clientProps, serverProps, props, "Thread");
        test(clientMetrics.getMetricsView("View").returnValue.get("Thread").length != 0);
        n = clientMetrics.getMetricsViewNames();
        test(n.returnValue.length == 1 && n.disabledViews.length == 0);

        props.put("IceMX.Metrics.View.Disabled", "1");
        updateProps(clientProps, serverProps, props, "Thread");
        test(clientMetrics.getMetricsView("View").returnValue.get("Thread") == null);
        n = clientMetrics.getMetricsViewNames();
        test(n.returnValue.length == 0 && n.disabledViews.length == 1);

        clientMetrics.enableMetricsView("View");
        test(clientMetrics.getMetricsView("View").returnValue.get("Thread").length != 0);
        n = clientMetrics.getMetricsViewNames();
        test(n.returnValue.length == 1 && n.disabledViews.length == 0);

        clientMetrics.disableMetricsView("View");
        test(clientMetrics.getMetricsView("View").returnValue.get("Thread") == null);
        n = clientMetrics.getMetricsViewNames();
        test(n.returnValue.length == 0 && n.disabledViews.length == 1);

        try
        {
            clientMetrics.enableMetricsView("UnknownView");
        }
        catch(UnknownMetricsView ex)
        {
        }

        out.println("ok");

        out.print("testing instrumentation observer delegate... ");
        out.flush();

        test(obsv.threadObserver.getTotal() > 0);
        if(!collocated)
        {
            test(obsv.connectionObserver.getTotal() > 0);
            test(obsv.connectionEstablishmentObserver.getTotal() > 0);
            test(obsv.endpointLookupObserver.getTotal() > 0);
            test(obsv.invocationObserver.remoteObserver.getTotal() > 0);
        }
        else
        {
            test(obsv.invocationObserver.collocatedObserver.getTotal() > 0);
        }
        test(obsv.dispatchObserver.getTotal() > 0);
        test(obsv.invocationObserver.getTotal() > 0);

        test(obsv.threadObserver.getCurrent() > 0);
        if(!collocated)
        {
            test(obsv.connectionObserver.getCurrent() > 0);
            test(obsv.connectionEstablishmentObserver.getCurrent() == 0);
            test(obsv.endpointLookupObserver.getCurrent() == 0);
            waitForObserverCurrent(obsv.invocationObserver.remoteObserver, 0);
            test(obsv.invocationObserver.remoteObserver.getCurrent() == 0);
        }
        else
        {
            waitForObserverCurrent(obsv.invocationObserver.collocatedObserver, 0);
            test(obsv.invocationObserver.collocatedObserver.getCurrent() == 0);
        }
        waitForObserverCurrent(obsv.dispatchObserver, 0);
        test(obsv.dispatchObserver.getCurrent() == 0);
        waitForObserverCurrent(obsv.invocationObserver, 0);
        test(obsv.invocationObserver.getCurrent() == 0);

        test(obsv.threadObserver.getFailedCount() == 0);
        if(!collocated)
        {
            test(obsv.connectionObserver.getFailedCount() > 0);
            test(obsv.connectionEstablishmentObserver.getFailedCount() > 0);
            test(obsv.endpointLookupObserver.getFailedCount() > 0);
            test(obsv.invocationObserver.remoteObserver.getFailedCount() > 0);
        }
        //test(obsv.dispatchObserver.getFailedCount() > 0);
        test(obsv.invocationObserver.getFailedCount() > 0);

        if(!collocated)
        {
            test(obsv.threadObserver.states > 0);
            test(obsv.connectionObserver.received > 0 && obsv.connectionObserver.sent > 0);
            test(obsv.invocationObserver.retriedCount > 0);
            test(obsv.invocationObserver.remoteObserver.replySize > 0);
        }
        else
        {
            test(obsv.invocationObserver.collocatedObserver.replySize > 0);
        }
        //test(obsv.dispatchObserver.userExceptionCount > 0);
        test(obsv.invocationObserver.userExceptionCount > 0);

        out.println("ok");

        return metrics;
    }
}
