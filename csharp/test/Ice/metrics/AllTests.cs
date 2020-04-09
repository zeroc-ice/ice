//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.IO;
using Ice;

using Test;

public class AllTests : Test.AllTests
{
    static IceMX.ConnectionMetrics
    getServerConnectionMetrics(IceMX.IMetricsAdminPrx metrics, long expected)
    {
        try
        {
            IceMX.ConnectionMetrics s;
            s = (IceMX.ConnectionMetrics)metrics.GetMetricsView("View").ReturnValue["Connection"][0];
            int nRetry = 30;
            while (s.SentBytes != expected && nRetry-- > 0)
            {
                // On some platforms, it's necessary to wait a little before obtaining the server metrics
                // to get an accurate sentBytes metric. The sentBytes metric is updated before the response
                // to the operation is sent and getMetricsView can be dispatched before the metric is really
                // updated.
                Thread.Sleep(100);
                s = (IceMX.ConnectionMetrics)metrics.GetMetricsView("View").ReturnValue["Connection"][0];
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

        public void exception(System.Exception ex)
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
    getPort(IPropertiesAdminPrx p)
    {
        return TestHelper.getTestPort(p.Communicator.GetProperties(), 0).ToString();
    }

    static private Dictionary<string, string>
    getClientProps(Ice.IPropertiesAdminPrx p, Dictionary<string, string> orig, string m)
    {
        Dictionary<string, string> props = p.GetPropertiesForPrefix("IceMX.Metrics");
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
    getServerProps(Ice.IPropertiesAdminPrx p, Dictionary<string, string> orig, string m)
    {
        Dictionary<string, string> props = p.GetPropertiesForPrefix("IceMX.Metrics");
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
        public UpdateCallbackI(Ice.IPropertiesAdminPrx serverProps)
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
            _serverProps.SetProperties(new Dictionary<string, string>());

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
        private Ice.IPropertiesAdminPrx _serverProps;
    };

    static void
    waitForCurrent(IceMX.IMetricsAdminPrx metrics, string viewName, string map, int value)
    {
        while (true)
        {
            Dictionary<string, IceMX.Metrics[]> view = metrics.GetMetricsView(viewName).ReturnValue;
            test(view.ContainsKey(map));
            bool ok = true;
            foreach (IceMX.Metrics m in view[map])
            {
                if (m.Current != value)
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
    waitForObserverCurrent(Observer observer, int value)
    {
        for (int i = 0; i < 10; ++i)
        {
            if (observer.GetCurrent() > 0)
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
    testAttribute(IceMX.IMetricsAdminPrx metrics,
                  Ice.IPropertiesAdminPrx props,
                  UpdateCallbackI update,
                  string map,
                  string attr,
                  string value,
                  System.Action func,
                  TextWriter output)
    {
        Dictionary<string, string> dict = new Dictionary<string, string>();
        dict.Add("IceMX.Metrics.View.Map." + map + ".GroupBy", attr);
        if (props.Identity.Category.Equals("client"))
        {
            props.SetProperties(getClientProps(props, dict, map));
            update.waitForUpdate();
        }
        else
        {
            props.SetProperties(getServerProps(props, dict, map));
            props.SetProperties(new Dictionary<string, string>());
        }

        func();
        Dictionary<string, IceMX.Metrics[]> view = metrics.GetMetricsView("View").ReturnValue;
        if (!view.ContainsKey(map) || view[map].Length == 0)
        {
            if (value.Length > 0)
            {
                output.WriteLine("no map `" + map + "' for group by = `" + attr + "'");
                test(false);
            }
        }
        else if (!view[map][0].Id.Equals(value))
        {
            output.WriteLine("invalid attribute value: " + attr + " = " + value + " got " + view[map][0].Id);
            test(false);
        }

        dict.Clear();
        if (props.Identity.Category.Equals("client"))
        {
            props.SetProperties(getClientProps(props, dict, map));
            update.waitForUpdate();
        }
        else
        {
            props.SetProperties(getServerProps(props, dict, map));
            props.SetProperties(new Dictionary<string, string>());
        }
    }

    static void connect(Ice.IObjectPrx proxy)
    {
        var conn = proxy.GetCachedConnection();
        if (conn != null)
        {
            conn.Close(Ice.ConnectionClose.GracefullyWithWait);
        }

        try
        {
            proxy.IcePing();
        }
        catch (System.Exception)
        {
        }

        conn = proxy.GetCachedConnection();
        if (conn != null)
        {
            conn.Close(Ice.ConnectionClose.GracefullyWithWait);
        }
    }

    static void invokeOp(IMetricsPrx proxy)
    {
        Dictionary<string, string> ctx = new Dictionary<string, string>();
        ctx.Add("entry1", "test");
        ctx.Add("entry2", "");
        proxy.op(ctx);
    }

    static void
    testAttribute(IceMX.IMetricsAdminPrx metrics,
                  Ice.IPropertiesAdminPrx props,
                  UpdateCallbackI update,
                  string map,
                  string attr,
                  string value,
                  TextWriter output)
    {
        testAttribute(metrics, props, update, map, attr, value, () => { }, output);
    }

    static void
    updateProps(Ice.IPropertiesAdminPrx cprops,
                Ice.IPropertiesAdminPrx sprops,
                UpdateCallbackI callback,
                Dictionary<string, string> props,
                string map)
    {
        if (sprops.GetConnection() != null)
        {
            cprops.SetProperties(getClientProps(cprops, props, map));
            sprops.SetProperties(getServerProps(sprops, props, map));
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
            cprops.SetProperties(serverProps);
        }
        callback.waitForUpdate();
    }

    static void
    clearView(Ice.IPropertiesAdminPrx cprops, Ice.IPropertiesAdminPrx sprops, UpdateCallbackI callback)
    {
        Dictionary<string, string> dict;

        dict = cprops.GetPropertiesForPrefix("IceMX.Metrics");
        dict["IceMX.Metrics.View.Disabled"] = "1";
        cprops.SetProperties(dict);

        dict = sprops.GetPropertiesForPrefix("IceMX.Metrics");
        dict["IceMX.Metrics.View.Disabled"] = "1";
        sprops.SetProperties(dict);

        callback.waitForUpdate();

        dict = cprops.GetPropertiesForPrefix("IceMX.Metrics");
        dict["IceMX.Metrics.View.Disabled"] = "";
        cprops.SetProperties(dict);

        dict = sprops.GetPropertiesForPrefix("IceMX.Metrics");
        dict["IceMX.Metrics.View.Disabled"] = "";
        sprops.SetProperties(dict);

        callback.waitForUpdate();
    }

    static void
    checkFailure(IceMX.IMetricsAdminPrx m, string map, string id, string failure, int count, TextWriter output)
    {
        IceMX.MetricsFailures f = m.GetMetricsFailures("View", map, id);
        if (!f.Failures.ContainsKey(failure))
        {
            output.WriteLine("couldn't find failure `" + failure + "' for `" + id + "'");
            test(false);
        }
        if (count > 0 && f.Failures[failure] != count)
        {
            output.Write("count for failure `" + failure + "' of `" + id + "' is different from expected: ");
            output.WriteLine(count + " != " + f.Failures[failure]);
            test(false);
        }
    }

    static Dictionary<string, IceMX.Metrics>
    toMap(IceMX.Metrics[] mmap)
    {
        Dictionary<string, IceMX.Metrics> m = new Dictionary<string, IceMX.Metrics>();
        foreach (IceMX.Metrics e in mmap)
        {
            m.Add(e.Id, e);
        }
        return m;
    }

    public static IMetricsPrx allTests(TestHelper helper, CommunicatorObserver obsv)
    {
        Communicator communicator = helper.communicator();

        string host = helper.getTestHost();
        string port = helper.getTestPort(0).ToString();
        string hostAndPort = host + ":" + port;
        string transport = helper.getTestTransport();
        string endpoint = transport + " -h " + host + " -p " + port;
        string timeout = communicator.GetProperty("Ice.Default.Timeout") ?? "60000";

        IMetricsPrx metrics = IMetricsPrx.Parse($"metrics:{endpoint}", communicator);
        bool collocated = metrics.GetConnection() == null;
        var output = helper.getWriter();
        output.Write("testing metrics admin facet checkedCast... ");
        output.Flush();
        IObjectPrx admin = communicator.GetAdmin();
        IPropertiesAdminPrx clientProps =
            IPropertiesAdminPrx.CheckedCast(admin.Clone(facet: "Properties", IObjectPrx.Factory));
        IceMX.IMetricsAdminPrx clientMetrics =
            IceMX.IMetricsAdminPrx.CheckedCast(admin.Clone(facet: "Metrics", IObjectPrx.Factory));
        test(clientProps != null && clientMetrics != null);

        admin = metrics.getAdmin();
        IPropertiesAdminPrx serverProps =
            IPropertiesAdminPrx.CheckedCast(admin.Clone(facet: "Properties", IObjectPrx.Factory));
        IceMX.IMetricsAdminPrx serverMetrics =
            IceMX.IMetricsAdminPrx.CheckedCast(admin.Clone(facet: "Metrics", IObjectPrx.Factory));
        test(serverProps != null && serverMetrics != null);

        UpdateCallbackI update = new UpdateCallbackI(serverProps);
        ((INativePropertiesAdmin)communicator.FindAdminFacet("Properties")).AddUpdateCallback(update.updated);

        output.WriteLine("ok");

        Dictionary<string, string> props = new Dictionary<string, string>();

        output.Write("testing group by none...");
        output.Flush();

        props.Add("IceMX.Metrics.View.GroupBy", "none");
        updateProps(clientProps, serverProps, update, props, "");
        Dictionary<string, IceMX.Metrics[]> view = clientMetrics.GetMetricsView("View").ReturnValue;
        if (!collocated)
        {
            test(view["Connection"].Length == 1 && view["Connection"][0].Current == 1 &&
                 view["Connection"][0].Total == 1);
        }
        test(view["Thread"].Length == 1 && view["Thread"][0].Current == 5 && view["Thread"][0].Total == 5);
        output.WriteLine("ok");

        output.Write("testing group by id...");
        output.Flush();

        props["IceMX.Metrics.View.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update, props, "");

        metrics.IcePing();
        metrics.IcePing();
        metrics.Clone(connectionId: "Con1").IcePing();
        metrics.Clone(connectionId: "Con1").IcePing();
        metrics.Clone(connectionId: "Con1").IcePing();

        waitForCurrent(clientMetrics, "View", "Invocation", 0);
        waitForCurrent(serverMetrics, "View", "Dispatch", 0);

        view = clientMetrics.GetMetricsView("View").ReturnValue;
        test(view["Thread"].Length == 5);
        if (!collocated)
        {
            test(view["Connection"].Length == 2);
        }
        test(view["Invocation"].Length == 1);

        IceMX.InvocationMetrics invoke = (IceMX.InvocationMetrics)view["Invocation"][0];

        test(invoke.Id.IndexOf("[ice_ping]") > 0 && invoke.Current == 0 && invoke.Total == 5);
        if (!collocated)
        {
            test(invoke.Remotes.Length == 2);
            test(invoke.Remotes[0].Total >= 2 && invoke.Remotes[1].Total >= 2);
            test((invoke.Remotes[0].Total + invoke.Remotes[1].Total) == 5);
        }
        else
        {
            test(invoke.Collocated.Length == 1);
            test(invoke.Collocated[0].Total == 5);
        }

        view = serverMetrics.GetMetricsView("View").ReturnValue;
        // With Ice for .NET, a new dispatching thread isn't necessarily created.
        //test(view["Thread"].Length > 5);
        if (!collocated)
        {
            test(view["Connection"].Length == 2);
        }
        test(view["Dispatch"].Length == 1);
        test(view["Dispatch"][0].Current == 0 && view["Dispatch"][0].Total == 5);
        test(view["Dispatch"][0].Id.IndexOf("[ice_ping]") > 0);

        if (!collocated)
        {
            metrics.GetConnection().Close(ConnectionClose.GracefullyWithWait);
            metrics.Clone(connectionId: "Con1").GetConnection().Close(ConnectionClose.GracefullyWithWait);

            waitForCurrent(clientMetrics, "View", "Connection", 0);
            waitForCurrent(serverMetrics, "View", "Connection", 0);
        }

        clearView(clientProps, serverProps, update);

        output.WriteLine("ok");

        string type = "";
        string isSecure = "";
        if (!collocated)
        {
            Ice.Endpoint connectionEndpoint = metrics.GetConnection().Endpoint;
            type = connectionEndpoint.Type.ToString();
            isSecure = connectionEndpoint.IsSecure ? "True" : "False";
        }

        Dictionary<string, IceMX.Metrics> map;

        if (!collocated)
        {
            output.Write("testing connection metrics... ");
            output.Flush();

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
            updateProps(clientProps, serverProps, update, props, "Connection");

            test(clientMetrics.GetMetricsView("View").ReturnValue["Connection"].Length == 0);
            test(serverMetrics.GetMetricsView("View").ReturnValue["Connection"].Length == 0);

            metrics.IcePing();

            IceMX.ConnectionMetrics cm1, sm1, cm2, sm2;
            cm1 = (IceMX.ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0];
            sm1 = getServerConnectionMetrics(serverMetrics, 25);

            metrics.IcePing();

            cm2 = (IceMX.ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0];
            sm2 = getServerConnectionMetrics(serverMetrics, 50);

            test(cm2.SentBytes - cm1.SentBytes == 45); // 45 for IcePing request
            test(cm2.ReceivedBytes - cm1.ReceivedBytes == 25); // 25 bytes for IcePing response
            test(sm2.ReceivedBytes - sm1.ReceivedBytes == 45);
            test(sm2.SentBytes - sm1.SentBytes == 25);

            cm1 = cm2;
            sm1 = sm2;

            byte[] bs = new byte[0];
            metrics.opByteS(bs);

            cm2 = (IceMX.ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.SentBytes + cm2.ReceivedBytes - cm1.ReceivedBytes);
            long requestSz = cm2.SentBytes - cm1.SentBytes;
            long replySz = cm2.ReceivedBytes - cm1.ReceivedBytes;

            cm1 = cm2;
            sm1 = sm2;

            bs = new byte[456];
            metrics.opByteS(bs);

            cm2 = (IceMX.ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.SentBytes + replySz);

            test(cm2.SentBytes - cm1.SentBytes == requestSz + bs.Length + 4); // 4 is for the seq variable size
            test(cm2.ReceivedBytes - cm1.ReceivedBytes == replySz);
            test(sm2.ReceivedBytes - sm1.ReceivedBytes == requestSz + bs.Length + 4);
            test(sm2.SentBytes - sm1.SentBytes == replySz);

            cm1 = cm2;
            sm1 = sm2;

            bs = new byte[1024 * 1024 * 10]; // Try with large amount of data which should be sent in several chunks
            metrics.opByteS(bs);

            cm2 = (IceMX.ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0];
            sm2 = getServerConnectionMetrics(serverMetrics, sm1.SentBytes + replySz);

            test((cm2.SentBytes - cm1.SentBytes) == (requestSz + bs.Length + 4)); // 4 is for the seq variable size
            test((cm2.ReceivedBytes - cm1.ReceivedBytes) == replySz);
            test((sm2.ReceivedBytes - sm1.ReceivedBytes) == (requestSz + bs.Length + 4));
            test((sm2.SentBytes - sm1.SentBytes) == replySz);

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "state";
            updateProps(clientProps, serverProps, update, props, "Connection");

            map = toMap(serverMetrics.GetMetricsView("View").ReturnValue["Connection"]);

            test(map["active"].Current == 1);

            IControllerPrx controller = IControllerPrx.Parse($"controller:{helper.getTestEndpoint(1)}", communicator);

            metrics.GetConnection().Close(Ice.ConnectionClose.GracefullyWithWait);

            map = toMap(clientMetrics.GetMetricsView("View").ReturnValue["Connection"]);
            // The connection might already be closed so it can be 0 or 1
            test(map["closing"].Current == 0 || map["closing"].Current == 1);

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
            updateProps(clientProps, serverProps, update, props, "Connection");

            metrics.GetConnection().Close(Ice.ConnectionClose.GracefullyWithWait);

            // TODO: remove or refactor depending on what we decide for connection timeouts
            // metrics.Clone(connectionTimeout: 500).IcePing();
            // controller.hold();
            // try
            // {
            //     metrics.Clone(connectionTimeout: 500).opByteS(new byte[10000000]);
            //     test(false);
            // }
            // catch (Ice.ConnectTimeoutException)
            // {
            // }
            // controller.resume();

            // cm1 = (IceMX.ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0];
            // while (true)
            // {
            //     sm1 = (IceMX.ConnectionMetrics)serverMetrics.GetMetricsView("View").ReturnValue["Connection"][0];
            //     if (sm1.Failures >= 2)
            //     {
            //         break;
            //     }
            //     Thread.Sleep(10);
            // }
            // test(cm1.Failures == 2 && sm1.Failures >= 2);

            // checkFailure(clientMetrics, "Connection", cm1.Id, "Ice.ConnectionTimeoutException", 1, output);
            // checkFailure(clientMetrics, "Connection", cm1.Id, "Ice.ConnectTimeoutException", 1, output);
            // checkFailure(serverMetrics, "Connection", sm1.Id, "Ice.ConnectionLostException", 0, output);

            IMetricsPrx m = metrics.Clone(connectionTimeout: 500, connectionId: "Con1");
            m.IcePing();

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

            m.GetConnection().Close(Ice.ConnectionClose.GracefullyWithWait);

            waitForCurrent(clientMetrics, "View", "Connection", 0);
            waitForCurrent(serverMetrics, "View", "Connection", 0);

            output.WriteLine("ok");

            output.Write("testing connection establishment metrics... ");
            output.Flush();

            props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
            updateProps(clientProps, serverProps, update, props, "ConnectionEstablishment");
            test(clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"].Length == 0);

            metrics.IcePing();

            test(clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"].Length == 1);
            IceMX.Metrics m1 = clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"][0];
            test(m1.Current == 0 && m1.Total == 1 && m1.Id.Equals(hostAndPort));

            metrics.GetConnection().Close(Ice.ConnectionClose.GracefullyWithWait);
            controller.hold();
            try
            {
                IObjectPrx.Parse($"test:tcp -h 127.0.0.1 -p {port}", communicator).Clone(connectionTimeout: 10).IcePing();
                test(false);
            }
            catch (Ice.ConnectTimeoutException)
            {
            }
            catch (System.Exception)
            {
                test(false);
            }
            controller.resume();
            test(clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"].Length == 1);
            m1 = clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"][0];
            test(m1.Id.Equals(hostAndPort) && m1.Total == 3 && m1.Failures == 2);

            checkFailure(clientMetrics, "ConnectionEstablishment", m1.Id, "Ice.ConnectTimeoutException", 2, output);

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
            test(clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"].Length == 0);

            var prx = IObjectPrx.Parse($"metrics:{transport} -p {port} -h localhost -t 500", communicator);
            try
            {
                prx.IcePing();
                prx.GetConnection().Close(ConnectionClose.GracefullyWithWait);
            }
            catch (System.Exception)
            {
            }

            test(clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"].Length == 1);
            m1 = clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"][0];
            test(m1.Current <= 1 && m1.Total == 1);

            bool dnsException = false;
            try
            {
                IObjectPrx.Parse($"test:tcp -t 500 -h unknownfoo.zeroc.com -p {port}", communicator).IcePing();
                test(false);
            }
            catch (DNSException)
            {
                dnsException = true;
            }
            catch (System.Exception)
            {
                // Some DNS servers don't fail on unknown DNS names.
            }
            test(clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"].Length == 2);
            m1 = clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"][0];
            if (!m1.Id.Equals("tcp -h unknownfoo.zeroc.com -p " + port + " -t 500"))
            {
                m1 = clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"][1];
            }
            test(m1.Id.Equals("tcp -h unknownfoo.zeroc.com -p " + port + " -t 500") && m1.Total == 2 &&
                 (!dnsException || m1.Failures == 2));
            if (dnsException)
            {
                checkFailure(clientMetrics, "EndpointLookup", m1.Id, "Ice.DNSException", 2, output);
            }

            c = () => { connect(prx); };

            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "parent", "Communicator", c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "id",
                          prx.GetConnection().Endpoint.ToString(), c, output);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpoint",
                          prx.GetConnection().Endpoint.ToString(), c, output);

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
        test(serverMetrics.GetMetricsView("View").ReturnValue["Dispatch"].Length == 0);

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
        catch (DispatchException)
        {
        }
        try
        {
            metrics.opWithLocalException();
            test(false);
        }
        catch (Ice.UnhandledException)
        {
        }
        try
        {
            metrics.opWithUnknownException();
            test(false);
        }
        catch (Ice.UnhandledException)
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

        map = toMap(serverMetrics.GetMetricsView("View").ReturnValue["Dispatch"]);
        test(collocated ? map.Count == 5 : map.Count == 6);

        IceMX.DispatchMetrics dm1;
        dm1 = (IceMX.DispatchMetrics)map["op"];
        test(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 0 && dm1.UserException == 0);
        test(dm1.Size == 21 && dm1.ReplySize == 7);

        dm1 = (IceMX.DispatchMetrics)map["opWithUserException"];
        test(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 0 && dm1.UserException == 1);
        test(dm1.Size == 38 && dm1.ReplySize == 27);

        dm1 = (IceMX.DispatchMetrics)map["opWithLocalException"];
        test(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 1 && dm1.UserException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.Id, "Ice.InvalidConfigurationException", 1, output);
        test(dm1.Size == 39 && dm1.ReplySize > 7); // Reply contains the exception stack depending on the OS.

        dm1 = (IceMX.DispatchMetrics)map["opWithRequestFailedException"];
        test(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 0 && dm1.UserException == 1);
        test(dm1.Size == 47 && dm1.ReplySize == 40);

        dm1 = (IceMX.DispatchMetrics)map["opWithUnknownException"];
        test(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 1 && dm1.UserException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.Id, "System.ArgumentOutOfRangeException", 1, output);
        test(dm1.Size == 41 && dm1.ReplySize > 7); // Reply contains the exception stack depending on the OS.

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
        test(serverMetrics.GetMetricsView("View").ReturnValue["Invocation"].Length == 0);

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
        catch (DispatchException)
        {
        }

        try
        {
            metrics.opWithRequestFailedExceptionAsync().Wait();
            test(false);
        }
        catch (System.AggregateException ex)
        {
            test(ex.InnerException is DispatchException);
        }

        try
        {
            metrics.opWithLocalException();
            test(false);
        }
        catch (Ice.UnhandledException)
        {
        }
        try
        {
            metrics.opWithLocalExceptionAsync().Wait();
            test(false);
        }
        catch (System.AggregateException ex)
        {
            test(ex.InnerException is Ice.UnhandledException);
        }

        try
        {
            metrics.opWithUnknownException();
            test(false);
        }
        catch (Ice.UnhandledException)
        {
        }

        try
        {
            metrics.opWithUnknownExceptionAsync().Wait();
            test(false);
        }
        catch (System.AggregateException ex)
        {
            test(ex.InnerException is Ice.UnhandledException);
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

        map = toMap(clientMetrics.GetMetricsView("View").ReturnValue["Invocation"]);
        test(map.Count == (collocated ? 5 : 6));

        IceMX.InvocationMetrics im1;
        IceMX.ChildInvocationMetrics rim1;
        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 0 && im1.Retry == 0);
        test(collocated ? im1.Collocated.Length == 1 : im1.Remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.Collocated[0] : im1.Remotes[0]);
        test(rim1.Current == 0 && rim1.Total == 2 && rim1.Failures == 0);
        test(rim1.Size == 42 && rim1.ReplySize == 14);

        im1 = (IceMX.InvocationMetrics)map["opWithUserException"];
        test(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 0 && im1.Retry == 0);
        test(collocated ? im1.Collocated.Length == 1 : im1.Remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.Collocated[0] : im1.Remotes[0]);
        test(rim1.Current == 0 && rim1.Total == 2 && rim1.Failures == 0);
        test(rim1.Size == 76 && rim1.ReplySize == 54);
        test(im1.UserException == 2);

        im1 = (IceMX.InvocationMetrics)map["opWithLocalException"];
        test(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 2 && im1.Retry == 0);
        test(collocated ? im1.Collocated.Length == 1 : im1.Remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.Collocated[0] : im1.Remotes[0]);
        test(rim1.Current == 0 && rim1.Total == 2 && rim1.Failures == 0);
        test(rim1.Size == 78 && rim1.ReplySize > 7);
        checkFailure(clientMetrics, "Invocation", im1.Id, "Ice.UnhandledException", 2, output);

        im1 = (IceMX.InvocationMetrics)map["opWithRequestFailedException"];
        test(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 2 && im1.Retry == 0);
        test(collocated ? im1.Collocated.Length == 1 : im1.Remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.Collocated[0] : im1.Remotes[0]);
        test(rim1.Current == 0 && rim1.Total == 2 && rim1.Failures == 0);
        test(rim1.Size == 94 && rim1.ReplySize == 80);
        checkFailure(clientMetrics, "Invocation", im1.Id, "Ice.ObjectNotExistException", 2, output);

        im1 = (IceMX.InvocationMetrics)map["opWithUnknownException"];
        test(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 2 && im1.Retry == 0);
        test(collocated ? im1.Collocated.Length == 1 : im1.Remotes.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.Collocated[0] : im1.Remotes[0]);
        test(rim1.Current == 0 && rim1.Total == 2 && rim1.Failures == 0);
        test(rim1.Size == 82 && rim1.ReplySize > 7);
        checkFailure(clientMetrics, "Invocation", im1.Id, "Ice.UnhandledException", 2, output);

        if (!collocated)
        {
            im1 = (IceMX.InvocationMetrics)map["fail"];
            test(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 2 && im1.Retry == 2 && im1.Remotes.Length == 1);
            rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.Collocated[0] : im1.Remotes[0]);
            test(rim1.Current == 0);
            test(rim1.Total == 4);
            test(rim1.Failures == 4);
            checkFailure(clientMetrics, "Invocation", im1.Id, "Ice.ConnectionLostException", 2, output);
        }

        testAttribute(clientMetrics, clientProps, update, "Invocation", "parent", "Communicator", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "id", "metrics -t -p ice1 -e 1.1 [op]", op,
            output);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "operation", "op", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "identity", "metrics", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "facet", "", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "encoding", "1.1", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "twoway", op, output);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "proxy",
                      "metrics -t -p ice1 -e 1.1:" + endpoint + " -t " + timeout, op, output);

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

        IMetricsPrx metricsOneway = metrics.Clone(oneway: true);
        metricsOneway.op();
        metricsOneway.opAsync().Wait();

        map = toMap(clientMetrics.GetMetricsView("View").ReturnValue["Invocation"]);
        test(map.Count == 1);

        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 0 && im1.Retry == 0);
        test(collocated ? (im1.Collocated.Length == 1) : (im1.Remotes.Length == 1));
        rim1 = (IceMX.ChildInvocationMetrics)(collocated ? im1.Collocated[0] : im1.Remotes[0]);
        test(rim1.Current <= 1 && rim1.Total == 2 && rim1.Failures == 0);
        test(rim1.Size == 42 && rim1.ReplySize == 0);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "oneway",
                      () => invokeOp(metricsOneway), output);

        output.Write("testing metrics view enable/disable...");
        output.Flush();

        props["IceMX.Metrics.View.GroupBy"] = "none";
        props["IceMX.Metrics.View.Disabled"] = "0";
        updateProps(clientProps, serverProps, update, props, "Thread");
        test(clientMetrics.GetMetricsView("View").ReturnValue["Thread"].Length != 0);
        var (names, disabledViews) = clientMetrics.GetMetricsViewNames();
        test(names.Length == 1 && disabledViews.Length == 0);

        props["IceMX.Metrics.View.Disabled"] = "1";
        updateProps(clientProps, serverProps, update, props, "Thread");
        test(!clientMetrics.GetMetricsView("View").ReturnValue.ContainsKey("Thread"));
        (names, disabledViews) = clientMetrics.GetMetricsViewNames();
        test(names.Length == 0 && disabledViews.Length == 1);

        clientMetrics.EnableMetricsView("View");
        test(clientMetrics.GetMetricsView("View").ReturnValue["Thread"].Length != 0);
        (names, disabledViews) = clientMetrics.GetMetricsViewNames();
        test(names.Length == 1 && disabledViews.Length == 0);

        clientMetrics.DisableMetricsView("View");
        test(!clientMetrics.GetMetricsView("View").ReturnValue.ContainsKey("Thread"));
        (names, disabledViews) = clientMetrics.GetMetricsViewNames();
        test(names.Length == 0 && disabledViews.Length == 1);

        try
        {
            clientMetrics.EnableMetricsView("UnknownView");
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

        test(obsv.threadObserver.GetCurrent() > 0);
        if (!collocated)
        {
            test(obsv.connectionObserver.GetCurrent() > 0);
            test(obsv.connectionEstablishmentObserver.GetCurrent() == 0);
            test(obsv.endpointLookupObserver.GetCurrent() == 0);
            waitForObserverCurrent(obsv.invocationObserver.remoteObserver, 0);
            test(obsv.invocationObserver.remoteObserver.GetCurrent() == 0);
        }
        else
        {
            waitForObserverCurrent(obsv.invocationObserver.collocatedObserver, 0);
            test(obsv.invocationObserver.collocatedObserver.GetCurrent() == 0);
        }
        waitForObserverCurrent(obsv.dispatchObserver, 0);
        test(obsv.dispatchObserver.GetCurrent() == 0);
        waitForObserverCurrent(obsv.invocationObserver, 0);
        test(obsv.invocationObserver.GetCurrent() == 0);

        test(obsv.threadObserver.GetFailedCount() == 0);
        if (!collocated)
        {
            test(obsv.connectionObserver.GetFailedCount() > 0);
            test(obsv.connectionEstablishmentObserver.GetFailedCount() > 0);
            test(obsv.endpointLookupObserver.GetFailedCount() > 0);
            test(obsv.invocationObserver.remoteObserver.GetFailedCount() > 0);
        }
        //test(obsv.dispatchObserver.getFailedCount() > 0);
        test(obsv.invocationObserver.GetFailedCount() > 0);

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
