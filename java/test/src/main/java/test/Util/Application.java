// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Util;

import com.zeroc.Ice.*;

public abstract class Application
{
    public final int WAIT = 2;

    public interface ServerReadyListener
    {
        void serverReady();
    }

    public interface CommunicatorListener
    {
        void communicatorInitialized(Communicator c);
    }

    public Application()
    {
    }

    //
    // This main() must be called by the global main(). main()
    // initializes the Communicator, calls run(), and destroys
    // the Communicator upon return from run(). It thereby handles
    // all exceptions properly, i.e., error messages are printed
    // if exceptions propagate to main(), and the Communicator is
    // always destroyed, regardless of exceptions.
    //
    public final int main(String appName, String[] args)
    {
        return main(appName, args, null);
    }

    public final int main(String appName, String[] args, InitializationData initializationData)
    {
        java.io.PrintWriter writer = getWriter();
        if(_communicator != null)
        {
            writer.println(appName + ": only one instance of the Application class can be used");
            return 1;
        }

        _testName = appName;

        //
        // We parse the properties here to extract Ice.ProgramName.
        //
        if(initializationData == null)
        {
            java.util.List<String> remainingArgs = new java.util.ArrayList<>();
            initializationData = getInitData(args, remainingArgs);
            args = remainingArgs.toArray(new String[remainingArgs.size()]);
        }

        InitializationData initData;
        if(initializationData != null)
        {
            initData = initializationData.clone();
        }
        else
        {
            initData = new InitializationData();
        }

        java.util.List<String> remainingArgs = new java.util.ArrayList<>();
        initData.properties = Util.createProperties(args, initData.properties, remainingArgs);
        args = remainingArgs.toArray(new String[remainingArgs.size()]);

        //
        // If the process logger is the default logger, we replace it with a
        // a logger that uses the program name as the prefix.
        //
        if(Util.getProcessLogger() instanceof LoggerI)
        {
            Util.setProcessLogger(new LoggerI(initData.properties.getProperty("Ice.ProgramName"), ""));
        }

        int status = 0;

        try(Communicator communicator = Util.initialize(args, initData, remainingArgs))
        {
            _communicator = communicator;
            args = remainingArgs.toArray(new String[remainingArgs.size()]);

            if(_communicatorListener != null)
            {
                _communicatorListener.communicatorInitialized(_communicator);
            }
            status = run(args);
            if(status == WAIT)
            {
                if(_cb != null)
                {
                    _cb.serverReady();
                }
                _communicator.waitForShutdown();
                status = 0;
            }
        }
        catch(LocalException ex)
        {
            writer.println(_testName + ": " + ex);
            ex.printStackTrace();
            status = 1;
        }
        catch(java.lang.Exception ex)
        {
            writer.println(_testName + ": unknown exception");
            ex.printStackTrace(writer);
            status = 1;
        }
        catch(java.lang.Error err)
        {
            //
            // We catch Error to avoid hangs in some non-fatal situations
            //
            writer.println(_testName + ": Java error");
            err.printStackTrace(writer);
            status = 1;
        }
        finally
        {
            _communicator = null;
        }
        writer.flush();

        return status;
    }

    public void stop()
    {
        if(_communicator != null)
        {
            _communicator.shutdown();
        }
    }

    //
    // Initialize a new communicator.
    //
    public Communicator initialize(InitializationData initData)
    {
        Communicator communicator = Util.initialize(initData);
        if(_communicatorListener != null)
        {
            _communicatorListener.communicatorInitialized(communicator);
        }
        return communicator;
    }

    public Communicator initialize()
    {
        Communicator communicator = Util.initialize();
        if(_communicatorListener != null)
        {
            _communicatorListener.communicatorInitialized(communicator);
        }
        return communicator;
    }

    public abstract int run(String[] args);

    //
    // Hook to override the initialization data. This hook is
    // necessary because some properties must be set prior to
    // communicator initialization.
    //
    protected InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        InitializationData initData = createInitializationData();
        initData.properties = Util.createProperties(args, rArgs);
        args = initData.properties.parseCommandLineOptions("Test", rArgs.toArray(new String[rArgs.size()]));
        rArgs.clear();
        rArgs.addAll(java.util.Arrays.asList(args));
        return initData;
    }

    public java.io.PrintWriter getWriter()
    {
        return _printWriter;
    }

    public void setWriter(java.io.Writer writer)
    {
        _printWriter = new java.io.PrintWriter(writer);
    }

    public void setLogger(com.zeroc.Ice.Logger logger)
    {
        _logger = logger;
    }

    public void setServerReadyListener(ServerReadyListener cb)
    {
        _cb = cb;
    }

    public void setCommunicatorListener(CommunicatorListener listener)
    {
        _communicatorListener = listener;
    }

    public void serverReady()
    {
        if(_cb != null)
        {
            _cb.serverReady();
        }
    }

    static public boolean isAndroid()
    {
        return com.zeroc.IceInternal.Util.isAndroid();
    }
    //
    // Return the application name, i.e., argv[0].
    //
    public String appName()
    {
        return _testName;
    }

    public Communicator communicator()
    {
        return _communicator;
    }

    public void setClassLoader(ClassLoader c)
    {
        _classLoader = c;
    }

    public ClassLoader getClassLoader()
    {
        return _classLoader;
    }

    public InitializationData createInitializationData()
    {
        InitializationData initData = new InitializationData();
        initData.classLoader = _classLoader;
        initData.logger = _logger;
        return initData;
    }

    public ClassLoader classLoader()
    {
        return _classLoader;
    }

    public String getTestEndpoint(int num)
    {
        return getTestEndpoint(num, "");
    }

    public String getTestEndpoint(com.zeroc.Ice.Properties properties, int num)
    {
        return getTestEndpoint(properties, num, "");
    }

    public String getTestEndpoint(int num, String prot)
    {
        return getTestEndpoint(_communicator.getProperties(), num, prot);
    }

    static public String getTestEndpoint(com.zeroc.Ice.Properties properties, int num, String prot)
    {
        String protocol = prot;
        if(protocol.isEmpty())
        {
            protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "default");
        }
        int basePort = properties.getPropertyAsIntWithDefault("Test.BasePort", 12010);
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

    private ClassLoader _classLoader;
    private String _testName;
    private Communicator _communicator;
    private Logger _logger = null;
    private java.io.PrintWriter _printWriter = new java.io.PrintWriter(new java.io.OutputStreamWriter(System.out));
    private ServerReadyListener _cb = null;
    private CommunicatorListener _communicatorListener = null;
}
