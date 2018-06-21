// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Text;

namespace Test
{
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
            if(_communicator == null)
            {
                _communicator = communicator;
            }
            return  communicator;
        }

        public Ice.Communicator communicator()
        {
            return _communicator;
        }

        public void shutdown()
        {
            if(_communicator != null)
            {
                _communicator.shutdown();
            }
        }

        protected static void test(bool b)
        {
            if (!b)
            {
                Debug.Assert(false);
                throw new Exception();
            }
        }

        private Ice.Communicator _communicator;
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

        public static void Write(string msg)
        {
            Console.Out.Write(msg);
        }

        public static void WriteLine(string msg)
        {
            Console.Out.WriteLine(msg);
        }

        public static void Flush()
        {
            Console.Out.Flush();
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
