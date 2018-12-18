// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test;

import com.zeroc.Ice.*;

public abstract class TestHelper
{
    public static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public abstract void run(String args[]);

    public interface ControllerHelper
    {
        void communicatorInitialized(Communicator c);
        void serverReady();
    }

    public String getTestEndpoint()
    {
        return getTestEndpoint(_communicator.getProperties(), 0, "");
    }

    static public String getTestEndpoint(com.zeroc.Ice.Properties properties)
    {
        return getTestEndpoint(properties, 0, "");
    }

    public String getTestEndpoint(int num)
    {
        return getTestEndpoint(_communicator.getProperties(), num, "");
    }

    static public String getTestEndpoint(com.zeroc.Ice.Properties properties, int num)
    {
        return getTestEndpoint(properties, num, "");
    }

    public String getTestEndpoint(int num, String protocol)
    {
        return getTestEndpoint(_communicator.getProperties(), num, protocol);
    }

    static public String getTestEndpoint(com.zeroc.Ice.Properties properties, int num, String prot)
    {
        String protocol = prot;
        if(protocol.isEmpty())
        {
            protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "default");
        }

        int basePort = properties.getPropertyAsIntWithDefault("Test.BasePort", 12010);

        if(protocol.indexOf("bt") == 0)
        {
            //
            // For Bluetooth, there's no need to specify a port (channel) number.
            // The client locates the server using its address and a UUID.
            //
            switch(num)
            {
                case 0:
                {
                    return "default -u 5e08f4de-5015-4507-abe1-a7807002db3d";
                }
                case 1:
                {
                    return "default -u dae56460-2485-46fd-a3ca-8b730e1e868b";
                }
                case 2:
                {
                    return "default -u 99e08bc6-fcda-4758-afd0-a8c00655c999";
                }
                default:
                {
                    assert(false);
                }
            }
        }
        return protocol + " -p " + Integer.toString(basePort + num);
    }

    public String getTestHost()
    {
        return getTestHost(_communicator.getProperties());
    }

    static public String getTestHost(com.zeroc.Ice.Properties properties)
    {
        return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
    }

    public String getTestProtocol()
    {
        return getTestProtocol(_communicator.getProperties());
    }

    static public String getTestProtocol(com.zeroc.Ice.Properties properties)
    {
        return properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
    }

    public int getTestPort(int num)
    {
        return getTestPort(_communicator.getProperties(), num);
    }

    static public int getTestPort(com.zeroc.Ice.Properties properties, int num)
    {
        return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;
    }

    public Properties createTestProperties(String[] args)
    {
        return createTestProperties(args, null);
    }

    public Properties createTestProperties(String[] args, java.util.List<String> rArgs)
    {
        rArgs = rArgs == null ? new java.util.ArrayList<String>() : rArgs;
        Properties properties = Util.createProperties(args, rArgs);
        args = properties.parseCommandLineOptions("Test", rArgs.toArray(new String[rArgs.size()]));
        rArgs.clear();
        rArgs.addAll(java.util.Arrays.asList(args));
        return properties;
    }

    public Communicator initialize(String[] args)
    {
        InitializationData initData = new InitializationData();
        initData.properties = createTestProperties(args);
        return initialize(initData);
    }

    public Communicator initialize(Properties properties)
    {
        InitializationData initData = new InitializationData();
        initData.properties = properties;
        return  initialize(initData);
    }

    public Communicator initialize(InitializationData initData)
    {
        if(_classLoader != null && initData.classLoader == null)
        {
            initData.classLoader = _classLoader;
        }

        Communicator communicator = Util.initialize(initData);
        if(_communicator == null)
        {
            _communicator = communicator;
        }

        if(_controllerHelper != null)
        {
            _controllerHelper.communicatorInitialized(communicator);
        }

        return communicator;
    }

    public void setControllerHelper(ControllerHelper controllerHelper)
    {
        _controllerHelper = controllerHelper;
    }

    public void serverReady()
    {
        if(_controllerHelper != null)
        {
            _controllerHelper.serverReady();
        }
    }

    static public boolean isAndroid()
    {
        return com.zeroc.IceInternal.Util.isAndroid();
    }

    public void setWriter(java.io.Writer writer)
    {
        _printWriter = new java.io.PrintWriter(writer);
    }

    public java.io.PrintWriter getWriter()
    {
        return _printWriter;
    }

    public void setClassLoader(ClassLoader classLoader)
    {
        _classLoader = classLoader;
    }

    public ClassLoader getClassLoader()
    {
        return _classLoader;
    }

    public Communicator communicator()
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

    private ControllerHelper _controllerHelper;
    private ClassLoader _classLoader;
    private Communicator _communicator;
    private java.io.PrintWriter _printWriter = new java.io.PrintWriter(new java.io.OutputStreamWriter(System.out));
}
