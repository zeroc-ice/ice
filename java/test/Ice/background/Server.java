// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    static public class LocatorI extends Ice._LocatorDisp
    {
        public void
        findAdapterById_async(Ice.AMD_Locator_findAdapterById response, String adapter, Ice.Current current)
            throws Ice.AdapterNotFoundException
        {
            _controller.checkCallPause(current);
            Ice.Communicator communicator = current.adapter.getCommunicator();
            response.ice_response(current.adapter.createDirectProxy(communicator.stringToIdentity("dummy")));
        }

        public void
        findObjectById_async(Ice.AMD_Locator_findObjectById response, Ice.Identity id, Ice.Current current)
            throws Ice.ObjectNotFoundException
        {
            _controller.checkCallPause(current);
            response.ice_response(current.adapter.createDirectProxy(id));
        }
    
        public Ice.LocatorRegistryPrx
        getRegistry(Ice.Current current)
        {
            return null;
        }
        
        LocatorI(BackgroundControllerI controller)
        {
            _controller = controller;
        }
        
        final private BackgroundControllerI _controller;
    }

    static public class RouterI extends Ice._RouterDisp
    {
        public Ice.ObjectPrx 
        getClientProxy(Ice.Current current)
        {
            _controller.checkCallPause(current);
            return null;
        }

        public Ice.ObjectPrx 
        getServerProxy(Ice.Current current)
        {
            _controller.checkCallPause(current);
            return null;
        }

        /**
         * @deprecated addProxy() is deprecated, use addProxies() instead.
         **/
        public void
        addProxy(Ice.ObjectPrx proxy, Ice.Current current)
        {
        }
        
        public Ice.ObjectPrx[]
        addProxies(Ice.ObjectPrx[] proxies, Ice.Current current)
        {
            return new Ice.ObjectPrx[0];
        }

        RouterI(BackgroundControllerI controller)
        {
            _controller = controller;
        }
        
        final private BackgroundControllerI _controller;
    }

    public static int
    run(String[] args, Ice.Communicator communicator, java.io.PrintStream out)
    {
        //
        // When running as a MIDlet the properties for the server may be
        // overridden by configuration. If it isn't then we assume
        // defaults.
        //
        if(communicator.getProperties().getProperty("TestAdapter.Endpoints").length() == 0)
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
        }
        if(communicator.getProperties().getProperty("ControllerAdapter.Endpoints").length() == 0)
        {
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", "tcp -p 12011");
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");
        }

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

        BackgroundControllerI backgroundController = new BackgroundControllerI(adapter);

        adapter.add(new BackgroundI(backgroundController), communicator.stringToIdentity("background"));
        adapter.add(new LocatorI(backgroundController), communicator.stringToIdentity("locator"));
        adapter.add(new RouterI(backgroundController), communicator.stringToIdentity("router"));
        adapter.activate();

        adapter2.add(backgroundController, communicator.stringToIdentity("backgroundController"));
        adapter2.activate();

        communicator.waitForShutdown();
        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(argsH);
        
            //
            // This test kills connections, so we don't want warnings.
            //
            initData.properties.setProperty("Ice.Warn.Connections", "0");

            //
            // Setup the test transport plugin.
            //
            initData.properties.setProperty("Ice.Plugin.Test", "PluginFactory");
            String defaultProtocol = initData.properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
            initData.properties.setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);
            
            communicator = Ice.Util.initialize(argsH, initData);
            status = run(argsH.value, communicator, System.out);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
