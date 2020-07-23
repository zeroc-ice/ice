//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        public static void Assert([DoesNotReturnIf(false)] bool b)
        {
            if (!b)
            {
                Debug.Assert(false);
                throw new Exception();
            }
        }
        public abstract Task RunAsync(string[] args);

        public string GetTestEndpoint(int num = 0, string transport = "") =>
            GetTestEndpoint(_communicator!.GetProperties(), num, transport);

        public static string GetTestEndpoint(Dictionary<string, string> properties, int num = 0, string transport = "")
        {
            if (transport.Length == 0 || transport == "default")
            {
                if (properties.TryGetValue("Ice.Default.Transport", out string? value))
                {
                    transport = value;
                }
                else
                {
                    transport = "tcp";
                }
            }

            bool uriFormat = true;
            // TODO: switch to a different test-only property to select the protocol
            if (properties.TryGetValue("Ice.Default.Protocol", out string? protocolValue))
            {
                if (protocolValue == "ice1")
                {
                    uriFormat = false;
                }
            }

            if (uriFormat)
            {
                var sb = new StringBuilder("ice+");
                sb.Append(transport);
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
                sb.Append(GetTestPort(properties, num));
                return sb.ToString();
            }
            else
            {
                var sb = new StringBuilder(transport);
                sb.Append(" -p ");
                sb.Append(GetTestPort(properties, num));
                return sb.ToString();
            }
        }

        public string GetTestProxy(string identity, int num = 0, string? transport = null) =>
            GetTestProxy(identity, _communicator!.GetProperties(), num, transport);

        public static string GetTestProxy(
            string identity,
            Dictionary<string, string> properties,
            int num = 0,
            string? transport = null)
        {
            bool uriFormat = true;
            // TODO: switch to a different test-only property to select the protocol
            if (properties.TryGetValue("Ice.Default.Protocol", out string? protocolValue))
            {
                if (protocolValue == "ice1")
                {
                    uriFormat = false;
                }
            }

            if (uriFormat) // i.e. ice2
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
                sb.Append(GetTestPort(properties, num));
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
                sb.Append(GetTestPort(properties, num));
                return sb.ToString();
            }
        }
        public string GetTestHost() => GetTestHost(_communicator!.GetProperties());

        public static string GetTestHost(Dictionary<string, string> properties)
        {
            if (!properties.TryGetValue("Ice.Default.Host", out string? host))
            {
                host = "127.0.0.1";
            }
            return host;
        }

        public string GetTestTransport() => GetTestTransport(_communicator!.GetProperties());

        public static string GetTestTransport(Dictionary<string, string> properties)
        {
            // TODO: switch to a different test-only property to select the transport
            if (!properties.TryGetValue("Ice.Default.Transport", out string? transport))
            {
                transport = "tcp";
            }
            return transport;
        }

        public int GetTestPort(int num) => GetTestPort(_communicator!.GetProperties(), num);

        // TODO: switch to ushort
        public static int GetTestPort(Dictionary<string, string> properties, int num)
        {
            int basePort = 12010;
            if (properties.TryGetValue("Test.BasePort", out string? value))
            {
                basePort = int.Parse(value);
            }
            return basePort + num;
        }

        public TextWriter GetWriter()
        {
            if (_writer == null)
            {
                return Console.Out;
            }
            else
            {
                return _writer;
            }
        }

        public void SetWriter(TextWriter writer) => _writer = writer;

        public Dictionary<string, string> CreateTestProperties(
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
            var communicator = new Communicator(properties, observer: observer);
            if (_communicator == null)
            {
                _communicator = communicator;
            }
            if (_controllerHelper != null)
            {
                _controllerHelper.CommunicatorInitialized(communicator);
            }
            return communicator;
        }

        public Communicator? Communicator() => _communicator;
        public void SetControllerHelper(IControllerHelper controllerHelper) => _controllerHelper = controllerHelper;

        public void ServerReady()
        {
            if (_controllerHelper != null)
            {
                _controllerHelper.ServerReady();
            }
        }

        private Communicator? _communicator;
        private IControllerHelper? _controllerHelper;
        private TextWriter? _writer;
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
