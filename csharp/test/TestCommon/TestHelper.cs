//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Text;
using System.IO;

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
            return getTestEndpoint(_communicator.getProperties(), num, protocol);
        }

        static public string getTestEndpoint(Ice.Properties properties, int num = 0, string protocol = "")
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(protocol == "" ? properties.getPropertyWithDefault("Ice.Default.Protocol", "default") :
                                       protocol);
            sb.Append(" -p ");
            sb.Append(properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num);
            return sb.ToString();
        }

        public string getTestHost()
        {
            return getTestHost(_communicator.getProperties());
        }

        static public string getTestHost(Ice.Properties properties)
        {
            return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
        }

        public String getTestProtocol()
        {
            return getTestProtocol(_communicator.getProperties());
        }

        static public String getTestProtocol(Ice.Properties properties)
        {
            return properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        }

        public int getTestPort(int num)
        {
            return getTestPort(_communicator.getProperties(), num);
        }

        static public int getTestPort(Ice.Properties properties, int num)
        {
            return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;
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

        public Ice.Properties createTestProperties(ref string[] args)
        {
            Ice.Properties properties = Ice.Util.createProperties(ref args);
            args = properties.parseCommandLineOptions("Test", args);
            return properties;
        }

        public Ice.Communicator initialize(ref string[] args)
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = createTestProperties(ref args);
            return initialize(initData);
        }

        public Ice.Communicator initialize(Ice.Properties properties)
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = properties;
            return initialize(initData);
        }

        public Ice.Communicator initialize(Ice.InitializationData initData)
        {
            Ice.Communicator communicator = Ice.Util.initialize(initData);
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
                throw new Exception();
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
                throw new Exception();
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
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                status = 1;
            }
            return status;
        }
    }
}
