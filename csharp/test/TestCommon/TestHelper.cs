// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace Test
{
    public interface IControllerHelper
    {
        void ServerReady();
        void CommunicatorInitialized(Communicator communicator);
    }

    public interface IPlatformAdapter
    {
        bool IsEmulator();

        string ProcessControllerRegistryHost();

        string ProcessControllerIdentity();
    }

    public abstract class TestHelper
    {
        private TextWriter? _writer;

        public ushort BasePort => GetTestBasePort(Communicator!.GetProperties());

        public IControllerHelper? ControllerHelper { get; set; }
        public Communicator? Communicator { get; private set; }

        public ZeroC.Ice.Encoding Encoding => GetTestEncoding(Communicator!.GetProperties());

        public string Host => GetTestHost(Communicator!.GetProperties());

        public Protocol Protocol => GetTestProtocol(Communicator!.GetProperties());

        public string Transport => GetTestTransport(Communicator!.GetProperties());

        public TextWriter Output
        {
            get => _writer ?? Console.Out;
            set => _writer = value;
        }

        public static void Assert([DoesNotReturnIf(false)] bool b, string message = "")
        {
            if (!b)
            {
                Debug.Assert(false, message);
                throw new Exception(message);
            }
        }
        public abstract Task RunAsync(string[] args);

        public string GetTestEndpoint(int num = 0, string transport = "", bool ephemeral = false) =>
            GetTestEndpoint(Communicator!.GetProperties(), num, transport, ephemeral);

        public static string GetTestEndpoint(
            Dictionary<string, string> properties, int num = 0, string transport = "", bool ephemeral = false)
        {
            if (transport.Length == 0 || transport == "default")
            {
                if (properties.TryGetValue("Test.Transport", out string? value))
                {
                    transport = value;
                }
                else
                {
                    transport = "tcp";
                }
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

        public string GetTestProxy(string identity, int num = 0, string? transport = null) =>
            GetTestProxy(identity, Communicator!.GetProperties(), num, transport);

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
                throw new InvalidConfigurationException(
                    $"invalid value for for Test.Protocol: `{value}'", ex);
            }
        }

        public static ZeroC.Ice.Encoding GetTestEncoding(Dictionary<string, string> properties)
            => ProtocolExtensions.GetEncoding(GetTestProtocol(properties));

        public static string GetTestTransport(Dictionary<string, string> properties)
        {
            if (!properties.TryGetValue("Test.Transport", out string? transport))
            {
                transport = "tcp";
            }
            return transport;
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
            Dictionary<string, string> properties =
                defaults == null ? new Dictionary<string, string>() : new Dictionary<string, string>(defaults);
            properties.ParseIceArgs(ref args);
            properties.ParseArgs(ref args, "Test");
            return properties;
        }

        public Communicator Initialize(ref string[] args,
            Dictionary<string, string>? defaults = null,
            ZeroC.Ice.Instrumentation.ICommunicatorObserver? observer = null) =>
            Initialize(CreateTestProperties(ref args, defaults), observer);

        public Communicator Initialize(
            Dictionary<string, string> properties,
            ZeroC.Ice.Instrumentation.ICommunicatorObserver? observer = null)
        {
            var tlsServerOptions = new TlsServerOptions();
            if (properties.TryGetValue("Test.Transport", out string? value))
            {
                // When running test with WSS disable client authentication for browser compatibility
                tlsServerOptions.RequireClientCertificate = value == "ssl";
            }
            var communicator = new Communicator(properties, tlsServerOptions: tlsServerOptions, observer: observer);

            Communicator ??= communicator;
            ControllerHelper?.CommunicatorInitialized(communicator);

            return communicator;
        }

        public void ServerReady() => ControllerHelper?.ServerReady();
    }

    public static class TestDriver
    {
        public static async Task<int> RunTestAsync<T>(string[] args) where T : TestHelper, new()
        {
            int status = 0;
            try
            {
                var h = new T();
                await h.RunAsync(args);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                status = 1;
            }
            return status;
        }
    }
}
