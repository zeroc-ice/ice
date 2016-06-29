// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

using Test;

public class AllTests : TestCommon.TestApp
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
            while(s.sentBytes != expected && nRetry-- > 0)
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
        catch(IceMX.UnknownMetricsView)
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
            lock(this)
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
            lock(this)
            {
                while(_wait)
                {
                    Monitor.Wait(this);
                }
                _wait = true;
            }
        }

        private bool _wait;
    };

    static private Dictionary<string, string>
    getClientProps(Ice.PropertiesAdminPrx p, Dictionary<string, string> orig, string m)
    {
        Dictionary<string, string> props = p.getPropertiesForPrefix("IceMX.Metrics");
        foreach(string e in new List<string>(props.Keys))
        {
            props[e] = "";
        }
        foreach(KeyValuePair<string, string> e in orig)
        {
            props[e.Key] = e.Value;
        }
        string map = "";
        if(m.Length > 0)
        {
            map += "Map." + m + '.';
        }
        props["IceMX.Metrics.View." + map + "Reject.parent"] = "Ice\\.Admin";
        props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = "12010";
        props["IceMX.Metrics.View." + map + "Reject.identity"] = ".*/admin|controller";
        return props;
    }

    static private Dictionary<string, string>
    getServerProps(Ice.PropertiesAdminPrx p, Dictionary<string, string> orig , string m)
    {
        Dictionary<string, string> props = p.getPropertiesForPrefix("IceMX.Metrics");
        foreach(string e in new List<string>(props.Keys))
        {
            props[e] = "";
        }
        foreach(KeyValuePair<string, string> e in orig)
        {
            props[e.Key] = e.Value;
        }
        string map = "";
        if(m.Length > 0)
        {
            map += "Map." + m + '.';
        }
        props["IceMX.Metrics.View." + map + "Reject.parent"] = "Ice\\.Admin|Controller";
        props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = "12010";
        return props;
    }

    class UpdateCallbackI : Ice.PropertiesAdminUpdateCallback
    {
        public UpdateCallbackI(Ice.PropertiesAdminPrx serverProps)
        {
            _updated = false;
            _serverProps = serverProps;
        }

        public void
        waitForUpdate()
        {
            lock(this)
            {
                while(!_updated)
                {
                    Monitor.Wait(this);
                }
            }

            // Ensure that the previous updates were committed, the setProperties call returns before
            // notifying the callbacks so to ensure all the update callbacks have be notified we call
            // a second time, this will block until all the notifications from the first update have
            // completed.
            _serverProps.setProperties(new Dictionary<string, string>());

            lock(this)
            {
                _updated = false;
            }
        }

        public void
        updated(Dictionary<string, string> dict)
        {
            lock(this)
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
        while(true)
        {
            long timestamp;
            Dictionary<string, IceMX.Metrics[]> view = metrics.getMetricsView(viewName, out timestamp);
            test(view.ContainsKey(map));
            bool ok = true;
            foreach(IceMX.Metrics m in view[map])
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
            Thread.Sleep(50);
        }
    }

    static void
    testAttribute(IceMX.MetricsAdminPrx metrics,
                  Ice.PropertiesAdminPrx props,
                  UpdateCallbackI update,
                  string map,
                  string attr,
                  string value,
                  System.Action func)
    {
        Dictionary<string, string> dict = new Dictionary<string, string>();
        dict.Add("IceMX.Metrics.View.Map." + map + ".GroupBy", attr);
        if(props.ice_getIdentity().category.Equals("client"))
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
        if(!view.ContainsKey(map) || view[map].Length == 0)
        {
            if(value.Length > 0)
            {
                WriteLine("no map `" + map + "' for group by = `" + attr + "'");
                test(false);
            }
        }
        else if(!view[map][0].id.Equals(value))
        {
            WriteLine("invalid attribute value: " + attr + " = " + value + " got " + view[map][0].id);
            test(false);
        }

        dict.Clear();
        if(props.ice_getIdentity().category.Equals("client"))
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
        if(proxy.ice_getCachedConnection() != null)
        {
            proxy.ice_getCachedConnection().close(false);
        }

        try
        {
            proxy.ice_ping();
        }
        catch(Ice.LocalException)
        {
        }

        if(proxy.ice_getCachedConnection() != null)
        {
            proxy.ice_getCachedConnection().close(false);
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
                  string value)
    {
        testAttribute(metrics, props, update, map, attr, value, ()=> {});
    }

    static void
    updateProps(Ice.PropertiesAdminPrx cprops,
                Ice.PropertiesAdminPrx sprops,
                UpdateCallbackI callback,
                Dictionary<string, string> props,
                string map)
    {
        if(sprops.ice_getConnection() != null)
        {
            cprops.setProperties(getClientProps(cprops, props, map));
            sprops.setProperties(getServerProps(sprops, props, map));
        }
        else
        {
            Dictionary<string, string> clientProps = getClientProps(cprops, props, map);
            Dictionary<string, string> serverProps = getClientProps(sprops, props, map);
            foreach(KeyValuePair<string, string> p in clientProps)
            {
                if(!serverProps.ContainsKey(p.Key))
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
    checkFailure(IceMX.MetricsAdminPrx m, string map, string id, string failure, int count)
    {
        IceMX.MetricsFailures f = m.getMetricsFailures("View", map, id);
        if(!f.failures.ContainsKey(failure))
        {
            WriteLine("couldn't find failure `" + failure + "' for `" + id + "'");
            test(false);
        }
        if(count > 0 && f.failures[failure] != count)
        {
            Write("count for failure `" + failure + "' of `" + id + "' is different from expected: ");
            WriteLine(count + " != " + f.failures[failure]);
            test(false);
        }
    }

    static Dictionary<string, IceMX.Metrics>
    toMap(IceMX.Metrics[] mmap)
    {
        Dictionary<string, IceMX.Metrics> m = new Dictionary<string, IceMX.Metrics>();
        foreach(IceMX.Metrics e in mmap)
        {
            m.Add(e.id, e);
        }
        return m;
    }

    public static MetricsPrx allTests(Ice.Communicator communicator, CommunicatorObserverI obsv)
    {
        MetricsPrx metrics = MetricsPrxHelper.checkedCast(communicator.stringToProxy("metrics:default -p 12010"));
        bool collocated = metrics.ice_getConnection() == null;

        Write("testing metrics admin facet checkedCast... ");
        Flush();
        Ice.ObjectPrx admin = communicator.getAdmin();
        Ice.PropertiesAdminPrx clientProps = Ice.PropertiesAdminPrxHelper.checkedCast(admin, "Properties");
        IceMX.MetricsAdminPrx clientMetrics = IceMX.MetricsAdminPrxHelper.checkedCast(admin, "Metrics");
        test(clientProps != null && clientMetrics != null);

        admin = metrics.getAdmin();
        Ice.PropertiesAdminPrx serverProps = Ice.PropertiesAdminPrxHelper.checkedCast(admin, "Properties");
        IceMX.MetricsAdminPrx serverMetrics = IceMX.MetricsAdminPrxHelper.checkedCast(admin, "Metrics");
        test(serverProps != null && serverMetrics != null);

        UpdateCallbackI update = new UpdateCallbackI(serverProps);
        ((Ice.NativePropertiesAdmin)communicator.findAdminFacet("Properties")).addUpdateCallback(update);

        WriteLine("ok");

        Dictionary<string, string> props = new Dictionary<string, string>();

        Write("testing group by none...");
        Flush();

        props.Add("IceMX.Metrics.View.GroupBy", "none");
        updateProps(clientProps, serverProps, update, props, "");
        long timestamp;
        Dictionary<string, IceMX.Metrics[]> view = clientMetrics.getMetricsView("View", out timestamp);
        if(!collocated)
        {
            test(view["Connection"].Length == 1 && view["Connection"][0].current == 1 &&
                 view["Connection"][0].total == 1);
        }
        test(view["Thread"].Length == 1 && view["Thread"][0].current == 5 && view["Thread"][0].total == 5);
        WriteLine("ok");

        Write("testing group by id...");
        Flush();

        props["IceMX.Metrics.View.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update, props, "");

        metrics.ice_ping();
        metrics.ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();
        metrics.ice_connectionId("Con1").ice_ping();

        waitForCurrent(clientMetrics, "View", "Invocation", 0);

        view = clientMetrics.getMetricsView("View", out timestamp);
        test(view["Thread"].Length == 5);
        if(!collocated)
        {
            test(view["Connection"].Length == 2);
        }
        test(view["Invocation"].Length == 1);

        IceMX.InvocationMetrics invoke = (IceMX.InvocationMetrics)view["Invocation"][0];

        test(invoke.id.IndexOf("[ice_ping]") > 0 && invoke.current == 0 && invoke.total == 5);
        if(!collocated)
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
        if(!collocated)
        {
            test(view["Connection"].Length == 2);
        }
        test(view["Dispatch"].Length == 1);
        test(view["Dispatch"][0].current <= 1 && view["Dispatch"][0].total == 5);
        test(view["Dispatch"][0].id.IndexOf("[ice_ping]") > 0);

        if(!collocated)
        {
            metrics.ice_getConnection().close(false);
            metrics.ice_connectionId("Con1").ice_getConnection().close(false);

            waitForCurrent(clientMetrics, "View", "Connection", 0);
            waitForCurrent(serverMetrics, "View", "Connection", 0);
        }

        clearView(clientProps, serverProps, update);

        WriteLine("ok");


        string endpoint = communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp") +
            " -h 127.0.0.1 -p 12010";
        string type = "";
        string isSecure = "";
        if(!collocated)
        {
            Ice.EndpointInfo endpointInfo = metrics.ice_getConnection().getEndpoint().getInfo();
            type = endpointInfo.type().ToString();
            isSecure = endpointInfo.secure() ? "True": "False";
        }

        Dictionary<string, IceMX.Metrics> map;

        if(!collocated)
        {
            Write("testing connection metrics... ");
            Flush();

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
                communicator.stringToProxy("controller:default -p 12011"));
            controller.hold();

            map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["active"].current == 1);
            map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["holding"].current == 1);

            metrics.ice_getConnection().close(false);

            map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["closing"].current == 1);
            map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["holding"].current == 1);

            controller.resume();

            map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Connection"]);
            test(map["holding"].current == 0);

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
            updateProps(clientProps, serverProps, update, props, "Connection");

            metrics.ice_getConnection().close(false);

            metrics.ice_timeout(500).ice_ping();
            controller.hold();
            try
            {
                ((MetricsPrx)metrics.ice_timeout(500)).opByteS(new byte[10000000]);
                test(false);
            }
            catch(Ice.TimeoutException)
            {
            }
            controller.resume();

            cm1 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
            while(true)
            {
                sm1 = (IceMX.ConnectionMetrics)serverMetrics.getMetricsView("View", out timestamp)["Connection"][0];
                if(sm1.failures >= 2)
                {
                    break;
                }
                Thread.Sleep(10);
            }
            test(cm1.failures == 2 && sm1.failures >= 2);

            checkFailure(clientMetrics, "Connection", cm1.id, "::Ice::TimeoutException", 1);
            checkFailure(clientMetrics, "Connection", cm1.id, "::Ice::ConnectTimeoutException", 1);
            checkFailure(serverMetrics, "Connection", sm1.id, "::Ice::ConnectionLostException", 0);

            MetricsPrx m = (MetricsPrx)metrics.ice_timeout(500).ice_connectionId("Con1");
            m.ice_ping();

            testAttribute(clientMetrics, clientProps, update, "Connection", "parent", "Communicator");
            //testAttribute(clientMetrics, clientProps, update, "Connection", "id", "");
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpoint",
                          endpoint + " -t 500");

            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointType", type);
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsDatagram", "False");
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsSecure", isSecure);
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointTimeout", "500");
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointCompress", "False");
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointHost", "127.0.0.1");
            testAttribute(clientMetrics, clientProps, update, "Connection", "endpointPort", "12010");

            testAttribute(clientMetrics, clientProps, update, "Connection", "incoming", "False");
            testAttribute(clientMetrics, clientProps, update, "Connection", "adapterName", "");
            testAttribute(clientMetrics, clientProps, update, "Connection", "connectionId", "Con1");
            testAttribute(clientMetrics, clientProps, update, "Connection", "localHost", "127.0.0.1");
            //testAttribute(clientMetrics, clientProps, update, "Connection", "localPort", "");
            testAttribute(clientMetrics, clientProps, update, "Connection", "remoteHost", "127.0.0.1");
            testAttribute(clientMetrics, clientProps, update, "Connection", "remotePort", "12010");
            testAttribute(clientMetrics, clientProps, update, "Connection", "mcastHost", "");
            testAttribute(clientMetrics, clientProps, update, "Connection", "mcastPort", "");

            m.ice_getConnection().close(false);

            waitForCurrent(clientMetrics, "View", "Connection", 0);
            waitForCurrent(serverMetrics, "View", "Connection", 0);

            WriteLine("ok");

            Write("testing connection establishment metrics... ");
            Flush();

            props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
            updateProps(clientProps, serverProps, update, props, "ConnectionEstablishment");
            test(clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"].Length == 0);

            metrics.ice_ping();

            test(clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"].Length == 1);
            IceMX.Metrics m1 = clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"][0];
            test(m1.current == 0 && m1.total == 1 && m1.id.Equals("127.0.0.1:12010"));

            metrics.ice_getConnection().close(false);
            controller.hold();
            try
            {
                communicator.stringToProxy("test:tcp -p 12010 -h 127.0.0.1").ice_timeout(10).ice_ping();
                test(false);
            }
            catch(Ice.ConnectTimeoutException)
            {
            }
            catch(Ice.LocalException)
            {
                test(false);
            }
            controller.resume();
            test(clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"].Length == 1);
            m1 = clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"][0];
            test(m1.id.Equals("127.0.0.1:12010") && m1.total == 3 && m1.failures == 2);

            checkFailure(clientMetrics, "ConnectionEstablishment", m1.id, "::Ice::ConnectTimeoutException", 2);

            System.Action c = () => { connect(metrics); };
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "parent", "Communicator", c);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "id", "127.0.0.1:12010", c);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpoint",
                          endpoint + " -t 60000", c);

            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointType", type, c);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsDatagram", "False",
                          c);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsSecure", isSecure,
                          c);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointTimeout", "60000", c);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointCompress", "False",
                          c);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointHost", "127.0.0.1",
                          c);
            testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointPort", "12010", c);

            WriteLine("ok");

            Write("testing endpoint lookup metrics... ");
            Flush();

            props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
            updateProps(clientProps, serverProps, update, props, "EndpointLookup");
            test(clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"].Length == 0);

            Ice.ObjectPrx prx = communicator.stringToProxy("metrics:default -p 12010 -h localhost -t infinite");
            prx.ice_ping();

            test(clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"].Length == 1);
            m1 = clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"][0];
            test(m1.current <= 1 && m1.total == 1 && m1.id.Equals(prx.ice_getConnection().getEndpoint().ToString()));

            prx.ice_getConnection().close(false);

            bool dnsException = false;
            try
            {
                communicator.stringToProxy("test:tcp -t 500 -p 12010 -h unknownfoo.zeroc.com").ice_ping();
                test(false);
            }
            catch(Ice.DNSException)
            {
                dnsException = true;
            }
            catch(Ice.LocalException)
            {
                // Some DNS servers don't fail on unknown DNS names.
            }
            test(clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"].Length == 2);
            m1 = clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"][0];
            if(!m1.id.Equals("tcp -h unknownfoo.zeroc.com -p 12010 -t 500"))
            {
                m1 = clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"][1];
            }
            test(m1.id.Equals("tcp -h unknownfoo.zeroc.com -p 12010 -t 500") && m1.total == 2 &&
                 (!dnsException || m1.failures == 2));
            if(dnsException)
            {
                checkFailure(clientMetrics, "EndpointLookup", m1.id, "::Ice::DNSException", 2);
            }

            c = () => { connect(prx); };

            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "parent", "Communicator", c);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "id",
                          prx.ice_getConnection().getEndpoint().ToString(), c);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpoint",
                          prx.ice_getConnection().getEndpoint().ToString(), c);

            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointType", type, c);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsDatagram", "False", c);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsSecure", isSecure, c);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointTimeout", "-1", c);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointCompress", "False", c);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointHost", "localhost", c);
            testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointPort", "12010", c);

            WriteLine("ok");
        }
        Write("testing dispatch metrics... ");
        Flush();

        props["IceMX.Metrics.View.Map.Dispatch.GroupBy"] = "operation";
        updateProps(clientProps, serverProps, update, props, "Dispatch");
        test(serverMetrics.getMetricsView("View", out timestamp)["Dispatch"].Length == 0);

        metrics.op();
        try
        {
            metrics.opWithUserException();
            test(false);
        }
        catch(UserEx)
        {
        }
        try
        {
            metrics.opWithRequestFailedException();
            test(false);
        }
        catch(Ice.RequestFailedException)
        {
        }
        try
        {
            metrics.opWithLocalException();
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        try
        {
            metrics.opWithUnknownException();
            test(false);
        }
        catch(Ice.UnknownException)
        {
        }
        if(!collocated)
        {
            try
            {
                metrics.fail();
                test(false);
            }
            catch(Ice.ConnectionLostException)
            {
            }
        }

        map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Dispatch"]);
        test(!collocated ? map.Count == 6 : map.Count == 5);

        IceMX.DispatchMetrics dm1;
        dm1 = (IceMX.DispatchMetrics)map["op"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 0 && dm1.userException == 0);
        test(dm1.size == 21 && dm1.replySize == 7);

        dm1 = (IceMX.DispatchMetrics)map["opWithUserException"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 0 && dm1.userException == 1);
        test(dm1.size == 38 && dm1.replySize == 23);

        dm1 = (IceMX.DispatchMetrics)map["opWithLocalException"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "::Ice::SyscallException", 1);
        test(dm1.size == 39 && dm1.replySize > 7); // Reply contains the exception stack depending on the OS.

        dm1 = (IceMX.DispatchMetrics)map["opWithRequestFailedException"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "::Ice::ObjectNotExistException", 1);
        test(dm1.size == 47 && dm1.replySize == 40);

        dm1 = (IceMX.DispatchMetrics)map["opWithUnknownException"];
        test(dm1.current <= 1 && dm1.total == 1 && dm1.failures == 1 && dm1.userException == 0);
        checkFailure(serverMetrics, "Dispatch", dm1.id, "System.ArgumentOutOfRangeException", 1);
        test(dm1.size == 41 && dm1.replySize > 7); // Reply contains the exception stack depending on the OS.

        System.Action op = () => { invokeOp(metrics); };
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "parent", "TestAdapter", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "id", "metrics [op]", op);

        if(!collocated)
        {
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpoint",
                          endpoint + " -t 60000", op);
            //testAttribute(serverMetrics, serverProps, update, "Dispatch", "connection", "", op);

            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointType", type, op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsDatagram", "False", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsSecure", isSecure, op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointTimeout", "60000", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointCompress", "False", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointHost", "127.0.0.1", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointPort", "12010", op);

            testAttribute(serverMetrics, serverProps, update, "Dispatch", "incoming", "True", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "adapterName", "TestAdapter", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "connectionId", "", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "localHost", "127.0.0.1", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "localPort", "12010", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "remoteHost", "127.0.0.1", op);
            //testAttribute(serverMetrics, serverProps, update, "Dispatch", "remotePort", "12010", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastHost", "", op);
            testAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastPort", "", op);
        }

        testAttribute(serverMetrics, serverProps, update, "Dispatch", "operation", "op", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "identity", "metrics", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "facet", "", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "mode", "twoway", op);

        testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry1", "test", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry2", "", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry3", "", op);

        WriteLine("ok");

        Write("testing invocation metrics... ");
        Flush();

        //
        // Tests for twoway
        //
        props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
        props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
        props["IceMX.Metrics.View.Map.Invocation.Map.Collocated.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update, props, "Invocation");
        test(serverMetrics.getMetricsView("View", out timestamp)["Invocation"].Length == 0);

        Callback cb = new Callback();

        metrics.op();
        metrics.end_op(metrics.begin_op());
        metrics.begin_op().whenCompleted(cb.response, cb.exception);
        cb.waitForResponse();

        try
        {
            metrics.opWithUserException();
            test(false);
        }
        catch(UserEx)
        {
        }
        try
        {
            metrics.end_opWithUserException(metrics.begin_opWithUserException());
            test(false);
        }
        catch(UserEx)
        {
        }
        metrics.begin_opWithUserException().whenCompleted(cb.response, cb.exception);
        cb.waitForResponse();

        try
        {
            metrics.opWithRequestFailedException();
            test(false);
        }
        catch(Ice.RequestFailedException)
        {
        }
        try
        {
            metrics.end_opWithRequestFailedException(metrics.begin_opWithRequestFailedException());
            test(false);
        }
        catch(Ice.RequestFailedException)
        {
        }
        metrics.begin_opWithRequestFailedException().whenCompleted(cb.response, cb.exception);
        cb.waitForResponse();

        try
        {
            metrics.opWithLocalException();
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        try
        {
            metrics.end_opWithLocalException(metrics.begin_opWithLocalException());
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        metrics.begin_opWithLocalException().whenCompleted(cb.response, cb.exception);
        cb.waitForResponse();

        try
        {
            metrics.opWithUnknownException();
            test(false);
        }
        catch(Ice.UnknownException)
        {
        }
        try
        {
            metrics.end_opWithUnknownException(metrics.begin_opWithUnknownException());
            test(false);
        }
        catch(Ice.UnknownException)
        {
        }
        metrics.begin_opWithUnknownException().whenCompleted(cb.response, cb.exception);
        cb.waitForResponse();

        if(!collocated)
        {
            try
            {
                metrics.fail();
                test(false);
            }
            catch(Ice.ConnectionLostException)
            {
            }
            try
            {
                metrics.end_fail(metrics.begin_fail());
                test(false);
            }
            catch(Ice.ConnectionLostException)
            {
            }
            metrics.begin_fail().whenCompleted(cb.response, cb.exception);
            cb.waitForResponse();
        }

        map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
        test(map.Count == (!collocated ? 6 : 5));

        IceMX.InvocationMetrics im1;
        IceMX.ChildInvocationMetrics rim1;
        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 0 && im1.retry == 0);
        test(!collocated ? im1.remotes.Length == 1 : im1.collocated.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(!collocated ? im1.remotes[0] : im1.collocated[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 63 && rim1.replySize == 21);

        im1 = (IceMX.InvocationMetrics)map["opWithUserException"];
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 0 && im1.retry == 0);
        test(!collocated ? im1.remotes.Length == 1 : im1.collocated.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(!collocated ? im1.remotes[0] : im1.collocated[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 114 && rim1.replySize == 69);
        test(im1.userException == 3);

        im1 = (IceMX.InvocationMetrics)map["opWithLocalException"];
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0);
        test(!collocated ? im1.remotes.Length == 1 : im1.collocated.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(!collocated ? im1.remotes[0] : im1.collocated[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 117 && rim1.replySize > 7);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::UnknownLocalException", 3);

        im1 = (IceMX.InvocationMetrics)map["opWithRequestFailedException"];
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0);
        test(!collocated ? im1.remotes.Length == 1 : im1.collocated.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(!collocated ? im1.remotes[0] : im1.collocated[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 141 && rim1.replySize == 120);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::ObjectNotExistException", 3);

        im1 = (IceMX.InvocationMetrics)map["opWithUnknownException"];
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 0);
        test(!collocated ? im1.remotes.Length == 1 : im1.collocated.Length == 1);
        rim1 = (IceMX.ChildInvocationMetrics)(!collocated ? im1.remotes[0] : im1.collocated[0]);
        test(rim1.current == 0 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 123 && rim1.replySize > 7);
        checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::UnknownException", 3);

        if(!collocated)
        {
            im1 = (IceMX.InvocationMetrics)map["fail"];
            test(im1.current <= 1 && im1.total == 3 && im1.failures == 3 && im1.retry == 3 && im1.remotes.Length == 6);
            test(im1.remotes[0].current == 0 && im1.remotes[0].total == 1 && im1.remotes[0].failures == 1);
            test(im1.remotes[1].current == 0 && im1.remotes[1].total == 1 && im1.remotes[1].failures == 1);
            test(im1.remotes[2].current == 0 && im1.remotes[2].total == 1 && im1.remotes[2].failures == 1);
            test(im1.remotes[3].current == 0 && im1.remotes[3].total == 1 && im1.remotes[3].failures == 1);
            test(im1.remotes[4].current == 0 && im1.remotes[4].total == 1 && im1.remotes[4].failures == 1);
            test(im1.remotes[5].current == 0 && im1.remotes[5].total == 1 && im1.remotes[5].failures == 1);
            checkFailure(clientMetrics, "Invocation", im1.id, "::Ice::ConnectionLostException", 3);
        }

        testAttribute(clientMetrics, clientProps, update, "Invocation", "parent", "Communicator", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "id", "metrics -t -e 1.1 [op]", op);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "operation", "op", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "identity", "metrics", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "facet", "", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "encoding", "1.1", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "twoway", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "proxy",
                      "metrics -t -e 1.1:" + endpoint + " -t 60000", op);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry1", "test", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry2", "", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry3", "", op);

        //
        // Oneway tests
        //
        clearView(clientProps, serverProps, update);
        props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
        props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
        updateProps(clientProps, serverProps, update, props, "Invocation");

        MetricsPrx metricsOneway = (MetricsPrx)metrics.ice_oneway();
        metricsOneway.op();
        metricsOneway.end_op(metricsOneway.begin_op());
        metricsOneway.begin_op().whenCompleted(cb.response, cb.exception).waitForSent();

        map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
        test(map.Count == 1);

        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.current <= 1 && im1.total == 3 && im1.failures == 0 && im1.retry == 0);
        test(!collocated ? (im1.remotes.Length == 1) : (im1.collocated.Length == 1));
        rim1 = (IceMX.ChildInvocationMetrics)(!collocated ? im1.remotes[0] : im1.collocated[0]);
        test(rim1.current <= 1 && rim1.total == 3 && rim1.failures == 0);
        test(rim1.size == 63 && rim1.replySize == 0);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "oneway",
                      () => { invokeOp(metricsOneway); });

        //
        // Batch oneway tests
        //
        props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
        props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
        updateProps(clientProps, serverProps, update, props, "Invocation");

        MetricsPrx metricsBatchOneway = (MetricsPrx)metrics.ice_batchOneway();
        metricsBatchOneway.op();
        metricsBatchOneway.end_op(metricsBatchOneway.begin_op());
        //metricsBatchOneway.begin_op().whenCompleted(cb.response, cb.exception).waitForSent();

        map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
        test(map.Count == 1);

        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.current == 0 && im1.total == 2 && im1.failures == 0 && im1.retry == 0);
        test(im1.remotes.Length == 0);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "batch-oneway",
                      () => { invokeOp(metricsBatchOneway); });

        WriteLine("ok");

        Write("testing metrics view enable/disable...");
        Flush();

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
        catch(IceMX.UnknownMetricsView)
        {
        }

        WriteLine("ok");

        Write("testing instrumentation observer delegate... ");
        Flush();

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
            test(obsv.invocationObserver.remoteObserver.getCurrent() == 0);
        }
        else
        {
            for(int i = 0; i < 10; ++i)
            {
                if(obsv.invocationObserver.collocatedObserver.getCurrent() > 0)
                {
                    Thread.Sleep(10);
                }
                else
                {
                    break;
                }
            }
            test(obsv.invocationObserver.collocatedObserver.getCurrent() == 0);
        }
        test(obsv.dispatchObserver.getCurrent() == 0);
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

        WriteLine("ok");
        return metrics;
    }
}
