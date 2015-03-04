// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

using System.Threading;
using System.Collections.Generic;

public class Server : Ice.Application
{
    //
    // The servant implements the Slice interface Demo::Props as well as the
    // native callback interface Ice.PropertiesAdminUpdateCallback.
    //
    class PropsI : Demo.PropsDisp_, Ice.PropertiesAdminUpdateCallback
    {
        public PropsI()
        {
            _called = false;
            _monitor = new IceUtilInternal.Monitor();
        }

        override public Dictionary<string, string> getChanges(Ice.Current current)
        {
            lock(_monitor)
            {
                //
                // Make sure that we have received the property updates before we
                // return the results.
                //
                while(!_called)
                {
                    _monitor.Wait();
                }

                _called = false;
                return _changes;
            }
        }
        
        override public void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        public void updated(Dictionary<string, string> changes)
        {
            lock(_monitor)
            {
                _changes = changes;
                _called = true;
                _monitor.Notify();
            }
        }

        Dictionary<string, string> _changes;
        private bool _called;
        IceUtilInternal.Monitor _monitor;
    }

    override public int run(string[] args)
    {
        if(args.Length > 0)
        {
            System.Console.Error.WriteLine(appName() + ": too many arguments");
            return 1;
        }

        PropsI props = new PropsI();

        //
        // Retrieve the PropertiesAdmin facet and register the servant as the update callback.
        //
        Ice.Object obj = communicator().findAdminFacet("Properties");
        Ice.NativePropertiesAdmin admin = (Ice.NativePropertiesAdmin)obj;
        admin.addUpdateCallback(props);

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Props");
        adapter.add(props, communicator().stringToIdentity("props"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.main(args, "config.server");
    }
}
