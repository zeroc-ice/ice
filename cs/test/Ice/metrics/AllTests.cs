// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;

#if SILVERLIGHT
using System.Windows.Controls;
#endif
using Test;

public class AllTests : TestCommon.TestApp
{
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
	    _monitor = new IceUtilInternal.Monitor();
        }
        
        public void
        waitForUpdate()
        {
            lock(_monitor)
            {
                while(!_updated)
                {
                    _monitor.Wait();
                }
                // Ensure that the previous updates were committed, the setProperties call returns before 
                // notifying the callbacks so to ensure all the update callbacks have be notified we call
                // a second time, this will block until all the notifications from the first update have
                // completed.
                _serverProps.setProperties(new Dictionary<string, string>()); 
                _updated = false;
            }
        }
            
        public void
        updated(Dictionary<string, string> dict)
        {
            lock(_monitor)
            {
                _updated = true;
                _monitor.Notify();
            }
        }
        
        private bool _updated;
        private Ice.PropertiesAdminPrx _serverProps;
	private IceUtilInternal.Monitor _monitor;
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
            System.Threading.Thread.Sleep(50);
        }
    }
    
    static void
    testAttribute(IceMX.MetricsAdminPrx metrics, 
                  Ice.PropertiesAdminPrx props, 
                  UpdateCallbackI update,
                  string map, 
                  string attr,
                  string value,
#if COMPACT
                  Ice.VoidAction func)
#else
                  System.Action func)
#endif
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
        if(!view.ContainsKey(map))
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
        cprops.setProperties(getClientProps(cprops, props, map));
        sprops.setProperties(getServerProps(sprops, props, map));
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

#if SILVERLIGHT
    override public void run(Ice.Communicator communicator)
#else
    public static MetricsPrx allTests(Ice.Communicator communicator)
