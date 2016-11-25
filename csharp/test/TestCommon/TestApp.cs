// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;

namespace TestCommon
{
    public abstract class Application
    {
        public
        Application()
        {
        }

        //
        // This runmain() must be called by the global main(). runmain()
        // initializes the Communicator, calls run(), and destroys
        // the Communicator upon return from run(). It thereby handles
        // all exceptions properly, i.e., error messages are printed
        // if exceptions propagate to main(), and the Communicator is
        // always destroyed, regardless of exceptions.
        //
        public int runmain(string[] args)
        {
            Ice.InitializationData initData = getInitData(ref args);
            return runmain(args, initData);
        }

        public int runmain(string[] args, Ice.InitializationData initializationData)
        {
            _testName = AppDomain.CurrentDomain.FriendlyName;

            if(_communicator != null)
            {
                Console.Out.WriteLine(_testName + ": only one instance of the Application class can be used");
                return 1;
            }

            //
            // We parse the properties here to extract Ice.ProgramName.
            //
            if(initializationData == null)
            {
                initializationData = getInitData(ref args);
            }

            Ice.InitializationData initData;
            if(initializationData != null)
            {
                initData = (Ice.InitializationData)initializationData.Clone();
            }
            else
            {
                initData = new Ice.InitializationData();
            }
            initData.properties = Ice.Util.createProperties(ref args, initData.properties);

            //
            // If the process logger is the default logger, we replace it with a
            // a logger that uses the program name as the prefix.
            //
            if(Ice.Util.getProcessLogger() is Ice.LoggerI)
            {
                Ice.Util.setProcessLogger(new Ice.ConsoleLoggerI(initData.properties.getProperty("Ice.ProgramName")));
            }

            int status = 0;
            try
            {
                _communicator = Ice.Util.initialize(ref args, initData);
                status = run(args);
            }
            catch(Exception ex)
            {
                Console.Out.WriteLine(_testName + ": " + ex);
                status = 1;
            }

            if(_communicator != null)
            {
                try
                {
                    _communicator.destroy();
                }
                catch(Exception ex)
                {
                    Console.Out.WriteLine(_testName + ": " + ex);
                    status = 1;
                }
                _communicator = null;
            }
            return status;
        }

        public void stop()
        {
            if(_communicator != null)
            {
                _communicator.shutdown();
            }
        }

        public abstract int run(string[] args);

        //
        // Hook to override the initialization data. This hook is
        // necessary because some properties must be set prior to
        // communicator initialization.
        //
        protected virtual Ice.InitializationData getInitData(ref string[] args)
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            args = initData.properties.parseCommandLineOptions("Test", args);
            return initData;
        }

        //
        // Return the application name, i.e., argv[0].
        //
        public string appName()
        {
            return _testName;
        }

        public Ice.Communicator communicator()
        {
            return _communicator;
        }

        public string getTestEndpoint(int num)
        {
            return getTestEndpoint(num, "");
        }

        public string getTestEndpoint(int num, string prot)
        {
            return getTestEndpoint(_communicator.getProperties(), num, prot);
        }

        static public string getTestEndpoint(Ice.Properties properties, int num)
        {
            return getTestEndpoint(properties, num, "");
        }

        static public string getTestEndpoint(Ice.Properties properties, int num, string prot)
        {
            string protocol = prot;
            if(protocol == "")
            {
                protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "default");
            }
            int basePort = properties.getPropertyAsIntWithDefault("Test.BasePort", 12010);
            return protocol + " -p " + (basePort + num);
        }

        public string getTestHost()
        {
            return getTestHost(_communicator.getProperties());
        }

        static public string getTestHost(Ice.Properties properties)
        {
            return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
        }

        public string getTestProtocol()
        {
            return getTestProtocol(_communicator.getProperties());
        }

        static public string getTestProtocol(Ice.Properties properties)
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

        protected static void test(bool b)
        {
            if(!b)
            {
                Debug.Assert(false);
                throw new Exception();
            }
        }

        private string _testName;
        private Ice.Communicator _communicator;
    }

    public abstract class AllTests
    {
        protected static void test(bool b)
        {
            if(!b)
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
}
