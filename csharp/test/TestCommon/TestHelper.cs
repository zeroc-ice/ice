//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;

using Ice;

namespace Test
{
    public interface ControllerHelper
    {
        void serverReady();
        void communicatorInitialized(Ice.Communicator communicator);
    }

    public interface PlatformAdapter
    {
        bool isEmulator();

        string processControllerRegistryHost();

        string processControllerIdentity();
    }

    public abstract class TestHelper
    {
        public abstract void run(string[] args);

        public string getTestEndpoint(int num = 0, string protocol = "")
        {
            return getTestEndpoint(_communicator.GetProperties(), num, protocol);
        }

        static public string getTestEndpoint(Dictionary<string, string> properties, int num = 0, string protocol = "")
        {
            string? value;
            if (protocol == "")
            {
                if (!properties.TryGetValue("Ice.Default.Protocol", out value))
                {
                    value = "default";
                }
            }
            else
            {
                value = protocol;
            }

            StringBuilder sb = new StringBuilder();
            sb.Append(value);
            sb.Append(" -p ");
            int basePort;
            if (!properties.TryGetValue("Test.BasePort", out value) || !int.TryParse(value, out basePort))
            {
                basePort = 12010;
            }
            sb.Append(basePort + num);
            return sb.ToString();
        }

        public string getTestHost()
        {
            return getTestHost(_communicator.GetProperties());
        }

        static public string getTestHost(Dictionary<string, string> properties)
        {
            string? host;
            if (!properties.TryGetValue("Ice.Default.Host", out host))
            {
                host = "127.0.0.1";
            }
            return host;
        }

        public string getTestProtocol()
        {
            return getTestProtocol(_communicator.GetProperties());
        }

        static public String getTestProtocol(Dictionary<string, string> properties)
        {
            string? protocol;
            if (!properties.TryGetValue("Ice.Default.Protocol", out protocol))
            {
                protocol = "tcp";
            }
            return protocol;
        }

        public int getTestPort(int num)
        {
            return getTestPort(_communicator.GetProperties(), num);
        }

        static public int getTestPort(Dictionary<string, string> properties, int num)
        {
            string? value;
            int basePort = 12010;
            if (properties.TryGetValue("Test.BasePort", out value))
            {
                basePort = int.Parse(value);
            }
            return basePort + num;
        }

        public TextWriter getWriter()
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

        public void setWriter(TextWriter writer)
        {
            _writer = writer;
        }

        public Dictionary<string, string> createTestProperties(
            ref string[] args,
            Dictionary<string, string>? defaults = null)
        {
            var properties = defaults == null ? new Dictionary<string, string>() : new Dictionary<string, string>(defaults);
            properties.ParseIceArgs(ref args);
            properties.ParseArgs(ref args, "Test");
            return properties;
        }

        public Communicator initialize(ref string[] args,
            Dictionary<string, string>? defaults = null,
            Action<Action, Connection?>? dispatcher = null,
            Ice.Instrumentation.CommunicatorObserver? observer = null,
            string[]? typeIdNamespaces = null)
        {
            return initialize(createTestProperties(ref args, defaults), dispatcher, observer, typeIdNamespaces);
        }

        public Communicator initialize(
            Dictionary<string, string> properties,
            Action<Action, Connection?>? dispatcher = null,
            Ice.Instrumentation.CommunicatorObserver? observer = null,
            string[]? typeIdNamespaces = null)
        {
            var communicator = new Communicator(properties, dispatcher: dispatcher, observer: observer, typeIdNamespaces: typeIdNamespaces);
            if (_communicator == null)
            {
                _communicator = communicator;
            }
            if (_controllerHelper != null)
            {
                _controllerHelper.communicatorInitialized(communicator);
            }
            return communicator;
        }

        public Ice.Communicator communicator()
        {
            return _communicator;
        }

        public static void test(bool b)
        {
            if (!b)
            {
                Debug.Assert(false);
                throw new System.Exception();
            }
        }

        public void setControllerHelper(ControllerHelper controllerHelper)
        {
            _controllerHelper = controllerHelper;
        }

        public void serverReady()
        {
            if (_controllerHelper != null)
            {
                _controllerHelper.serverReady();
            }
        }

        private Ice.Communicator _communicator;
        private ControllerHelper _controllerHelper;
        private TextWriter _writer;
    }

    public abstract class AllTests
    {
        protected static void test(bool b)
        {
            if (!b)
            {
                Debug.Assert(false);
                throw new System.Exception();
            }
        }
    }

    public static class TestDriver
    {
        public static int runTest<T>(string[] args)
            where T : TestHelper, new()
        {
            int status = 0;
            try
            {
                T h = new T();
                h.run(args);
            }
            catch (System.Exception ex)
            {
                Console.WriteLine(ex);
                status = 1;
            }
            return status;
        }
    }
}