#endif
    {
        MetricsPrx metrics = MetricsPrxHelper.checkedCast(communicator.stringToProxy("metrics:default -p 12010"));
            
        Write("testing metrics admin facet checkedCast... ");
        Flush();
        Ice.ObjectPrx admin = communicator.getAdmin().ice_collocationOptimized(false);
        Ice.PropertiesAdminPrx clientProps = Ice.PropertiesAdminPrxHelper.checkedCast(admin, "Properties");
        IceMX.MetricsAdminPrx clientMetrics = IceMX.MetricsAdminPrxHelper.checkedCast(admin, "MetricsAdmin");
        test(clientProps != null && clientMetrics != null);

        admin = metrics.getAdmin();
        Ice.PropertiesAdminPrx serverProps = Ice.PropertiesAdminPrxHelper.checkedCast(admin, "Properties");
        IceMX.MetricsAdminPrx serverMetrics = IceMX.MetricsAdminPrxHelper.checkedCast(admin, "MetricsAdmin");
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
        test(view["Connection"].Length == 1 && view["Connection"][0].current == 1 && view["Connection"][0].total == 1);
        test(view["Thread"].Length == 1 && view["Thread"][0].current == 4 && view["Thread"][0].total == 4);
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

        view = clientMetrics.getMetricsView("View", out timestamp);
        test(view["Thread"].Length == 4);
        test(view["Connection"].Length == 2);
        test(view["Invocation"].Length == 1);

        IceMX.InvocationMetrics invoke = (IceMX.InvocationMetrics)view["Invocation"][0];

        test(invoke.id.IndexOf("[ice_ping]") > 0 && invoke.current == 0 && invoke.total == 5);
        test(invoke.remotes.Length == 2);
        test(invoke.remotes[0].total >= 2 && invoke.remotes[1].total >= 2);
        test((invoke.remotes[0].total + invoke.remotes[1].total) == 5);

        view = serverMetrics.getMetricsView("View", out timestamp);
        test(view["Thread"].Length > 4);
        test(view["Connection"].Length == 2);
        test(view["Dispatch"].Length == 1);
        test(view["Dispatch"][0].current == 0 && view["Dispatch"][0].total == 5);
        test(view["Dispatch"][0].id.IndexOf("[ice_ping]") > 0);

        metrics.ice_getConnection().close(false);
        metrics.ice_connectionId("Con1").ice_getConnection().close(false);

        waitForCurrent(clientMetrics, "View", "Connection", 0);
        waitForCurrent(serverMetrics, "View", "Connection", 0);

        clearView(clientProps, serverProps, update);
    
        WriteLine("ok");

        Write("testing connection metrics... ");
        Flush();

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
        updateProps(clientProps, serverProps, update, props, "Connection");

        test(!clientMetrics.getMetricsView("View", out timestamp).ContainsKey("Connection"));
        test(!serverMetrics.getMetricsView("View", out timestamp).ContainsKey("Connection"));

        metrics.ice_ping();

        IceMX.ConnectionMetrics cm1, sm1, cm2, sm2;
        cm1 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
        sm1 = (IceMX.ConnectionMetrics)serverMetrics.getMetricsView("View", out timestamp)["Connection"][0];

        metrics.ice_ping();

        cm2 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
        sm2 = (IceMX.ConnectionMetrics)serverMetrics.getMetricsView("View", out timestamp)["Connection"][0];

        test(cm2.sentBytes - cm1.sentBytes == 45); // 45 for ice_ping request
        test(cm2.receivedBytes - cm1.receivedBytes == 25); // 25 bytes for ice_ping response
        test(sm2.receivedBytes - sm1.receivedBytes == 45);
        test(sm2.sentBytes - sm1.sentBytes == 25);

        cm1 = cm2;
        sm1 = sm2;

        byte[] bs = new byte[0];
        metrics.opByteS(bs);

        cm2 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
        sm2 = (IceMX.ConnectionMetrics)serverMetrics.getMetricsView("View", out timestamp)["Connection"][0];
        long requestSz = cm2.sentBytes - cm1.sentBytes;
        long replySz = cm2.receivedBytes - cm1.receivedBytes;

        cm1 = cm2;
        sm1 = sm2;

        bs = new byte[456];
        metrics.opByteS(bs);

        cm2 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
        sm2 = (IceMX.ConnectionMetrics)serverMetrics.getMetricsView("View", out timestamp)["Connection"][0];

        test(cm2.sentBytes - cm1.sentBytes == requestSz + bs.Length + 4); // 4 is for the seq variable size
        test(cm2.receivedBytes - cm1.receivedBytes == replySz);
        test(sm2.receivedBytes - sm1.receivedBytes == requestSz + bs.Length + 4);
        test(sm2.sentBytes - sm1.sentBytes == replySz);

        cm1 = cm2;
        sm1 = sm2;

        bs = new byte[1024 * 1024 * 10]; // Try with large amount of data which should be sent in several chunks
        metrics.opByteS(bs);

        cm2 = (IceMX.ConnectionMetrics)clientMetrics.getMetricsView("View", out timestamp)["Connection"][0];
        sm2 = (IceMX.ConnectionMetrics)serverMetrics.getMetricsView("View", out timestamp)["Connection"][0];

        test((cm2.sentBytes - cm1.sentBytes) == (requestSz + bs.Length + 4)); // 4 is for the seq variable size
        test((cm2.receivedBytes - cm1.receivedBytes) == replySz);
        test((sm2.receivedBytes - sm1.receivedBytes) == (requestSz + bs.Length + 4));
        test((sm2.sentBytes - sm1.sentBytes) == replySz);
    
        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "state";
        updateProps(clientProps, serverProps, update, props, "Connection");

        Dictionary<string, IceMX.Metrics> map =
            toMap(serverMetrics.getMetricsView("View", out timestamp)["Connection"]);

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
            metrics.ice_timeout(500).ice_ping();
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
            if(sm1. failures >= 2)
            {
                break;
            }
            System.Threading.Thread.Sleep(10);
        }

        test(cm1.failures == 2 && sm1.failures >= 1);

        checkFailure(clientMetrics, "Connection", cm1.id, "Ice::TimeoutException", 1);
        checkFailure(clientMetrics, "Connection", cm1.id, "Ice::ConnectTimeoutException", 1);
        checkFailure(serverMetrics, "Connection", sm1.id, "Ice::ConnectionLostException", 0);

        MetricsPrx m = (MetricsPrx)metrics.ice_timeout(500).ice_connectionId("Con1");
        m.ice_ping();

        testAttribute(clientMetrics, clientProps, update, "Connection", "parent", "Communicator");
        //testAttribute(clientMetrics, clientProps, update, "Connection", "id", "");
        testAttribute(clientMetrics, clientProps, update, "Connection", "endpoint",
                      "tcp -e 1.1 -h 127.0.0.1 -p 12010 -t 500");

        testAttribute(clientMetrics, clientProps, update, "Connection", "endpointType", "1");
        testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsDatagram", "False");
        testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsSecure", "False");
        testAttribute(clientMetrics, clientProps, update, "Connection", "endpointProtocolVersion", "1.0");
        testAttribute(clientMetrics, clientProps, update, "Connection", "endpointEncodingVersion", "1.1");
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
        test(!clientMetrics.getMetricsView("View", out timestamp).ContainsKey("ConnectionEstablishment"));

        metrics.ice_ping();
    
        test(clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"].Length == 1);
        IceMX.Metrics m1 = clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"][0];
        test(m1.current == 0 && m1.total == 1 && m1.id.Equals("127.0.0.1:12010"));

        metrics.ice_getConnection().close(false);

        try
        {
            communicator.stringToProxy("test:tcp -p 12010 -h 127.0.0.50").ice_timeout(10).ice_ping();
            test(false);
        }
        catch(Ice.ConnectTimeoutException)
        {
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        test(clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"].Length == 2);
        m1 = clientMetrics.getMetricsView("View", out timestamp)["ConnectionEstablishment"][1];
        test(m1.id.Equals("127.0.0.50:12010") && m1.total == 2 && m1.failures == 2);

        checkFailure(clientMetrics, "ConnectionEstablishment", m1.id, "Ice::ConnectTimeoutException", 2);

#if COMPACT
        Ice.VoidAction c = () => { connect(metrics); };
#else
	System.Action c = () => { connect(metrics); };
#endif
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "parent", "Communicator", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "id", "127.0.0.1:12010", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpoint",
                      "tcp -e 1.1 -h 127.0.0.1 -p 12010", c);

        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointType", "1", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsDatagram", "False", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsSecure", "False", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointProtocolVersion", "1.0", 
                      c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointEncodingVersion", "1.1",
                      c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointTimeout", "-1", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointCompress", "False", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointHost", "127.0.0.1", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointPort", "12010", c);

        WriteLine("ok");

        Write("testing endpoint lookup metrics... ");
        Flush();

        props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update, props, "EndpointLookup");
        test(!clientMetrics.getMetricsView("View", out timestamp).ContainsKey("EndpointLookup"));

        Ice.ObjectPrx prx = communicator.stringToProxy("metrics:default -p 12010 -h localhost");
        prx.ice_ping();
    
        test(clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"].Length == 1);
        m1 = clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"][0];
        test(m1.current == 0 && m1.total == 1 && m1.id.Equals("tcp -e 1.1 -h localhost -p 12010"));

        prx.ice_getConnection().close(false);

        try
        {
            communicator.stringToProxy("test:tcp -p 12010 -h unknownfoo.zeroc.com").ice_ping();
            test(false);
        }
        catch(Ice.DNSException)
        {
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        test(clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"].Length == 2);
        m1 = clientMetrics.getMetricsView("View", out timestamp)["EndpointLookup"][1];
        test(m1.id.Equals("tcp -e 1.1 -h unknownfoo.zeroc.com -p 12010") && m1.total == 2 && m1.failures == 2);
    
        checkFailure(clientMetrics, "EndpointLookup", m1.id, "Ice::DNSException", 2);

        c = () => { connect(prx); };

        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "parent", "Communicator", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "id", "tcp -e 1.1 -h localhost -p 12010", 
                      c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpoint", 
                      "tcp -e 1.1 -h localhost -p 12010", c);

        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointType", "1", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsDatagram", "False", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsSecure", "False", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointProtocolVersion", "1.0", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointEncodingVersion", "1.1", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointTimeout", "-1", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointCompress", "False", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointHost", "localhost", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointPort", "12010", c);

        WriteLine("ok");

        Write("testing dispatch metrics... ");
        Flush();

        props["IceMX.Metrics.View.Map.Dispatch.GroupBy"] = "operation";
        updateProps(clientProps, serverProps, update, props, "Dispatch");
        test(!serverMetrics.getMetricsView("View", out timestamp).ContainsKey("Dispatch"));

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
        try
        {
            metrics.fail();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
        }

        map = toMap(serverMetrics.getMetricsView("View", out timestamp)["Dispatch"]);
        test(map.Count == 6);

        m1 = map["op"];
        test(m1.current == 0 && m1.total == 1 && m1.failures == 0);

        m1 = map["opWithUserException"];
        test(m1.current == 0 &m1.total == 1 &m1.failures == 1);
        checkFailure(serverMetrics, "Dispatch", m1.id, "Test::UserEx", 1);

        m1 = map["opWithLocalException"];
        test(m1.current == 0 && m1.total == 1 && m1.failures == 1);
        checkFailure(serverMetrics, "Dispatch", m1.id, "Ice::SyscallException", 1);

        m1 = map["opWithRequestFailedException"];
        test(m1.current == 0 && m1.total == 1 && m1.failures == 1);
        checkFailure(serverMetrics, "Dispatch", m1.id, "Ice::ObjectNotExistException", 1);

        m1 = map["opWithUnknownException"];
        test(m1.current == 0 && m1.total == 1 && m1.failures == 1);
        checkFailure(serverMetrics, "Dispatch", m1.id, "System.ArgumentOutOfRangeException", 1);

#if COMPACT
        Ice.VoidAction op = () => { invokeOp(metrics); };
#else
	System.Action op = () => { invokeOp(metrics); };
#endif

        testAttribute(serverMetrics, serverProps, update, "Dispatch", "parent", "TestAdapter", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "id", "metrics [op]", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpoint", 
                      "tcp -e 1.1 -h 127.0.0.1 -p 12010", op);
        //testAttribute(serverMetrics, serverProps, update, "Dispatch", "connection", "", op);

        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointType", "1", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsDatagram", "False", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsSecure", "False", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointProtocolVersion", "1.0", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointEncodingVersion", "1.1", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointTimeout", "-1", op);
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

        props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
        props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
        updateProps(clientProps, serverProps, update, props, "Invocation");
        test(!serverMetrics.getMetricsView("View", out timestamp).ContainsKey("Invocation"));

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
        try
        {
            metrics.fail();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
        }

        map = toMap(clientMetrics.getMetricsView("View", out timestamp)["Invocation"]);
        test(map.Count == 6);

        IceMX.InvocationMetrics im1;
        im1 = (IceMX.InvocationMetrics)map["op"];
        test(im1.current == 0 && im1.total == 1 && im1.failures == 0 && im1.retry == 0 && im1.remotes.Length == 1);

        im1 = (IceMX.InvocationMetrics)map["opWithUserException"];
        test(im1.current == 0 && im1.total == 1 && im1.failures == 1 && im1.retry == 0 && im1.remotes.Length == 1);
        checkFailure(clientMetrics, "Invocation", im1.id, "Test::UserEx", 1);

        im1 = (IceMX.InvocationMetrics)map["opWithLocalException"];
        test(im1.current == 0 && im1.total == 1 && im1.failures == 1 && im1.retry == 0 && im1.remotes.Length == 1);
        checkFailure(clientMetrics, "Invocation", im1.id, "Ice::UnknownLocalException", 1);

        im1 = (IceMX.InvocationMetrics)map["opWithRequestFailedException"];
        test(im1.current == 0 && im1.total == 1 && im1.failures == 1 && im1.retry == 0 && im1.remotes.Length == 1);
        checkFailure(clientMetrics, "Invocation", im1.id, "Ice::ObjectNotExistException", 1);

        im1 = (IceMX.InvocationMetrics)map["opWithUnknownException"];
        test(im1.current == 0 && im1.total == 1 && im1.failures == 1 && im1.retry == 0 && im1.remotes.Length == 1);
        checkFailure(clientMetrics, "Invocation", im1.id, "Ice::UnknownException", 1);

        im1 = (IceMX.InvocationMetrics)map["fail"];
        test(im1.current == 0 && im1.total == 1 && im1.failures == 1 && im1.retry == 1 && im1.remotes.Length == 2);
        checkFailure(clientMetrics, "Invocation", im1.id, "Ice::ConnectionLostException", 1);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "parent", "Communicator", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "id",
                      "metrics -t:tcp -e 1.1 -h 127.0.0.1 -p 12010 [op]", op);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "operation", "op", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "identity", "metrics", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "facet", "", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "encoding", "1.1", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "twoway", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "proxy",
                      "metrics -t:tcp -e 1.1 -h 127.0.0.1 -p 12010", op);

        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry1", "test", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry2", "", op);
        testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry3", "", op);

        WriteLine("ok");

#if SILVERLIGHT
        metrics.shutdown();
#else
        return metrics;
#endif
    }
}
