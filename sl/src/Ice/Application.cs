// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    using System;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using System.Threading;

    public abstract class Application
    {
        public abstract int run(string[] args);

        public Application()
        {
        }

        //
        // This main() must be called by the global Main(). main()
        // initializes the Communicator, calls run(), and destroys
        // the Communicator upon return from run(). It thereby handles
        // all exceptions properly, i.e., error messages are printed
        // if exceptions propagate to main(), and the Communicator is
        // always destroyed, regardless of exceptions.
        //
        public int main(string[] args)
        {
            return main(args, new InitializationData());
        }

        public int main(string[] args, string configFile)
        {
            InitializationData initData = new InitializationData();
            if(configFile != null)
            {
                try
                {
                    initData.properties = Util.createProperties();
                    initData.properties.load(configFile);
                }
                catch(Ice.Exception ex)
                {
                    Console.Error.WriteLine(_appName + ": " + ex);
                    return 1;
                }
                catch(System.Exception ex)
                {
                    Console.Error.WriteLine(_appName + ": unknown exception: " + ex);
                    return 1;
                }
            }
            return main(args, initData);
        }

        public int main(string[] args, InitializationData initData)
        {
            if(_communicator != null)
            {
                Console.Error.WriteLine(_appName + ": only one instance of the Application class can be used");
                return 1;
            }   
            int status = 0;
            
            try
            {
                _communicator = Util.initialize(ref args, initData);
                
                Properties props = _communicator.getProperties();
                _appName = props.getPropertyWithDefault("Ice.ProgramName", _appName);

                status = run(args);
            }
            catch(Ice.Exception ex)
            {
                Console.Error.WriteLine(_appName + ": " + ex);
                status = 1;
            }
            catch(System.Exception ex)
            {
                Console.Error.WriteLine(_appName + ": unknown exception: " + ex);
                status = 1;
            }

            if(_communicator != null)
            {
                try
                {
                    _communicator.destroy();
                }
                catch(Ice.Exception ex)
                {
                    Console.Error.WriteLine(_appName + ": " + ex);
                    status = 1;
                }
                catch(System.Exception ex)
                {
                    Console.Error.WriteLine(_appName + ": unknown exception: " + ex);
                    status = 1;
                }
                _communicator = null;
            }   
            return status;
        }

        //
        // Return the application name.
        //
        public static string appName()
        {
            return _appName;
        }
        
        //
        // One limitation of this class is that there can only be one
        // Application instance, with one global Communicator, accessible
        // with this communicator() operation. This limitiation is due to
        // how the signal handling functions below operate. If you require
        // multiple Communicators, then you cannot use this Application
        // framework class.
        //
        public static Communicator communicator()
        {
            return _communicator;
        }
        
        //
        // We use FriendlyName instead of Process.GetCurrentProcess().ProcessName because the latter
        // is terribly slow. (It takes around 1 second!)
        //
        private static string _appName = AppDomain.CurrentDomain.FriendlyName;
        private static Communicator _communicator;
    }
}
