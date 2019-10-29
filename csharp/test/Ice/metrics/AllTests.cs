//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.IO;

using Test;

public class AllTests : Test.AllTests
{
    static IceMX.ConnectionMetrics
    getServerConnectionMetrics(IceMX.MetricsAdminPrx metrics, long expected)
    {
        try
        {
            IceMX.ConnectionMetrics s;
            long timestamp;
            s = (IceMX.ConnectionMetrics)metrics.getMetricsView("View", out timestamp)["Connection"][0];
            int nRetry = 30;
            while (s.sentBytes != expected && nRetry-- > 0)
            {
                // On some platforms, it's necessary to wait a little before obtaining the server metrics
                // to get an accurate sentBytes metric. The sentBytes metric is updated before the response
                // to the operation is sent and getMetricsView can be dispatched before the metric is really
                // updated.
                Thread.Sleep(100);
                s = (IceMX.ConnectionMetrics)metrics.getMetricsView("View", out timestamp)["Connection"][0];
            }
            return s;
        }
        catch (IceMX.UnknownMetricsView)
        {
            Debug.Assert(false);
            return null;
        }
    }

    class Callback
    {
        public Callback()
        {
            _wait = true;
        }

        public void response()
        {
            lock (this)
            {
                _wait = false;
                Monitor.Pulse(this);
            }
        }

        public void exception(Ice.Exception ex)
        {
            response();
        }

        public void waitForResponse()
        {
            lock (this)
            {
                while (_wait)
                {
                    Monitor.Wait(this);
                }
                _wait = true;
            }
        }

        private bool _wait;
    };

    static string
    getPort(Ice.PropertiesAdminPrx p)
    {
        return Test.TestHelper.getTestPort(p.ice_getCommunicator().getProperties(), 0).ToString();
    }

    static private Dictionary<string, string>
    getClientProps(Ice.PropertiesAdminPrx p, Dictionary<string, string> orig, string m)
    {
        Dictionary<string, string> props = p.getPropertiesForPrefix("IceMX.Metrics");
        foreach (string e in new List<string>(props.Keys))
        {
            props[e] = "";
        }
        foreach (KeyValuePair<string, string> e in orig)
        {
            props[e.Key] = e.Value;
        }
        string map = "";
        if (m.Length > 0)
        {
            map += "Map." + m + '.';
        }
        props["IceMX.Metrics.View." + map + "Reject.parent"] = "Ice\\.Admin";
        props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = getPort(p);
        props["IceMX.Metrics.View." + map + "Reject.identity"] = ".*/admin|controller";
        return props;
    }

    static private Dictionary<string, string>
    getServerProps(Ice.PropertiesAdminPrx p, Dictionary<string, string> orig, string m)
    {
        Dictionary<string, string> props = p.getPropertiesForPrefix("IceMX.Metrics");
        foreach (string e in new List<string>(props.Keys))
        {
            props[e] = "";
        }
        foreach (KeyValuePair<string, string> e in orig)
        {
            props[e.Key] = e.Value;
        }
        string map = "";
        if (m.Length > 0)
        {
            map += "Map." + m + '.';
        }
        props["IceMX.Metrics.View." + map + "Reject.parent"] = "Ice\\.Admin|Controller";
        props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = getPort(p);
        return props;
    }

    class UpdateCallbackI
    {
        public UpdateCallbackI(Ice.PropertiesAdminPrx serverProps)
        {
            _updated = false;
            _serverProps = serverProps;
        }

        public void
        waitForUpdate()
        {
            lock (this)
            {
                while (!_updated)
                {
                    Monitor.Wait(this);
                }
            }

            // Ensure that the previous updates were committed, the setProperties call returns before
            // notifying the callbacks so to ensure all the update callbacks have be notified we call
            // a second time, this will block until all the notifications from the first update have
            // completed.
            _serverProps.setProperties(new Dictionary<string, string>());

            lock (this)
            {
                _updated = false;
            }
        }

        public void
        updated(Dictionary<string, string> dict)
        {
            lock (this)
            {
                _updated = true;
                Monitor.Pulse(this);
            }
        }

        private bool _updated;
        private Ice.PropertiesAdminPrx _serverProps;
    };

