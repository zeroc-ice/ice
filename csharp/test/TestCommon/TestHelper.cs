// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using ZeroC.Ice;
using ZeroC.Ice.Instrumentation;

// The new version of the Test helper classes. Will be renamed Test once all the tests have migrated to TestNew.
namespace ZeroC.Test
{
    public abstract class TestHelper
    {
        private TextWriter? _writer;

        public ushort BasePort => GetTestBasePort(Communicator.GetProperties());

        public Communicator Communicator { get; private init; } = null!;

        public Ice.Encoding Encoding => GetTestEncoding(Communicator.GetProperties());

        public string Host => GetTestHost(Communicator.GetProperties());

        public Protocol Protocol => GetTestProtocol(Communicator.GetProperties());

        public string Transport => GetTestTransport(Communicator.GetProperties());

        public TextWriter Output
        {
            get => _writer ?? Console.Out;
            set => _writer = value;
        }

        static TestHelper()
        {
            // Replace the default trace listener that is responsible of displaying the retry/abort dialog
            // with our custom trace listener that always aborts upon failure.
            // see: https://docs.microsoft.com/en-us/dotnet/api/system.diagnostics.defaulttracelistener?view=net-5.0#remarks
            Trace.Listeners.Clear();
            Trace.Listeners.Add(new TestTraceListener());
        }

        public static void Assert([DoesNotReturnIf(false)] bool b, string message = "")
        {
            if (!b)
            {
                Fail(message, null);
            }
        }

        [DoesNotReturn]
        internal static void Fail(string? message, string? detailMessage)
        {
            var sb = new StringBuilder();
            sb.Append("failed:\n");
            if (message != null && message.Length > 0)
            {
                sb.Append("message: ").Append(message).Append('\n');
            }
            if (detailMessage != null && detailMessage.Length > 0)
            {
                sb.Append("details: ").Append(detailMessage).Append('\n');
            }
            try
            {
                sb.Append(new StackTrace(fNeedFileInfo: true).ToString()).Append('\n');
            }
            catch
            {
            }

            Console.WriteLine(sb.ToString());
            Environment.Exit(1);
        }

        public static Communicator CreateCommunicator(
            ref string[] args,
            Dictionary<string, string>? defaults = null,
            ICommunicatorObserver? observer = null) =>
            CreateCommunicator(CreateTestProperties(ref args, defaults), observer);

        public static Communicator CreateCommunicator(
            Dictionary<string, string> properties,
            ICommunicatorObserver? observer = null)
        {
            var tlsServerOptions = new TlsServerOptions();
            if (properties.TryGetValue("Test.Transport", out string? value))
            {
                // When running test with WSS, disable client authentication for browser compatibility
                tlsServerOptions.RequireClientCertificate = value == "ssl";
            }

            return new Communicator(properties, tlsServerOptions: tlsServerOptions, observer: observer);
        }

        public string GetTestEndpoint(
            int num = 0,
            string transport = "",
            bool ephemeral = false) =>
            GetTestEndpoint(Communicator.GetProperties(), num, transport, ephemeral);

        public static string GetTestEndpoint(
            Dictionary<string, string> properties,
            int num = 0,
            string transport = "",
            bool ephemeral = false)
        {
            if (transport.Length == 0)
            {
                transport = GetTestTransport(properties);
            }

            string host = GetTestHost(properties);
            string port = ephemeral ? "0" : $"{GetTestBasePort(properties) + num}";

            if (GetTestProtocol(properties) == Protocol.Ice2 && transport != "udp")
            {
                var sb = new StringBuilder("ice+");
                sb.Append(transport);
                sb.Append("://");

                if (host.Contains(':'))
                {
                    sb.Append('[');
                    sb.Append(host);
                    sb.Append(']');
                }
                else
                {
                    sb.Append(host);
                }
                sb.Append(':');
                sb.Append(port);
                return sb.ToString();
            }
            else
            {
                var sb = new StringBuilder(transport);
                sb.Append(" -h ");
                if (host.Contains(':'))
                {
                    sb.Append('"');
                    sb.Append(host);
                    sb.Append('"');
                }
                else
                {
                    sb.Append(host);
                }
                sb.Append(" -p ");
                sb.Append(port);
                return sb.ToString();
            }
        }

