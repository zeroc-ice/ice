// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.IceMX;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Metrics
{
    public static class AllTests
    {
        public static ConnectionMetrics? GetServerConnectionMetrics(IMetricsAdminPrx metrics, long expected)
        {
            try
            {
                ConnectionMetrics? s;
                s = (ConnectionMetrics?)metrics.GetMetricsView("View").ReturnValue["Connection"][0];
                TestHelper.Assert(s != null);
                int nRetry = 30;
                while (s.SentBytes < expected && nRetry-- > 0)
                {
                    // On some platforms, it's necessary to wait a little before obtaining the server metrics
                    // to get an accurate sentBytes metric. The sentBytes metric is updated before the response
                    // to the operation is sent and getMetricsView can be dispatched before the metric is really
                    // updated.
                    Thread.Sleep(100);
                    s = (ConnectionMetrics?)metrics.GetMetricsView("View").ReturnValue["Connection"][0];
                    TestHelper.Assert(s != null);
                }
                if (s.SentBytes < expected)
                {
                    Console.Error.WriteLine($"received only {s.SentBytes} from the server instead of {expected}");
                }
                TestHelper.Assert(s.SentBytes >= expected);
                return s;
            }
            catch (UnknownMetricsView)
            {
                TestHelper.Assert(false);
                return null;
            }
        }

        public static string GetPort(IPropertiesAdminPrx p, int port) =>
            $"{TestHelper.GetTestBasePort(p.Communicator.GetProperties()) + port}";

        private static Dictionary<string, string> MergeProps(IPropertiesAdminPrx p, Dictionary<string, string> orig)
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
            return props;
        }

        private static Dictionary<string, string> GetClientProps(
            IPropertiesAdminPrx p,
            Dictionary<string, string> orig,
            string m)
        {
            Dictionary<string, string> props = MergeProps(p, orig);
            string map = m.Length > 0 ? $"Map.{m}." : "";
            props[$"IceMX.Metrics.View.{map}Reject.parent"] = "Ice\\.Admin";
            props[$"IceMX.Metrics.View.{map}Accept.endpointPort"] = $"{GetPort(p, 0)}|{GetPort(p, 2)}";
            props[$"IceMX.Metrics.View.{map}Reject.identity"] = ".*/admin|controller";
            return props;
        }

        private static Dictionary<string, string> GetColocatedProps(
            IPropertiesAdminPrx p,
            Dictionary<string, string> orig,
            string m)
        {
            Dictionary<string, string> props = MergeProps(p, orig);
            string map = m.Length > 0 ? $"Map.{m}." : "";
            props[$"IceMX.Metrics.View.{map}Reject.parent"] = "Ice\\.Admin|Controller";
            props[$"IceMX.Metrics.View.{map}Accept.endpointHost"] = "TestAdapter";
            props[$"IceMX.Metrics.View.{map}Reject.identity"] = ".*/admin|controller";
            return props;
        }

        private static Dictionary<string, string> GetServerProps(
            IPropertiesAdminPrx p,
            Dictionary<string, string> orig,
            string m)
        {
            Dictionary<string, string> props = MergeProps(p, orig);
            string map = m.Length > 0 ? $"Map.{m}." : "";
            props[$"IceMX.Metrics.View.{map}Reject.parent"] = "Ice\\.Admin|Controller";
            props[$"IceMX.Metrics.View.{map}Accept.endpointPort"] = $"{GetPort(p, 0)}|{GetPort(p, 2)}";
            return props;
        }

        public class UpdateCallbackI
        {
            private readonly object _mutex = new();
            private readonly IPropertiesAdminPrx _serverProps;
            private bool _updated;

            public UpdateCallbackI(IPropertiesAdminPrx serverProps)
            {
                _updated = false;
                _serverProps = serverProps;
            }

            public void WaitForUpdate()
            {
                lock (_mutex)
                {
                    while (!_updated)
                    {
                        Monitor.Wait(_mutex);
                    }
                }

                // Ensure that the previous updates were committed, the setProperties call returns before
                // notifying the callbacks so to ensure all the update callbacks have be notified we call
                // a second time, this will block until all the notifications from the first update have
                // completed.
                _serverProps.SetProperties(new Dictionary<string, string>());

                lock (_mutex)
                {
                    _updated = false;
                }
            }

            public void Updated()
            {
                lock (_mutex)
                {
                    _updated = true;
                    Monitor.Pulse(_mutex);
                }
            }
        }

        public static void WaitForCurrent(IMetricsAdminPrx metrics, string viewName, string map, int value)
        {
            while (true)
            {
                Dictionary<string, IceMX.Metrics?[]> view = metrics.GetMetricsView(viewName).ReturnValue;
                TestHelper.Assert(view.ContainsKey(map));
                int v = 0;
                foreach (IceMX.Metrics? m in view[map])
                {
                    TestHelper.Assert(m != null);
                    v += m.Current;
                }
                if (v == value)
                {
                    return;
                }
                Thread.Sleep(50);
            }
        }

        public static void WaitForObserverCurrent(Observer observer)
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

        public static void TestAttribute(
            IMetricsAdminPrx metrics,
            IPropertiesAdminPrx props,
            UpdateCallbackI update,
            string map,
            string attr,
            string value,
            Action func,
            TextWriter output)
        {
            var dict = new Dictionary<string, string>
            {
                { $"IceMX.Metrics.View.Map.{map}.GroupBy", attr }
            };

            if (props.Identity.Category.Equals("client"))
            {
                props.SetProperties(GetClientProps(props, dict, map));
                update.WaitForUpdate();
            }
            else if (props.Identity.Category.Equals("colocated"))
            {
                props.SetProperties(GetColocatedProps(props, dict, map));
                update.WaitForUpdate();
            }
            else
            {
                props.SetProperties(GetServerProps(props, dict, map));
                props.SetProperties(new Dictionary<string, string>());
            }

            func();
            Dictionary<string, IceMX.Metrics?[]> view = metrics.GetMetricsView("View").ReturnValue;
            if (!view.ContainsKey(map) || view[map].Length == 0)
            {
                if (value.Length > 0)
                {
                    output.WriteLine($"no map `{map}' for group by = `{attr}'");
                    TestHelper.Assert(false);
                }
            }
            else if (!view[map][0]!.Id.Equals(value))
            {
                output.WriteLine($"invalid attribute value: {attr} = `{value}' got `{view[map][0]!.Id}'");
                TestHelper.Assert(false);
            }

            dict.Clear();
            if (props.Identity.Category.Equals("client"))
            {
                props.SetProperties(GetClientProps(props, dict, map));
                update.WaitForUpdate();
            }
            else if (props.Identity.Category.Equals("colocated"))
            {
                props.SetProperties(GetColocatedProps(props, dict, map));
                update.WaitForUpdate();
            }
            else
            {
                props.SetProperties(GetServerProps(props, dict, map));
                props.SetProperties(new Dictionary<string, string>());
            }
        }

        public static void Connect(IObjectPrx proxy)
        {
            Connection? conn = proxy.GetCachedConnection();
            if (conn != null)
            {
                conn.GoAwayAsync();
            }

            try
            {
                proxy.IcePing();
            }
            catch
            {
            }

            conn = proxy.GetCachedConnection();
            if (conn != null)
            {
                conn.GoAwayAsync();
            }
        }

        public static void InvokeOp(IMetricsPrx proxy)
        {
            var ctx = new Dictionary<string, string>
            {
                { "entry1", "test" },
                { "entry2", "" }
            };
            proxy.Op(ctx);
        }

        public static void TestAttribute(
            IMetricsAdminPrx metrics,
            IPropertiesAdminPrx props,
            UpdateCallbackI update,
            string map,
            string attr,
            string value,
            TextWriter output) => TestAttribute(metrics, props, update, map, attr, value, () => { }, output);

        public static void UpdateProps(
            IPropertiesAdminPrx cprops,
            IPropertiesAdminPrx sprops,
            UpdateCallbackI callback,
            Dictionary<string, string> props,
            string map)
        {
            if (cprops.Identity.Category.Equals("client"))
            {
                cprops.SetProperties(GetClientProps(cprops, props, map));
                sprops.SetProperties(GetServerProps(sprops, props, map));
            }
            else
            {
                cprops.SetProperties(GetColocatedProps(cprops, props, map));
            }
            callback.WaitForUpdate();
        }

        public static void ClearView(IPropertiesAdminPrx cprops, IPropertiesAdminPrx sprops, UpdateCallbackI callback)
        {
            Dictionary<string, string> dict;

            dict = cprops.GetPropertiesForPrefix("IceMX.Metrics");
            dict["IceMX.Metrics.View.Disabled"] = "1";
            cprops.SetProperties(dict);

            dict = sprops.GetPropertiesForPrefix("IceMX.Metrics");
            dict["IceMX.Metrics.View.Disabled"] = "1";
            sprops.SetProperties(dict);

            callback.WaitForUpdate();

            dict = cprops.GetPropertiesForPrefix("IceMX.Metrics");
            dict["IceMX.Metrics.View.Disabled"] = "";
            cprops.SetProperties(dict);

            dict = sprops.GetPropertiesForPrefix("IceMX.Metrics");
            dict["IceMX.Metrics.View.Disabled"] = "";
            sprops.SetProperties(dict);

            callback.WaitForUpdate();
        }

        public static void CheckFailure(
            IMetricsAdminPrx m,
            string map,
            string id,
            string failure,
            int count,
            TextWriter output)
        {
            MetricsFailures f = m.GetMetricsFailures("View", map, id);
            if (!f.Failures.ContainsKey(failure))
            {
                output.WriteLine($"couldn't find failure `{failure}' for `{id}'");
                foreach (KeyValuePair<string, int> value in f.Failures)
                {
                    output.WriteLine($"{value.Key} = {value.Value}");
                }
                TestHelper.Assert(false);
            }
            if (count > 0 && f.Failures[failure] != count)
            {
                output.Write($"count for failure `{failure}' of `{id}' is different from expected: ");
                output.WriteLine(count + " != " + f.Failures[failure]);
                TestHelper.Assert(false);
            }
        }

        public static Dictionary<string, IceMX.Metrics> ToMap(IceMX.Metrics[] mmap)
        {
            var m = new Dictionary<string, IceMX.Metrics>();
            foreach (IceMX.Metrics e in mmap)
            {
                m.Add(e.Id, e);
            }
            return m;
        }

        public static async Task RunAsync(TestHelper helper,  CommunicatorObserverI obsv, bool colocated)
        {
            Communicator communicator = helper.Communicator;
            TextWriter output = helper.Output;

            bool ice1 = helper.Protocol == Protocol.Ice1;

            string host = helper.Host;
            string port = $"{helper.BasePort + 0}";
            string hostAndPort = host + ":" + port;
            string transport = helper.Transport;
            string defaultTimeout = "60000";
            string endpoint = ice1 ? $"{transport} -h {host} -p {port}" : $"ice+{transport}://{hostAndPort}";
            string adapterName = colocated ? "TestAdapter" : "";

            IMetricsPrx metrics = IMetricsPrx.Parse(ice1 ? $"metrics:{endpoint}" : $"{endpoint}/metrics", communicator);

            IObjectPrx? admin = await communicator.GetAdminAsync();
            TestHelper.Assert(admin != null);
            var clientProps = admin.Clone(IPropertiesAdminPrx.Factory, facet: "Properties");
            var clientMetrics = admin.Clone(IMetricsAdminPrx.Factory, facet: "Metrics");
            TestHelper.Assert(clientProps != null && clientMetrics != null);

            admin = metrics.GetAdmin();
            TestHelper.Assert(admin != null);
            var serverProps = admin.Clone(IPropertiesAdminPrx.Factory, facet: "Properties");
            var serverMetrics = admin.Clone(IMetricsAdminPrx.Factory, facet: "Metrics");
            TestHelper.Assert(serverProps != null && serverMetrics != null);

            var update = new UpdateCallbackI(serverProps);
            ((IAsyncPropertiesAdmin)communicator.FindAdminFacet("Properties")!).Updated += (_, u) => update.Updated();

            var props = new Dictionary<string, string>();

            output.Write("testing group by none...");
            output.Flush();

            props.Add("IceMX.Metrics.View.GroupBy", "none");
            UpdateProps(clientProps, serverProps, update, props, "");
            Dictionary<string, IceMX.Metrics?[]> view = clientMetrics.GetMetricsView("View").ReturnValue;
            TestHelper.Assert(
                view["Connection"].Length == 1 &&
                view["Connection"][0]!.Current == 1 &&
                view["Connection"][0]!.Total == 1);
            output.WriteLine("ok");

            output.Write("testing group by id...");
            output.Flush();

            props["IceMX.Metrics.View.GroupBy"] = "id";
            UpdateProps(clientProps, serverProps, update, props, "");

            metrics.IcePing();
            metrics.IcePing();
            metrics.Clone(label: "Con1").IcePing();
            metrics.Clone(label: "Con1").IcePing();
            metrics.Clone(label: "Con1").IcePing();

            WaitForCurrent(clientMetrics, "View", "Invocation", 0);
            WaitForCurrent(serverMetrics, "View", "Dispatch", 0);

            view = clientMetrics.GetMetricsView("View").ReturnValue;
            TestHelper.Assert(view["Connection"].Length == 2);

            if (ice1)
            {
                // With ice1 the label is not part of the ID
                TestHelper.Assert(view["Invocation"].Length == 1);
                var invoke = (InvocationMetrics)view["Invocation"][0]!;
                TestHelper.Assert(invoke.Id.IndexOf("[ice_ping]", StringComparison.InvariantCulture) > 0 &&
                                  invoke.Current == 0 &&
                                  invoke.Total == 5);
                TestHelper.Assert(invoke.Children.Length == 2);
                TestHelper.Assert(invoke.Children[0]!.Total >= 2 && invoke.Children[1]!.Total >= 2);
                TestHelper.Assert((invoke.Children[0]!.Total + invoke.Children[1]!.Total) == 5);
            }
            else
            {
                // With ice2 the label is part of the ID
                TestHelper.Assert(view["Invocation"].Length == 2);
                var invoke = (InvocationMetrics)view["Invocation"][0]!;
                TestHelper.Assert(invoke.Id.IndexOf("[ice_ping]", StringComparison.InvariantCulture) > 0 &&
                                  invoke.Current == 0 &&
                                  invoke.Total == 2);
                TestHelper.Assert(invoke.Children.Length == 1);
                TestHelper.Assert(invoke.Children[0]!.Total == 2);

                invoke = (InvocationMetrics)view["Invocation"][1]!;
                TestHelper.Assert(invoke.Id.IndexOf("[ice_ping]", StringComparison.InvariantCulture) > 0 &&
                                  invoke.Current == 0 &&
                                  invoke.Total == 3);
                TestHelper.Assert(invoke.Children.Length == 1);
                TestHelper.Assert(invoke.Children[0]!.Total == 3);
            }

            view = serverMetrics.GetMetricsView("View").ReturnValue;
            TestHelper.Assert(view["Connection"].Length == 2);

            TestHelper.Assert(view["Dispatch"].Length == 1);
            TestHelper.Assert(view["Dispatch"][0]!.Current == 0 && view["Dispatch"][0]!.Total == 5);
            TestHelper.Assert(view["Dispatch"][0]!.Id.IndexOf("[ice_ping]", StringComparison.InvariantCulture) > 0);

            _ = (await metrics.GetConnectionAsync()).GoAwayAsync();
            _ = (await metrics.Clone(label: "Con1").GetConnectionAsync()).GoAwayAsync();

            WaitForCurrent(clientMetrics, "View", "Connection", 0);
            WaitForCurrent(serverMetrics, "View", "Connection", 0);

            ClearView(clientProps, serverProps, update);

            output.WriteLine("ok");

            Endpoint connectionEndpoint = metrics.GetCachedConnection()!.Endpoint;
            string transportName = connectionEndpoint.Transport.ToString();
            string isSecure = connectionEndpoint.IsAlwaysSecure ? "True" : "False";

            Dictionary<string, IceMX.Metrics> map;

            output.Write("testing connection metrics... ");
            output.Flush();

            ConnectionMetrics cm1, sm1, cm2, sm2;
            if (!colocated)
            {
                props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
                UpdateProps(clientProps, serverProps, update, props, "Connection");
                TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["Connection"].Length == 0);
                TestHelper.Assert(serverMetrics.GetMetricsView("View").ReturnValue["Connection"].Length == 0);

                metrics.IcePing();

                cm1 = (ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0]!;
                sm1 = GetServerConnectionMetrics(serverMetrics, ice1 ? 22 : 10)!;

                metrics.IcePing();

                cm2 = (ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0]!;
                sm2 = GetServerConnectionMetrics(serverMetrics, ice1 ? 44 : 20)!;

                if (ice1)
                {
                    TestHelper.Assert(cm2.SentBytes - cm1.SentBytes == 45); // ice_ping request
                    TestHelper.Assert(cm2.ReceivedBytes - cm1.ReceivedBytes == 25); // ice_ping response
                    TestHelper.Assert(sm2.ReceivedBytes - sm1.ReceivedBytes == 45);
                    TestHelper.Assert(sm2.SentBytes - sm1.SentBytes == 25);
                }
                else
                {
                    TestHelper.Assert(cm2.SentBytes - cm1.SentBytes == 43); // ice_ping request
                    TestHelper.Assert(cm2.ReceivedBytes - cm1.ReceivedBytes == 16); // ice_ping response
                    TestHelper.Assert(sm2.ReceivedBytes - sm1.ReceivedBytes == 43);
                    TestHelper.Assert(sm2.SentBytes - sm1.SentBytes == 16);
                }

                cm1 = cm2;
                sm1 = sm2;

                byte[] bs = Array.Empty<byte>();
                metrics.OpByteS(bs);

                cm2 = (ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0]!;
                sm2 = GetServerConnectionMetrics(serverMetrics, sm1.SentBytes + cm2.ReceivedBytes - cm1.ReceivedBytes)!;
                long requestSz = cm2.SentBytes - cm1.SentBytes;
                long replySz = cm2.ReceivedBytes - cm1.ReceivedBytes;

                cm1 = cm2;
                sm1 = sm2;

                bs = new byte[456];
                metrics.OpByteS(bs);

                cm2 = (ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0]!;
                sm2 = GetServerConnectionMetrics(serverMetrics, sm1.SentBytes + replySz)!;

                // TODO: explanation
                int sizeLengthIncrease = helper.Encoding == Encoding.V11 ? 4 : 2;
                TestHelper.Assert(cm2.SentBytes - cm1.SentBytes == requestSz + bs.Length + sizeLengthIncrease);
                TestHelper.Assert(cm2.ReceivedBytes - cm1.ReceivedBytes == replySz);
                TestHelper.Assert(sm2.ReceivedBytes - sm1.ReceivedBytes == requestSz + bs.Length + sizeLengthIncrease);
                TestHelper.Assert(sm2.SentBytes - sm1.SentBytes == replySz);

                cm1 = cm2;
                sm1 = sm2;

                bs = new byte[1024 * 1024 * 10]; // Try with large amount of data which should be sent in several chunks
                metrics.OpByteS(bs);

                cm2 = (ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0]!;
                sm2 = GetServerConnectionMetrics(serverMetrics, sm1.SentBytes + replySz)!;

                // TODO: explanation!
                sizeLengthIncrease = helper.Encoding == Encoding.V11 ? 4 : 3;
                if (!ice1 && metrics.GetCachedConnection() is IPConnection)
                {
                    sizeLengthIncrease += 1921; // 1921 additional bytes for the Slic frame fragmentation.
                }

                TestHelper.Assert((cm2.SentBytes - cm1.SentBytes) == (requestSz + bs.Length + sizeLengthIncrease));
                TestHelper.Assert((cm2.ReceivedBytes - cm1.ReceivedBytes) == replySz);

                TestHelper.Assert((sm2.ReceivedBytes - sm1.ReceivedBytes) == (requestSz + bs.Length + sizeLengthIncrease));
                TestHelper.Assert((sm2.SentBytes - sm1.SentBytes) == replySz);
            }

            metrics.IcePing();

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "state";
            UpdateProps(clientProps, serverProps, update, props, "Connection");

            map = ToMap(serverMetrics.GetMetricsView("View").ReturnValue["Connection"]!);

            TestHelper.Assert(map["active"].Current == 1);
            _ = (await metrics.GetConnectionAsync()).GoAwayAsync();

            map = ToMap(clientMetrics.GetMetricsView("View").ReturnValue["Connection"]!);
            // The connection might already be closed so it can be 0 or 1
            TestHelper.Assert(map["closing"].Current == 0 ||
                              map["closing"].Current == 1 ||
                              colocated && map["closing"].Current == 2);

            var controller = IControllerPrx.Parse(helper.GetTestProxy("controller", 1), communicator);
            var metricsWithHold = IMetricsPrx.Parse(helper.GetTestProxy("metrics", 2), communicator);

            props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
            UpdateProps(clientProps, serverProps, update, props, "Connection");
            _ = (await metrics.GetConnectionAsync()).GoAwayAsync();

            if (!colocated)
            {
                controller.Hold();
                try
                {
                    metricsWithHold.IcePing();
                    TestHelper.Assert(false);
                }
                catch (ConnectTimeoutException)
                {
                }
                controller.Resume();

                cm1 = (ConnectionMetrics)clientMetrics.GetMetricsView("View").ReturnValue["Connection"][0]!;
                while (true)
                {
                    sm1 = (ConnectionMetrics)serverMetrics.GetMetricsView("View").ReturnValue["Connection"][0]!;
                    if (sm1.Failures >= 2)
                    {
                        break;
                    }
                    Thread.Sleep(10);
                }
                TestHelper.Assert(cm1.Failures == 2 && sm1.Failures >= 2);

                CheckFailure(clientMetrics, "Connection", cm1.Id, "ZeroC.Ice.ConnectTimeoutException", 2, output);
                // The exception depends on the transport and might not necessarily be a connection lost exception so
                // we can't test the exception raised by the server side.
                // CheckFailure(serverMetrics, "Connection", sm1.Id, "ZeroC.Ice.ConnectionLostException", 0, output);
            }

            IMetricsPrx m = metrics.Clone(label: "Con1");
            m.IcePing();

            TestAttribute(clientMetrics, clientProps, update, "Connection", "parent", "Communicator", output);
            TestAttribute(clientMetrics,
                          clientProps,
                          update,
                          "Connection",
                          "endpoint",
                          colocated ?
                            (ice1 ? "colocated" : "ice+colocated://TestAdapter") :
                            (ice1 ? endpoint + " -t " + defaultTimeout : endpoint),
                          output);

            TestAttribute(clientMetrics, clientProps, update, "Connection", "endpointTransport", transportName, output);
            TestAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsDatagram", "False", output);
            TestAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsAlwaysSecure", isSecure, output);
            if (colocated)
            {
                TestAttribute(clientMetrics, clientProps, update, "Connection", "endpointHost", adapterName, output);
                TestAttribute(clientMetrics, clientProps, update, "Connection", "endpointPort", "0", output);
            }
            else
            {
                TestAttribute(clientMetrics, clientProps, update, "Connection", "endpointHost", host, output);
                TestAttribute(clientMetrics, clientProps, update, "Connection", "endpointPort", port, output);
            }
            TestAttribute(clientMetrics, clientProps, update, "Connection", "incoming", "False", output);
            TestAttribute(clientMetrics, clientProps, update, "Connection", "adapterName", "", output);
            TestAttribute(clientMetrics, clientProps, update, "Connection", "label", "Con1", output);
            if (!colocated)
            {
                TestAttribute(clientMetrics, clientProps, update, "Connection", "localHost", host, output);
                TestAttribute(clientMetrics, clientProps, update, "Connection", "remoteHost", host, output);
                TestAttribute(clientMetrics, clientProps, update, "Connection", "remotePort", port, output);
            }
            TestAttribute(clientMetrics, clientProps, update, "Connection", "mcastHost", "", output);
            TestAttribute(clientMetrics, clientProps, update, "Connection", "mcastPort", "", output);

            _ = (await m.GetConnectionAsync()).GoAwayAsync();

            WaitForCurrent(clientMetrics, "View", "Connection", 0);
            WaitForCurrent(serverMetrics, "View", "Connection", 0);

            output.WriteLine("ok");

            if (!colocated)
            {
                output.Write("testing connection establishment metrics... ");
                output.Flush();

                props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
                UpdateProps(clientProps, serverProps, update, props, "ConnectionEstablishment");
                TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"].Length == 0);

                metrics.IcePing();

                TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"].Length == 1);
                IceMX.Metrics? m1;
                m1 = clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"][0]!;
                TestHelper.Assert(m1.Current == 0 && m1.Total == 1 && m1.Id.Equals(hostAndPort));

                _ = (await metrics.GetConnectionAsync()).GoAwayAsync();

                ClearView(clientProps, serverProps, update);
                TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"].Length == 0);

                if (!colocated)
                {
                    controller.Hold();
                    try
                    {
                        IObjectPrx.Parse(helper.GetTestProxy("test", 2), communicator).IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (ConnectTimeoutException)
                    {
                    }
                    catch
                    {
                        TestHelper.Assert(false);
                    }
                    controller.Resume();

                    TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"].Length == 1);
                    m1 = clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"][0]!;
                    TestHelper.Assert(m1.Total == 2 && m1.Failures == 2);

                    CheckFailure(clientMetrics,
                                "ConnectionEstablishment",
                                m1.Id,
                                "ZeroC.Ice.ConnectTimeoutException",
                                2,
                                output);
                }
                controller.Resume();

                TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"].Length == 1);
                m1 = clientMetrics.GetMetricsView("View").ReturnValue["ConnectionEstablishment"][0]!;
                TestHelper.Assert(m1.Total == 2 && m1.Failures == 2);

                CheckFailure(clientMetrics, "ConnectionEstablishment", m1.Id, "ZeroC.Ice.ConnectTimeoutException", 2, output);

                Action c = () => Connect(metrics);
                TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "parent", "Communicator",
                    c, output);
                TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "id", hostAndPort,
                    c, output);
                if (ice1)
                {
                    TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpoint",
                        endpoint + " -t " + defaultTimeout, c, output);
                }
                else
                {
                    TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpoint",
                        endpoint, c, output);
                }

                TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointTransport",
                    transportName, c, output);
                TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsDatagram",
                    "False", c, output);
                TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsAlwaysSecure",
                    isSecure, c, output);
                TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointHost", host,
                    c, output);
                TestAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointPort", port,
                    c, output);

                output.WriteLine("ok");

                output.Write("testing endpoint lookup metrics... ");
                output.Flush();

                props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
                UpdateProps(clientProps, serverProps, update, props, "EndpointLookup");
                TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"].Length == 0);

                var prx = IObjectPrx.Parse(
                    ice1 ?
                        $"metrics:{transport} -h localhost -p {port}" :
                        $"ice+{transport}://localhost:{port}/metrics",
                    communicator);
                try
                {
                    prx.IcePing();
                    _ = (await prx.GetConnectionAsync()).GoAwayAsync();
                }
                catch
                {
                }

                TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"].Length == 1);
                m1 = clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"][0];
                TestHelper.Assert(m1 != null && m1.Current <= 1 && m1.Total == 1);

                bool dnsException = false;
                try
                {
                    if (ice1)
                    {
                        IObjectPrx.Parse($"test:tcp -h unknownfoo.zeroc.com -p {port} -t 500", communicator).IcePing();
                    }
                    else
                    {
                        IObjectPrx.Parse($"ice+tcp://unknownfoo.zeroc.com:{port}/test", communicator).IcePing();
                    }
                    TestHelper.Assert(false);
                }
                catch (DNSException)
                {
                    dnsException = true;
                }
                catch
                {
                    // Some DNS servers don't fail on unknown DNS names.
                    // TODO: what's the point of this test then?
                }
                TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"].Length == 2);
                m1 = clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"][0]!;

                if (ice1)
                {
                    if (!m1.Id.Equals($"tcp -h unknownfoo.zeroc.com -p {port} -t 500"))
                    {
                        m1 = clientMetrics.GetMetricsView("View").ReturnValue["EndpointLookup"][1]!;
                    }

                    TestHelper.Assert(m1.Id.Equals("tcp -h unknownfoo.zeroc.com -p " + port + " -t 500") &&
                        m1.Total == 1 && (!dnsException || m1.Failures == 1));
                    if (dnsException)
                    {
                        CheckFailure(clientMetrics, "EndpointLookup", m1.Id, "ZeroC.Ice.DNSException", 1, output);
                    }
                }
                // TODO: ice2 version

                c = () => Connect(prx);

                TestAttribute(clientMetrics, clientProps, update, "EndpointLookup", "parent", "Communicator", c, output);
                TestAttribute(clientMetrics, clientProps, update, "EndpointLookup", "id",
                            (await prx.GetConnectionAsync()).Endpoint.ToString(), c, output);
                TestAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpoint",
                            (await prx.GetConnectionAsync()).Endpoint.ToString(), c, output);

                TestAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointTransport", transportName,
                    c, output);
                TestAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsDatagram", "False",
                    c, output);
                TestAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsAlwaysSecure", isSecure,
                    c, output);
                TestAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointHost", "localhost",
                    c, output);
                TestAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointPort", port,
                    c, output);

                output.WriteLine("ok");
            }

            output.Write("testing dispatch metrics... ");
            output.Flush();

            props["IceMX.Metrics.View.Map.Dispatch.GroupBy"] = "operation";
            UpdateProps(clientProps, serverProps, update, props, "Dispatch");
            TestHelper.Assert(serverMetrics.GetMetricsView("View").ReturnValue["Dispatch"].Length == 0);

            metrics.Op();
            int userExErrorMessageSize = 0;
            try
            {
                metrics.OpWithUserException();
                TestHelper.Assert(false);
            }
            catch (UserEx ex)
            {
                userExErrorMessageSize = ex.Message.Length;
            }
            try
            {
                metrics.OpWithRequestFailedException();
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
            }
            try
            {
                metrics.OpWithLocalException();
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
            }
            try
            {
                metrics.OpWithUnknownException();
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
            }

            try
            {
                metrics.Fail();
                TestHelper.Assert(false);
            }
            catch (ConnectionLostException)
            {
            }

            map = ToMap(serverMetrics.GetMetricsView("View").ReturnValue["Dispatch"]!);
            TestHelper.Assert(map.Count == 6);

            DispatchMetrics dm1;
            dm1 = (DispatchMetrics)map["op"];
            TestHelper.Assert(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 0 && dm1.UserException == 0);

            // We measure the payload size here.
            if (ice1)
            {
                TestHelper.Assert(dm1.Size == 6 && dm1.ReplySize == 7);
            }
            else
            {
                TestHelper.Assert(dm1.Size == 4 && dm1.ReplySize == 5);
            }

            dm1 = (DispatchMetrics)map["opWithUserException"];
            TestHelper.Assert(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 0 && dm1.UserException == 1);

            // We assume the error message is encoded in ASCII (each character uses 1-byte when encoded in UTF-8).
            TestHelper.Assert(dm1.Size == (ice1 ? 6 : 4) &&
                dm1.ReplySize == (metrics.Encoding == Encoding.V11 ? 48 : 81 + userExErrorMessageSize));

            dm1 = (DispatchMetrics)map["opWithLocalException"];
            TestHelper.Assert(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 1 && dm1.UserException == 0);
            CheckFailure(serverMetrics, "Dispatch", dm1.Id, "ZeroC.Ice.InvalidConfigurationException", 1, output);

            // Reply contains the exception stack depending on the OS.
            TestHelper.Assert(dm1.Size == (ice1 ? 6 : 4) && dm1.ReplySize > 7);
            dm1 = (DispatchMetrics)map["opWithRequestFailedException"];
            TestHelper.Assert(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 0 && dm1.UserException == 1);
            if (ice1)
            {
                TestHelper.Assert(dm1.Size == 6 && dm1.ReplySize == 40);
            }
            else
            {
                // We marshal the full ONE.
                TestHelper.Assert(dm1.Size == 4 && dm1.ReplySize == 203);
            }

            dm1 = (DispatchMetrics)map["opWithUnknownException"];
            TestHelper.Assert(dm1.Current <= 1 && dm1.Total == 1 && dm1.Failures == 1 && dm1.UserException == 0);
            CheckFailure(serverMetrics, "Dispatch", dm1.Id, "System.ArgumentOutOfRangeException", 1, output);
            TestHelper.Assert(dm1.Size == (ice1 ? 6 : 4) && dm1.ReplySize > 7); // Reply contains the exception stack.

            Action op = () => InvokeOp(metrics);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "parent", "TestAdapter", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "id", "metrics [op]", op, output);

            string expectedEndpoint = colocated ?
                (ice1 ? "colocated" : "ice+colocated://TestAdapter") :
                (ice1 ? endpoint + " -t 60000" : endpoint);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "endpoint", expectedEndpoint, op, output);

            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointTransport", transportName, op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsDatagram", "False", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsAlwaysSecure", isSecure, op, output);
            if (colocated)
            {
                TestAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointHost", adapterName, op, output);
                TestAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointPort", "0", op, output);
            }
            else
            {
                TestAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointHost", host, op, output);
                TestAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointPort", port, op, output);
            }

            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "incoming", "True", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "adapterName", "TestAdapter", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "label", "", op, output);
            if (!colocated)
            {
                TestAttribute(serverMetrics, serverProps, update, "Dispatch", "localHost", host, op, output);
                TestAttribute(serverMetrics, serverProps, update, "Dispatch", "localPort", port, op, output);
                TestAttribute(serverMetrics, serverProps, update, "Dispatch", "remoteHost", host, op, output);
            }
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastHost", "", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastPort", "", op, output);

            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "operation", "op", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "identity", "metrics", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "facet", "", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "mode", "twoway", op, output);

            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry1", "test", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry2", "", op, output);
            TestAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry3", "", op, output);

            output.WriteLine("ok");

            output.Write("testing invocation metrics... ");
            output.Flush();

            // Tests for twoway
            props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
            props["IceMX.Metrics.View.Map.Invocation.Map.ChildInvocation.GroupBy"] = "id";
            UpdateProps(clientProps, serverProps, update, props, "Invocation");
            TestHelper.Assert(serverMetrics.GetMetricsView("View").ReturnValue["Invocation"].Length == 0);

            metrics.Op();
            metrics.OpAsync().Wait();

            try
            {
                metrics.OpWithUserException();
                TestHelper.Assert(false);
            }
            catch (UserEx)
            {
            }

            try
            {
                metrics.OpWithUserExceptionAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is UserEx);
            }

            try
            {
                metrics.OpWithRequestFailedException();
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
            }

            try
            {
                metrics.OpWithRequestFailedExceptionAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is ObjectNotExistException);
            }

            try
            {
                metrics.OpWithLocalException();
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
            }
            try
            {
                metrics.OpWithLocalExceptionAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is UnhandledException);
            }

            try
            {
                metrics.OpWithUnknownException();
                TestHelper.Assert(false);
            }
            catch (UnhandledException)
            {
            }

            try
            {
                metrics.OpWithUnknownExceptionAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is UnhandledException);
            }

            try
            {
                metrics.Fail();
                TestHelper.Assert(false);
            }
            catch (ConnectionLostException)
            {
            }

            try
            {
                metrics.FailAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ex)
            {
                TestHelper.Assert(ex.InnerException is ConnectionLostException);
            }

            map = ToMap(clientMetrics.GetMetricsView("View").ReturnValue["Invocation"]!);
            TestHelper.Assert(map.Count == 6);

            InvocationMetrics im1;
            ChildInvocationMetrics rim1;
            im1 = (InvocationMetrics)map["op"];
            TestHelper.Assert(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 0 && im1.Retry == 0);
            TestHelper.Assert(im1.Children.Length == 1);
            rim1 = (ChildInvocationMetrics)im1.Children[0]!;
            TestHelper.Assert(rim1.Current == 0 && rim1.Total == 2 && rim1.Failures == 0);

            TestHelper.Assert(rim1.Size == (ice1 ? 12 : 8) && rim1.ReplySize == (ice1 ? 14 : 10));

            im1 = (InvocationMetrics)map["opWithUserException"];
            TestHelper.Assert(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 2 && im1.Retry == 0);
            TestHelper.Assert(im1.Children.Length == 1);
            rim1 = (ChildInvocationMetrics)im1.Children[0]!;
            TestHelper.Assert(rim1.Current == 0 && rim1.Total == 2 && rim1.Failures == 0);
            TestHelper.Assert(rim1.Size == (ice1 ? 12 : 8) && rim1.ReplySize > 7);
            TestHelper.Assert(im1.UserException == 2);

            im1 = (InvocationMetrics)map["opWithLocalException"];
            TestHelper.Assert(im1.Current <= 1 && im1.Total == 2 && im1.Retry == 0);
            // Local exceptions raised by the servant are reported as remote exceptions only with ice2, both as
            // a failure and remote exceptions with ice1.
            TestHelper.Assert(im1.Failures == 2 && im1.UserException == 2);
            TestHelper.Assert(im1.Children.Length == 1);
            rim1 = (ChildInvocationMetrics)im1.Children[0]!;
            TestHelper.Assert(rim1.Current <= 1 && rim1.Total == 2 && rim1.Failures == 0);
            TestHelper.Assert(rim1.Size == (ice1 ? 12 : 8) && rim1.ReplySize > 7);
            // TODO: observers needs fixing to report a better exception than System.Exception
            CheckFailure(clientMetrics, "Invocation", im1.Id, "System.Exception", 2, output);

            im1 = (InvocationMetrics)map["opWithRequestFailedException"];
            TestHelper.Assert(im1.Current <= 1 && im1.Total == 2 && im1.Retry == 0);
            // System exceptions raised by the servant are reported as remote exceptions only with ice2, both as
            // a failure and remote exceptions with ice1.
            TestHelper.Assert(im1.Failures == 2 && im1.UserException == 2);
            TestHelper.Assert(im1.Children.Length == 1);
            rim1 = (ChildInvocationMetrics)im1.Children[0]!;
            TestHelper.Assert(rim1.Current <= 1 && rim1.Total == 2 && rim1.Failures == 0);
            TestHelper.Assert(rim1.Size == (ice1 ? 12 : 8) && rim1.ReplySize > 7);

            // TODO: observers needs fixing to report a better exception than System.Exception
            CheckFailure(clientMetrics, "Invocation", im1.Id, "System.Exception", 2, output);

            im1 = (InvocationMetrics)map["opWithUnknownException"];
            TestHelper.Assert(im1.Current <= 1 && im1.Total == 2 && im1.Retry == 0);
            // Local exceptions raised by the servant are reported as remote exceptions only with ice2, both as
            // a failure and remote exceptions with ice1.
            TestHelper.Assert(im1.Failures == 2 && im1.UserException == 2);
            TestHelper.Assert(im1.Children.Length == 1);
            rim1 = (ChildInvocationMetrics)im1.Children[0]!;
            TestHelper.Assert(rim1.Current <= 1 && rim1.Total == 2 && rim1.Failures == 0);
            TestHelper.Assert(rim1.Size == (ice1 ? 12 : 8) && rim1.ReplySize > 7);
            // TODO: observers needs fixing to report a better exception than System.Exception
            CheckFailure(clientMetrics, "Invocation", im1.Id, "System.Exception", 2, output);

            im1 = (InvocationMetrics)map["fail"];
            TestHelper.Assert(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 2 && im1.Retry == 2,
                $"Current: {im1.Current} Total: {im1.Total} Failures: {im1.Failures} Retry: {im1.Retry}");
            TestHelper.Assert(im1.Children.Length == 1);
            rim1 = (ChildInvocationMetrics)im1.Children[0]!;
            TestHelper.Assert(rim1.Current == 0);
            TestHelper.Assert(rim1.Total == 4);
            TestHelper.Assert(rim1.Failures == 4);
            CheckFailure(clientMetrics, "Invocation", im1.Id, "ZeroC.Ice.ConnectionLostException", 2, output);

            Encoding defaultEncoding = helper.Encoding;
            string defaultProtocolName = helper.Protocol.GetName();

            TestAttribute(clientMetrics, clientProps, update, "Invocation", "parent", "Communicator", op, output);
            TestAttribute(
                clientMetrics,
                clientProps,
                update,
                "Invocation",
                "id",
                ice1 ? $"metrics -t -e {defaultEncoding} [op]" : "ice:metrics [op]",
                op,
                output);

            TestAttribute(clientMetrics, clientProps, update, "Invocation", "operation", "op", op, output);
            TestAttribute(clientMetrics, clientProps, update, "Invocation", "identity", "metrics", op, output);
            TestAttribute(clientMetrics, clientProps, update, "Invocation", "facet", "", op, output);
            TestAttribute(clientMetrics, clientProps, update, "Invocation", "encoding", $"{defaultEncoding}", op, output);
            TestAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "twoway", op, output);

            TestAttribute(
                clientMetrics,
                clientProps,
                update,
                "Invocation",
                "proxy",
                ice1 ? $"metrics -t -e {defaultEncoding}:{endpoint} -t {defaultTimeout}" : $"{endpoint}/metrics",
                op,
                output);

            TestAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry1", "test", op, output);
            TestAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry2", "", op, output);
            TestAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry3", "", op, output);

            // Oneway tests
            ClearView(clientProps, serverProps, update);
            props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
            props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
            UpdateProps(clientProps, serverProps, update, props, "Invocation");

            IMetricsPrx metricsOneway = metrics.Clone(oneway: true);
            metricsOneway.Op();
            metricsOneway.OpAsync().Wait();

            map = ToMap(clientMetrics.GetMetricsView("View").ReturnValue["Invocation"]!);
            TestHelper.Assert(map.Count == 1);

            im1 = (InvocationMetrics)map["op"];
            TestHelper.Assert(im1.Current <= 1 && im1.Total == 2 && im1.Failures == 0 && im1.Retry == 0);
            TestHelper.Assert(im1.Children.Length == 1);
            rim1 = (ChildInvocationMetrics)im1.Children[0]!;
            TestHelper.Assert(rim1.Current <= 1 && rim1.Total == 2 && rim1.Failures == 0);
            TestHelper.Assert(rim1.Size == (ice1 ? 12 : 8) && rim1.ReplySize == 0);

            TestAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "oneway",
                        () => InvokeOp(metricsOneway), output);
            output.WriteLine("ok");

            output.Write("testing metrics view enable/disable...");
            output.Flush();

            props["IceMX.Metrics.View.GroupBy"] = "none";
            props["IceMX.Metrics.View.Disabled"] = "0";
            UpdateProps(clientProps, serverProps, update, props, "Connection");
            TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["Connection"].Length != 0);
            (string[] names, string[] disabledViews) = clientMetrics.GetMetricsViewNames();
            TestHelper.Assert(names.Length == 1 && disabledViews.Length == 0);

            props["IceMX.Metrics.View.Disabled"] = "1";
            UpdateProps(clientProps, serverProps, update, props, "Connection");
            TestHelper.Assert(!clientMetrics.GetMetricsView("View").ReturnValue.ContainsKey("Connection"));
            (names, disabledViews) = clientMetrics.GetMetricsViewNames();
            TestHelper.Assert(names.Length == 0 && disabledViews.Length == 1);

            clientMetrics.EnableMetricsView("View");
            TestHelper.Assert(clientMetrics.GetMetricsView("View").ReturnValue["Connection"].Length != 0);
            (names, disabledViews) = clientMetrics.GetMetricsViewNames();
            TestHelper.Assert(names.Length == 1 && disabledViews.Length == 0);

            clientMetrics.DisableMetricsView("View");
            TestHelper.Assert(!clientMetrics.GetMetricsView("View").ReturnValue.ContainsKey("Connection"));
            (names, disabledViews) = clientMetrics.GetMetricsViewNames();
            TestHelper.Assert(names.Length == 0 && disabledViews.Length == 1);

            try
            {
                clientMetrics.EnableMetricsView("UnknownView");
            }
            catch (UnknownMetricsView)
            {
            }

            output.WriteLine("ok");

            output.Write("testing instrumentation observer delegate... ");
            output.Flush();

            TestHelper.Assert(obsv.ConnectionObserver!.GetTotal() > 0);
            TestHelper.Assert(obsv.ConnectionEstablishmentObserver!.GetTotal() > 0);
            if (!colocated)
            {
                TestHelper.Assert(obsv.EndpointLookupObserver!.GetTotal() > 0);
            }
            TestHelper.Assert(obsv.InvocationObserver!.ChildInvocationObserver!.GetTotal() > 0);
            TestHelper.Assert(obsv.InvocationObserver!.GetTotal() > 0);
            TestHelper.Assert(obsv.DispatchObserver!.GetTotal() > 0);

            TestHelper.Assert(obsv.ConnectionObserver!.GetCurrent() > 0);
            TestHelper.Assert(obsv.ConnectionEstablishmentObserver!.GetCurrent() == 0);
            if (!colocated)
            {
                TestHelper.Assert(obsv.EndpointLookupObserver!.GetCurrent() == 0);
            }
            WaitForObserverCurrent(obsv.InvocationObserver!.ChildInvocationObserver!);
            TestHelper.Assert(obsv.InvocationObserver!.ChildInvocationObserver!.GetCurrent() == 0);

            WaitForObserverCurrent(obsv.InvocationObserver);
            TestHelper.Assert(obsv.InvocationObserver.GetCurrent() == 0);
            WaitForObserverCurrent(obsv.DispatchObserver);
            TestHelper.Assert(obsv.DispatchObserver.GetCurrent() == 0);

            TestHelper.Assert(obsv.ConnectionObserver!.GetFailedCount() > 0);
            if (!colocated)
            {
                TestHelper.Assert(obsv.ConnectionEstablishmentObserver!.GetFailedCount() > 0);
                TestHelper.Assert(obsv.EndpointLookupObserver!.GetFailedCount() > 0);
            }
            TestHelper.Assert(obsv.InvocationObserver!.ChildInvocationObserver!.GetFailedCount() > 0);
            TestHelper.Assert(obsv.InvocationObserver.GetFailedCount() > 0);
            if (colocated)
            {
                TestHelper.Assert(obsv.DispatchObserver.GetFailedCount() > 0);
            }

            if (!colocated)
            {
                TestHelper.Assert(obsv.ConnectionObserver!.Received > 0 && obsv.ConnectionObserver!.Sent > 0);
                TestHelper.Assert(obsv.InvocationObserver!.ChildInvocationObserver!.ReplySize > 0);
            }
            TestHelper.Assert(obsv.InvocationObserver!.RetriedCount > 0);
            TestHelper.Assert(obsv.InvocationObserver.UserExceptionCount > 0);
            if (colocated)
            {
                TestHelper.Assert(obsv.DispatchObserver.UserExceptionCount > 0);
            }

            output.WriteLine("ok");
            await metrics.ShutdownAsync();
        }
    }
}