    static void
    waitForCurrent(IceMX.MetricsAdminPrx metrics, string viewName, string map, int value)
    {
        while (true)
        {
            long timestamp;
            Dictionary<string, IceMX.Metrics[]> view = metrics.getMetricsView(viewName, out timestamp);
            test(view.ContainsKey(map));
            bool ok = true;
            foreach (IceMX.Metrics m in view[map])
            {
                if (m.current != value)
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
            {
                break;
            }
            Thread.Sleep(50);
        }
    }

    static void
    waitForObserverCurrent(ObserverI observer, int value)
    {
        for (int i = 0; i < 10; ++i)
        {
            if (observer.getCurrent() > 0)
            {
                Thread.Sleep(10);
            }
            else
            {
                break;
            }
        }
    }

    static void
    testAttribute(IceMX.MetricsAdminPrx metrics,
                  Ice.PropertiesAdminPrx props,
                  UpdateCallbackI update,
                  string map,
                  string attr,
                  string value,
                  System.Action func,
                  TextWriter output)
    {
        Dictionary<string, string> dict = new Dictionary<string, string>();
        dict.Add("IceMX.Metrics.View.Map." + map + ".GroupBy", attr);
        if (props.ice_getIdentity().category.Equals("client"))
        {
            props.setProperties(getClientProps(props, dict, map));
            update.waitForUpdate();
        }
        else
        {
            props.setProperties(getServerProps(props, dict, map));
            props.setProperties(new Dictionary<string, string>());
        }

        func();
        long timestamp;
        Dictionary<string, IceMX.Metrics[]> view = metrics.getMetricsView("View", out timestamp);
        if (!view.ContainsKey(map) || view[map].Length == 0)
        {
            if (value.Length > 0)
            {
                output.WriteLine("no map `" + map + "' for group by = `" + attr + "'");
                test(false);
            }
        }
        else if (!view[map][0].id.Equals(value))
        {
            output.WriteLine("invalid attribute value: " + attr + " = " + value + " got " + view[map][0].id);
            test(false);
        }

        dict.Clear();
        if (props.ice_getIdentity().category.Equals("client"))
        {
            props.setProperties(getClientProps(props, dict, map));
            update.waitForUpdate();
        }
        else
        {
            props.setProperties(getServerProps(props, dict, map));
            props.setProperties(new Dictionary<string, string>());
        }
    }

    static void connect(Ice.ObjectPrx proxy)
    {
        if (proxy.ice_getCachedConnection() != null)
        {
            proxy.ice_getCachedConnection().close(Ice.ConnectionClose.GracefullyWithWait);
        }

        try
        {
            proxy.ice_ping();
        }
        catch (Ice.LocalException)
        {
        }

        if (proxy.ice_getCachedConnection() != null)
        {
            proxy.ice_getCachedConnection().close(Ice.ConnectionClose.GracefullyWithWait);
        }
    }

    static void invokeOp(MetricsPrx proxy)
    {
        Dictionary<string, string> ctx = new Dictionary<string, string>();
        ctx.Add("entry1", "test");
        ctx.Add("entry2", "");
        proxy.op(ctx);
    }

    static void
    testAttribute(IceMX.MetricsAdminPrx metrics,
                  Ice.PropertiesAdminPrx props,
                  UpdateCallbackI update,
                  string map,
                  string attr,
                  string value,
                  TextWriter output)
    {
        testAttribute(metrics, props, update, map, attr, value, () => { }, output);
    }

    static void
    updateProps(Ice.PropertiesAdminPrx cprops,
                Ice.PropertiesAdminPrx sprops,
                UpdateCallbackI callback,
                Dictionary<string, string> props,
                string map)
    {
        if (sprops.ice_getConnection() != null)
        {
            cprops.setProperties(getClientProps(cprops, props, map));
            sprops.setProperties(getServerProps(sprops, props, map));
        }
        else
        {
            Dictionary<string, string> clientProps = getClientProps(cprops, props, map);
            Dictionary<string, string> serverProps = getClientProps(sprops, props, map);
            foreach (KeyValuePair<string, string> p in clientProps)
            {
                if (!serverProps.ContainsKey(p.Key))
                {
                    serverProps.Add(p.Key, p.Value);
                }
            }
            cprops.setProperties(serverProps);
        }
        callback.waitForUpdate();
    }

    static void
    clearView(Ice.PropertiesAdminPrx cprops, Ice.PropertiesAdminPrx sprops, UpdateCallbackI callback)
    {
        Dictionary<string, string> dict;

        dict = cprops.getPropertiesForPrefix("IceMX.Metrics");
        dict["IceMX.Metrics.View.Disabled"] = "1";
        cprops.setProperties(dict);

        dict = sprops.getPropertiesForPrefix("IceMX.Metrics");
        dict["IceMX.Metrics.View.Disabled"] = "1";
        sprops.setProperties(dict);

        callback.waitForUpdate();

        dict = cprops.getPropertiesForPrefix("IceMX.Metrics");
        dict["IceMX.Metrics.View.Disabled"] = "";
        cprops.setProperties(dict);

        dict = sprops.getPropertiesForPrefix("IceMX.Metrics");
        dict["IceMX.Metrics.View.Disabled"] = "";
        sprops.setProperties(dict);

        callback.waitForUpdate();
    }

    static void
    checkFailure(IceMX.MetricsAdminPrx m, string map, string id, string failure, int count, TextWriter output)
    {
        IceMX.MetricsFailures f = m.getMetricsFailures("View", map, id);
        if (!f.failures.ContainsKey(failure))
        {
            output.WriteLine("couldn't find failure `" + failure + "' for `" + id + "'");
            test(false);
        }
        if (count > 0 && f.failures[failure] != count)
        {
            output.Write("count for failure `" + failure + "' of `" + id + "' is different from expected: ");
            output.WriteLine(count + " != " + f.failures[failure]);
            test(false);
        }
    }

    static Dictionary<string, IceMX.Metrics>
    toMap(IceMX.Metrics[] mmap)
    {
        Dictionary<string, IceMX.Metrics> m = new Dictionary<string, IceMX.Metrics>();
        foreach (IceMX.Metrics e in mmap)
        {
            m.Add(e.id, e);
        }
        return m;
    }

    public static MetricsPrx allTests(Test.TestHelper helper, CommunicatorObserverI obsv)
    {
        Ice.Communicator communicator = helper.communicator();

        string host = helper.getTestHost();
        string port = helper.getTestPort(0).ToString();
        string hostAndPort = host + ":" + port;
        string protocol = helper.getTestProtocol();
        string endpoint = protocol + " -h " + host + " -p " + port;
        string timeout = communicator.getProperties().getPropertyWithDefault("Ice.Default.Timeout", "60000");

        MetricsPrx metrics = MetricsPrxHelper.checkedCast(communicator.stringToProxy("metrics:" + endpoint));
        bool collocated = metrics.ice_getConnection() == null;
        var output = helper.getWriter();
        output.Write("testing metrics admin facet checkedCast... ");
        output.Flush();
        Ice.ObjectPrx admin = communicator.getAdmin();
        Ice.PropertiesAdminPrx clientProps = Ice.PropertiesAdminPrxHelper.checkedCast(admin, "Properties");
        IceMX.MetricsAdminPrx clientMetrics = IceMX.MetricsAdminPrxHelper.checkedCast(admin, "Metrics");
        test(clientProps != null && clientMetrics != null);

        admin = metrics.getAdmin();
        Ice.PropertiesAdminPrx serverProps = Ice.PropertiesAdminPrxHelper.checkedCast(admin, "Properties");
        IceMX.MetricsAdminPrx serverMetrics = IceMX.MetricsAdminPrxHelper.checkedCast(admin, "Metrics");
        test(serverProps != null && serverMetrics != null);

        UpdateCallbackI update = new UpdateCallbackI(serverProps);
        ((Ice.NativePropertiesAdmin)communicator.findAdminFacet("Properties")).addUpdateCallback(update.updated);

        output.WriteLine("ok");

        Dictionary<string, string> props = new Dictionary<string, string>();

        output.Write("testing group by none...");
        output.Flush();

        props.Add("IceMX.Metrics.View.GroupBy", "none");
        updateProps(clientProps, serverProps, update, props, "");
        long timestamp;
        Dictionary<string, IceMX.Metrics[]> view = clientMetrics.getMetricsView("View", out timestamp);
        if (!collocated)
        {
            test(view["Connection"].Length == 1 && view["Connection"][0].current == 1 &&
                 view["Connection"][0].total == 1);
        }
        test(view["Thread"].Length == 1 && view["Thread"][0].current == 5 && view["Thread"][0].total == 5);
        output.WriteLine("ok");

        output.Write("testing group by id...");
        output.Flush();

        props["IceMX.Metrics.View.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update, props, "");

        metrics.ice_ping();
        metrics.ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();

        waitForCurrent(clientMetrics, "View", "Invocation", 0);
        waitForCurrent(serverMetrics, "View", "Dispatch", 0);

        view = clientMetrics.getMetricsView("View", out timestamp);
        test(view["Thread"].Length == 5);
        if (!collocated)
        {
            test(view["Connection"].Length == 2);
        }
        test(view["Invocation"].Length == 1);

        IceMX.InvocationMetrics invoke = (IceMX.InvocationMetrics)view["Invocation"][0];

        test(invoke.id.IndexOf("[ice_ping]") > 0 && invoke.current == 0 && invoke.total == 5);
        if (!collocated)
        {
            test(invoke.remotes.Length == 2);
            test(invoke.remotes[0].total >= 2 && invoke.remotes[1].total >= 2);
            test((invoke.remotes[0].total + invoke.remotes[1].total) == 5);
        }
        else
        {
            test(invoke.collocated.Length == 1);
            test(invoke.collocated[0].total == 5);
        }

        view = serverMetrics.getMetricsView("View", out timestamp);
        // With Ice for .NET, a new dispatching thread isn't necessarily created.
        //test(view["Thread"].Length > 5);
        if (!collocated)
        {
            test(view["Connection"].Length == 2);
        }
        test(view["Dispatch"].Length == 1);
        test(view["Dispatch"][0].current == 0 && view["Dispatch"][0].total == 5);
        test(view["Dispatch"][0].id.IndexOf("[ice_ping]") > 0);

        if (!collocated)
        {
            metrics.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            metrics.ice_connectionId("Con1").ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

            waitForCurrent(clientMetrics, "View", "Connection", 0);
            waitForCurrent(serverMetrics, "View", "Connection", 0);
        }

        clearView(clientProps, serverProps, update);

        output.WriteLine("ok");

        string type = "";
        string isSecure = "";
        if (!collocated)
        {
            Ice.EndpointInfo endpointInfo = metrics.ice_getConnection().getEndpoint().getInfo();
            type = endpointInfo.type().ToString();
            isSecure = endpointInfo.secure() ? "True" : "False";
        }

        Dictionary<string, IceMX.Metrics> map;

        if (!collocated)
        {
            output.Write("testing connection metrics... ");
            output.Flush();

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
            updateProps(clientProps, serverProps, update, props, "Connection");

            test(clientMetrics.getMetricsView("View", out timestamp)["Connection"].Length == 0);
            test(serverMetrics.getMetricsView("View", out timestamp)["Connection"].Length == 0);

            metrics.ice_ping();

            IceMX.ConnectionMetrics cm1, sm1, cm2, sm2;
            cm1 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
            sm1 = getServerConnectionMetrics(serverMetrics, 25);

            metrics.ice_ping();

            cm2 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
            sm2 = getServerConnectionMetrics(serverMetrics, 50);

            test(cm2.sentBytes - cm1.sentBytes == 45); // 45 for ice_ping request
            test(cm2.receivedBytes - cm1.receivedBytes == 25); // 25 bytes for ice_ping response
            test(sm2.receivedBytes - sm1.receivedBytes == 45);
            test(sm2.sentBytes - sm1.sentBytes == 25);

            cm1 = cm2;
            sm1 = sm2;

            byte[] bs = new byte[0];
            metrics.opByteS(bs);

            cm2 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + cm2.receivedBytes - cm1.receivedBytes);
            long requestSz = cm2.sentBytes - cm1.sentBytes;
            long replySz = cm2.receivedBytes - cm1.receivedBytes;

            cm1 = cm2;
            sm1 = sm2;

            bs = new byte[456];
            metrics.opByteS(bs);

            cm2 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + replySz);

            test(cm2.sentBytes - cm1.sentBytes == requestSz + bs.Length + 4); // 4 is for the seq variable size
            test(cm2.receivedBytes - cm1.receivedBytes == replySz);
            test(sm2.receivedBytes - sm1.receivedBytes == requestSz + bs.Length + 4);
            test(sm2.sentBytes - sm1.sentBytes == replySz);

            cm1 = cm2;
            sm1 = sm2;

            bs = new byte[1024 * 1024 * 10]; // Try with large amount of data which should be sent in several chunks
            metrics.opByteS(bs);

            cm2 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.sentBytes + replySz);

            test((cm2.sentBytes - cm1.sentBytes) == (requestSz + bs.Length + 4)); // 4 is for the seq variable size
            test((cm2.receivedBytes - cm1.receivedBytes) == replySz);
            test((sm2.receivedBytes - sm1.receivedBytes) == (requestSz + bs.Length + 4));
            test((sm2.sentBytes - sm1.sentBytes) == replySz);

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "state";
            updateProps(clientProps, serverProps, update, props, "Connection");

            map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Connection"]);

            test(map["active"].current == 1);

            ControllerPrx controller = ControllerPrxHelper.checkedCast(
                communicator.stringToProxy("controller:" + helper.getTestEndpoint(1)));
            controller.hold();

            map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["active"].current == 1);
            map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["holding"].current == 1);

            metrics.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

            map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["closing"].current == 1);
            map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["holding"].current == 1);

            controller.resume();

            map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["holding"].current == 0);

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
            updateProps(clientProps, serverProps, update, props, "Connection");

            metrics.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

            metrics.ice_timeout(500).ice_ping();
            controller.hold();
            try
            {
                ((MetricsPrx)metrics.ice_timeout(500)).opByteS(new byte[10000000]);
                test(false);
            }
            catch (Ice.TimeoutException)
            {
            }
            controller.resume();

            cm1 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
            while (true)
            {
                sm1 = (IceMX.ConnectionMetrics)serverMetrics.getMetricsView("View", out timestamp)["Connection"][0];
                if (sm1.failures >= 2)
                {
                    break;
                }
                Thread.Sleep(10);
            }
            test(cm1.failures == 2 && sm1.failures >= 2);

            checkFailure(clientMetrics, "Connection", cm1.id, "::Ice::TimeoutException", 1, output);
            checkFailure(clientMetrics, "Connection", cm1.id, "::Ice::ConnectTimeoutException", 1, output);
            checkFailure(serverMetrics, "Connection", sm1.id, "::Ice::ConnectionLostException", 0, output);

            MetricsPrx m = (MetricsPrx)metrics.ice_timeout(500).ice_connectionId("Con1");
            m.ice_ping();

            testAttribute(clientMetrics, clientProps, update, "Connection", "parent", "Communicator", output);
            //testAttribute(clientMetrics, clientProps, update, "Connection", "id", "");
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpoint",
                          endpoint + " -t 500", output);

            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointType", type, output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsDatagram", "False", output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsSecure", isSecure, output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointTimeout", "500", output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointCompress", "False", output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointHost", host, output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointPort", port, output);

            testAttribute(clientMetrics, clientProps, update, "Connection", "incoming", "False", output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "adapterName", "", output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "connectionId", "Con1", output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "localHost", host, output);
            //testAttribute(clientMetrics, clientProps, update, "Connection", "localPort", "", output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "remoteHost", host, output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "remotePort", port, output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "mcastHost", "", output);
            testAttribute(clientMetrics, clientProps, update, "Connection", "mcastPort", "", output);

            m.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

            waitForCurrent(clientMetrics, "View", "Connection", 0);
            waitForCurrent(serverMetrics, "View", "Connection", 0);

            output.WriteLine("ok");

            output.Write("testing connection establishment metrics... ");
            output.Flush();

            props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
            updateProps(clientProps, serverProps, update, props, "ConnectionEstablishment");
            test(clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"].Length == 0);

            metrics.ice_ping();

            test(clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"].Length == 1);
            IceMX.Metrics m1 = clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"][0];
            test(m1.current == 0 && m1.total == 1 && m1.id.Equals(hostAndPort));

            metrics.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            controller.hold();
            try
            {
                communicator.stringToProxy("test:tcp -h 127.0.0.1 -p " + port).ice_timeout(10).ice_ping();
                test(false);
            }
            catch (Ice.ConnectTimeoutException)
            {
            }
            catch (Ice.LocalException)
            {
                test(false);
            }
            controller.resume();
            test(clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"].Length == 1);
            m1 = clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"][0];
            test(m1.id.Equals(hostAndPort) && m1.total == 3 && m1.failures == 2);

            checkFailure(clientMetrics, "ConnectionEstablishment", m1.id, "::Ice::ConnectTimeoutException", 2, output);

            System.Action c = () => { connect(metrics); };
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "parent", "Communicator", c, output);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "id", hostAndPort, c, output);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpoint",
                          endpoint + " -t " + timeout, c, output);

            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointType", type, c, output);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsDatagram", "False",
                          c, output);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsSecure", isSecure,
                          c, output);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointTimeout", timeout, c, output);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointCompress", "False",
                          c, output);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointHost", host, c, output);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointPort", port, c, output);

            output.WriteLine("ok");

            output.Write("testing endpoint lookup metrics... ");
            output.Flush();

            props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
            updateProps(clientProps, serverProps, update, props, "EndpointLookup");
            test(clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"].Length == 0);

            Ice.ObjectPrx prx =
                communicator.stringToProxy("metrics:" + protocol + " -p " + port + " -h localhost -t 500");
            try
            {
                prx.ice_ping();
                prx.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            }
            catch (Ice.LocalException)
            {
            }

            test(clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"].Length == 1);
            m1 = clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"][0];
            test(m1.current <= 1 && m1.total == 1);

            bool dnsException = false;
            try
            {
                communicator.stringToProxy("test:tcp -t 500 -h unknownfoo.zeroc.com -p " + port).ice_ping();
                test(false);
            }
            catch (Ice.DNSException)
            {
                dnsException = true;
            }
            catch (Ice.LocalException)
            {
                // Some DNS servers don't fail on unknown DNS names.
            }
            test(clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"].Length == 2);
            m1 = clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"][0];
            if (!m1.id.Equals("tcp -h unknownfoo.zeroc.com -p " + port + " -t 500"))
            {
                m1 = clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"][1];
            }
            test(m1.id.Equals("tcp -h unknownfoo.zeroc.com -p " + port + " -t 500") && m1.total == 2 &&
                 (!dnsException || m1.failures == 2));
            if (dnsException)
            {
                checkFailure(clientMetrics, "EndpointLookup", m1.id, "::Ice::DNSException", 2, output);
            }

            c = () => { connect(prx); };

            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "parent", "Communicator", c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "id",
                          prx.ice_getConnection().getEndpoint().ToString(), c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpoint",
                          prx.ice_getConnection().getEndpoint().ToString(), c, output);

            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointType", type, c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsDatagram", "False", c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsSecure", isSecure, c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointTimeout", "500", c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointCompress", "False", c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointHost", "localhost", c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointPort", port, c, output);

            output.WriteLine("ok");
        }
        output.Write("testing dispatch metrics... ");
        output.Flush();

        props["IceMX.Metrics.View.Map.Dispatch.GroupBy"] = "operation";
        updateProps(clientProps, serverProps, update, props, "Dispatch");
        test(serverMetrics.getMetricsView("View", out timestamp)["Dispatch"].Length == 0);

        metrics.op();
        try
        {
            metrics.opWithUserException();
            test(false);
        }
        catch (UserEx)
        {
        }
        try
        {
            metrics.opWithRequestFailedException();
            test(false);
        }
        catch (Ice.RequestFailedException)
        {
        }
        try
        {
            metrics.opWithLocalException();
            test(false);
        }
        catch (Ice.LocalException)
        {
        }
        try
        {
            metrics.opWithUnknownException();
            test(false);
        }
        catch (Ice.UnknownException)
        {
        }
        if (!collocated)
        {
            try
            {
                metrics.fail();
                test(false);
            }
            catch (Ice.ConnectionLostException)
            {
            }
        }

        map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Dispatch"]);
        test(collocated ? map.Count == 5 : map.Count == 6);

        IceMX.DispatchMetrics dm1;
        dm1 = (IceMX.DispatchMetrics)map["op"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 0 && dm1.userException == 0);
        test(dm1.size == 21 && dm1.replySize == 7);

        dm1 = (IceMX.DispatchMetrics)map["opWithUserException"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 0 && dm1.userException == 1);
        test(dm1.size == 38 && dm1.replySize == 23);

        dm1 = (IceMX.DispatchMetrics)map["opWithLocalException"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "::Ice::SyscallException", 1, output);
        test(dm1.size == 39 && dm1.replySize > 7); // Reply contains the exception stack depending on the OS.

        dm1 = (IceMX.DispatchMetrics)map["opWithRequestFailedException"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "::Ice::ObjectNotExistException", 1, output);
        test(dm1.size == 47 && dm1.replySize == 40);

        dm1 = (IceMX.DispatchMetrics)map["opWithUnknownException"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "System.ArgumentOutOfRangeException", 1, output);
        test(dm1.size == 41 && dm1.replySize > 7); // Reply contains the exception stack depending on the OS.

        System.Action op = () => { invokeOp(metrics); };
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "parent", "TestAdapter", op, output);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "id", "metrics [op]", op, output);

        if (!collocated)
        {
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpoint",
                          endpoint + " -t 60000", op, output);
            //testAttribute(serverMetrics, serverProps, update, "Dispatch", "connection", "", op);

            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointType", type, op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsDatagram", "False", op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsSecure", isSecure, op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointTimeout", "60000", op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointCompress", "False", op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointHost", host, op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointPort", port, op, output);

            testAttribute(serverMetrics, serverProps, update, "Dispatch", "incoming", "True", op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "adapterName", "TestAdapter", op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "connectionId", "", op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "localHost", host, op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "localPort", port, op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "remoteHost", host, op, output);
            //testAttribute(serverMetrics, serverProps, update, "Dispatch", "remotePort", port, op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastHost", "", op, output);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastPort", "", op, output);
        }

        testAttribute(serverMetrics, serverProps, update, "Dispatch", "operation", "op", op, output);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "identity", "metrics", op, output);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "facet", "", op, output);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "mode", "twoway", op, output);

        testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry1", "test", op, output);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry2", "", op, output);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry3", "", op, output);

        output.WriteLine("ok");

        output.Write("testing invocation metrics... ");
        output.Flush();

        //
        // Tests for twoway
        //
        props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
        props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "id";
        props["IceMX.Metrics.View.Map.Invocation.Map.Collocated.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update, props, "Invocation");
        test(serverMetrics.getMetricsView("View", out timestamp)["Invocation"].Length == 0);

        metrics.op();
        metrics.opAsync().Wait();

        try
        {
            metrics.opWithUserException();
            test(false);
        }
        catch (UserEx)
        {
        }

        try
        {
            metrics.opWithUserExceptionAsync().Wait();
            test(false);
        }
        catch (System.AggregateException ex)
        {
            test(ex.InnerException is UserEx);
        }

        try
        {
            metrics.opWithRequestFailedException();
            test(false);
        }
        catch (Ice.RequestFailedException)
        {
        }

        try
        {
            metrics.opWithRequestFailedExceptionAsync().Wait();
            test(false);
        }
        catch (System.AggregateException ex)
        {
            test(ex.InnerException is Ice.RequestFailedException);
        }

        try
        {
            metrics.opWithLocalException();
            test(false);
        }
        catch (Ice.LocalException)
        {
        }
        try
        {
            metrics.opWithLocalExceptionAsync().Wait();
            test(false);
        }
        catch (System.AggregateException ex)
        {
            test(ex.InnerException is Ice.LocalException);
        }

        try
        {
            metrics.opWithUnknownException();
            test(false);
        }
        catch (Ice.UnknownException)
        {
        }

        try
        {
            metrics.opWithUnknownExceptionAsync().Wait();
            test(false);
        }
        catch (System.AggregateException ex)
        {
            test(ex.InnerException is Ice.UnknownException);
        }

        if (!collocated)
        {
            try
            {
                metrics.fail();
                test(false);
            }
            catch (Ice.ConnectionLostException)
            {
            }

            try
            {
                metrics.failAsync().Wait();
                test(false);
            }
            catch (System.AggregateException ex)
            {
                test(ex.InnerException is Ice.ConnectionLostException);
            }
        }

        map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
        test(map.Count == (collocated ? 5 : 6));

        IceMX.InvocationMetrics im1;
        IceMX.ChildInvocationMetrics rim1;
        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.current <= 1 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
        test(collocated ? im1.collocated.Length == 1 : im1.remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 2 && rim1.failures == 0);
        test(rim1.size == 42 && rim1.replySize == 14);

        im1 = (IceMX.InvocationMetrics)map["opWithUserException"];
        test(im1.current <= 1 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
        test(collocated ? im1.collocated.Length == 1 : im1.remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 2 && rim1.failures == 0);
        test(rim1.size == 76 && rim1.replySize == 46);
        test(im1.userException == 2);

        im1 = (IceMX.InvocationMetrics)map["opWithLocalException"];
        test(im1.current <= 1 && im1.total == 2 && im1.failures == 2 && im1.retry == 0);
        test(collocated ? im1.collocated.Length == 1 : im1.remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 2 && rim1.failures == 0);
        test(rim1.size == 78 && rim1.replySize > 7);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::UnknownLocalException", 2, output);

        im1 = (IceMX.InvocationMetrics)map["opWithRequestFailedException"];
        test(im1.current <= 1 && im1.total == 2 && im1.failures == 2 && im1.retry == 0);
        test(collocated ? im1.collocated.Length == 1 : im1.remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 2 && rim1.failures == 0);
        test(rim1.size == 94 && rim1.replySize == 80);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::ObjectNotExistException", 2, output);

        im1 = (IceMX.InvocationMetrics)map["opWithUnknownException"];
        test(im1.current <= 1 && im1.total == 2 && im1.failures == 2 && im1.retry == 0);
        test(collocated ? im1.collocated.Length == 1 : im1.remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current == 0 && rim1.total == 2 && rim1.failures == 0);
        test(rim1.size == 82 && rim1.replySize > 7);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::UnknownException", 2, output);

        if (!collocated)
        {
            im1 = (IceMX.InvocationMetrics)map["fail"];
            test(im1.current <= 1 && im1.total == 2 && im1.failures == 2 && im1.retry == 2 && im1.remotes.Length == 1);
            rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
            test(rim1.current == 0);
            test(rim1.total == 4);
            test(rim1.failures == 4);
            checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::ConnectionLostException", 2, output);
        }

        testAttribute(clientMetrics, clientProps, update, "Invocation", "parent", "Communicator", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "id", "metrics -t -e 1.1 [op]", op, output);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "operation", "op", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "identity", "metrics", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "facet", "", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "encoding", "1.1", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "twoway", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "proxy",
                      "metrics -t -e 1.1:" + endpoint + " -t " + timeout, op, output);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry1", "test", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry2", "", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry3", "", op, output);

        //
        // Oneway tests
        //
        clearView(clientProps, serverProps, update);
        props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
        props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
        updateProps(clientProps, serverProps, update, props, "Invocation");

        MetricsPrx metricsOneway = (MetricsPrx)metrics.ice_oneway();
        metricsOneway.op();
        metricsOneway.opAsync().Wait();

        map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
        test(map.Count == 1);

        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.current <= 1 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
        test(collocated ? (im1.collocated.Length == 1) : (im1.remotes.Length == 1));
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.collocated[0] : im1.remotes[0]);
        test(rim1.current <= 1 && rim1.total == 2 && rim1.failures == 0);
        test(rim1.size == 42 && rim1.replySize == 0);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "oneway",
                      () => { invokeOp(metricsOneway); }, output);

        //
        // Batch oneway tests
        //
        props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
        props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
        updateProps(clientProps, serverProps, update, props, "Invocation");

        MetricsPrx metricsBatchOneway = (MetricsPrx)metrics.ice_batchOneway();
        metricsBatchOneway.op();
        metricsBatchOneway.opAsync().Wait();

        map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
        test(map.Count == 1);

        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.current == 0 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
        test(im1.remotes.Length == 0);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "batch-oneway",
                      () => { invokeOp(metricsBatchOneway); }, output);

        //
        // Tests flushBatchRequests
        //
        props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
        props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
        updateProps(clientProps, serverProps, update, props, "Invocation");

        metricsBatchOneway = (MetricsPrx)metrics.ice_batchOneway();
        metricsBatchOneway.op();

        metricsBatchOneway.ice_flushBatchRequests();
        metricsBatchOneway.ice_flushBatchRequestsAsync().Wait();

        map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
        test(map.Count == 2);

        im1 = (IceMX.InvocationMetrics)map["ice_flushBatchRequests"];
        test(im1.current <= 1 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
        if (!collocated)
        {
            test(im1.remotes.Length == 1); // The first operation got sent over a connection
        }

        if (!collocated)
        {
            clearView(clientProps, serverProps, update);

            Ice.Connection con = metricsBatchOneway.ice_getConnection();

            metricsBatchOneway = (MetricsPrx)metricsBatchOneway.ice_fixed(con);
            metricsBatchOneway.op();

            con.flushBatchRequests(Ice.CompressBatch.No);
            con.flushBatchRequestsAsync(Ice.CompressBatch.No).Wait();

            map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
            test(map.Count == 3);

            im1 = (IceMX.InvocationMetrics)map["flushBatchRequests"];
            test(im1.current == 0 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
            test(im1.remotes.Length == 1); // The first operation got sent over a connection

            clearView(clientProps, serverProps, update);
            metricsBatchOneway.op();

            communicator.flushBatchRequests(Ice.CompressBatch.No);
            communicator.flushBatchRequestsAsync(Ice.CompressBatch.No).Wait();
            map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
            test(map.Count == 2);

            im1 = (IceMX.InvocationMetrics)map["flushBatchRequests"];
            test(im1.current <= 1 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
            test(im1.remotes.Length == 1); // The first operation got sent over a connection
        }
        output.WriteLine("ok");

        output.Write("testing metrics view enable/disable...");
        output.Flush();

        string[] disabledViews;
        props["IceMX.Metrics.View.GroupBy"] = "none";
        props["IceMX.Metrics.View.Disabled"] = "0";
        updateProps(clientProps, serverProps, update, props, "Thread");
        test(clientMetrics.getMetricsView("View", out timestamp)["Thread"].Length != 0);
        test(clientMetrics.getMetricsViewNames(out disabledViews).Length == 1 && disabledViews.Length == 0);

        props["IceMX.Metrics.View.Disabled"] = "1";
        updateProps(clientProps, serverProps, update, props, "Thread");
        test(!clientMetrics.getMetricsView("View", out timestamp).ContainsKey("Thread"));
        test(clientMetrics.getMetricsViewNames(out disabledViews).Length == 0 && disabledViews.Length == 1);

        clientMetrics.enableMetricsView("View");
        test(clientMetrics.getMetricsView("View", out timestamp)["Thread"].Length != 0);
        test(clientMetrics.getMetricsViewNames(out disabledViews).Length == 1 && disabledViews.Length == 0);

        clientMetrics.disableMetricsView("View");
        test(!clientMetrics.getMetricsView("View", out timestamp).ContainsKey("Thread"));
        test(clientMetrics.getMetricsViewNames(out disabledViews).Length == 0 && disabledViews.Length == 1);

        try
        {
            clientMetrics.enableMetricsView("UnknownView");
        }
        catch (IceMX.UnknownMetricsView)
        {
        }

        output.WriteLine("ok");

        output.Write("testing instrumentation observer delegate... ");
        output.Flush();

        test(obsv.threadObserver.getTotal() > 0);
        if (!collocated)
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
        if (!collocated)
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
        if (!collocated)
        {
            test(obsv.connectionObserver.getFailedCount() > 0);
            test(obsv.connectionEstablishmentObserver.getFailedCount() > 0);
            test(obsv.endpointLookupObserver.getFailedCount() > 0);
            test(obsv.invocationObserver.remoteObserver.getFailedCount() > 0);
        }
        //test(obsv.dispatchObserver.getFailedCount() > 0);
        test(obsv.invocationObserver.getFailedCount() > 0);

        if (!collocated)
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

        output.WriteLine("ok");
        return metrics;
    }
}