        public static string GetTestProxy(
            string identity,
            Dictionary<string, string> properties,
            int num = 0,
            string? transport = null)
        {
            if (GetTestProtocol(properties) == Protocol.Ice2 && transport != "udp")
            {
                var sb = new StringBuilder("ice+");
                sb.Append(transport ?? GetTestTransport(properties));
                sb.Append("://");
                string host = GetTestHost(properties);
                if (host.Contains(':'))
                {
                    sb.Append('[');
                    sb.Append(host);
                    sb.Append(']');
                }
                else
                {
                    sb.Append(host);
                }
                sb.Append(':');
                sb.Append(GetTestBasePort(properties) + num);
                sb.Append('/');
                sb.Append(identity);
                return sb.ToString();
            }
            else // i.e. ice1
            {
                var sb = new StringBuilder(identity);
                sb.Append(':');
                sb.Append(transport ?? GetTestTransport(properties));
                sb.Append(" -h ");
                string host = GetTestHost(properties);
                if (host.Contains(':'))
                {
                    sb.Append('"');
                    sb.Append(host);
                    sb.Append('"');
                }
                else
                {
                    sb.Append(host);
                }
                sb.Append(" -p ");
                sb.Append(GetTestBasePort(properties) + num);
                return sb.ToString();
            }
        }

        public static string GetTestHost(Dictionary<string, string> properties)
        {
            if (!properties.TryGetValue("Test.Host", out string? host))
            {
                host = "127.0.0.1";
            }
            return host;
        }

        public static Protocol GetTestProtocol(Dictionary<string, string> properties)
        {
            if (!properties.TryGetValue("Test.Protocol", out string? value))
            {
                return Protocol.Ice2;
            }
            try
            {
                return ProtocolExtensions.Parse(value);
            }
            catch (Exception ex)
            {
                throw new InvalidConfigurationException($"invalid value for for Test.Protocol: `{value}'", ex);
            }
        }

        public static Ice.Encoding GetTestEncoding(Dictionary<string, string> properties) =>
            GetTestProtocol(properties).GetEncoding();

        public static string GetTestTransport(Dictionary<string, string> properties)
        {
            if (properties.TryGetValue("Test.Transport", out string? transport))
            {
                if (GetTestProtocol(properties) == Protocol.Ice1)
                {
                    return transport;
                }
                return transport switch
                {
                    "wss" => "ws",
                    "ssl" => "tcp",
                    _ => transport,
                };
            }
            else
            {
                return "tcp";
            }
        }

        public static ushort GetTestBasePort(Dictionary<string, string> properties)
        {
            ushort basePort = 12010;
            if (properties.TryGetValue("Test.BasePort", out string? value))
            {
                basePort = ushort.Parse(value);
            }
            return basePort;
        }

        public static Dictionary<string, string> CreateTestProperties(
            ref string[] args,
            Dictionary<string, string>? defaults = null)
        {
            var properties =
                defaults == null ? new Dictionary<string, string>() : new Dictionary<string, string>(defaults);
            properties.ParseIceArgs(ref args);
            properties.ParseArgs(ref args, "Test");
            return properties;
        }

        public string GetTestProxy(string identity, int num = 0, string? transport = null) =>
            GetTestProxy(identity, Communicator.GetProperties(), num, transport);

        public abstract Task RunAsync(string[] args);

        public static async Task<int> RunTestAsync<T>(Communicator communicator, string[] args)
            where T : TestHelper, new()
        {
            try
            {
                var helper = new T() { Communicator = communicator };
                await helper.RunAsync(args);
                return 0;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return 1;
            }
        }

        public virtual void ServerReady()
        {
        }

        // A custom trace listener that always aborts the application upon failure.
        internal class TestTraceListener : DefaultTraceListener
        {
            public override void Fail(string? message) => TestHelper.Fail(message, null);

            public override void Fail(string? message, string? detailMessage) => TestHelper.Fail(message, detailMessage);
        }
    }
}
