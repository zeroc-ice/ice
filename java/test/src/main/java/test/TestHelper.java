// Copyright (c) ZeroC, Inc.

package test;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public abstract class TestHelper {
    public static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public abstract void run(String[] args);

    public interface ControllerHelper {
        void communicatorInitialized(Communicator c);

        InputStream loadResource(String name);

        void serverReady();
    }

    public String getTestEndpoint() {
        return getTestEndpoint(_communicator.getProperties(), 0, "");
    }

    public static String getTestEndpoint(Properties properties) {
        return getTestEndpoint(properties, 0, "");
    }

    public String getTestEndpoint(int num) {
        return getTestEndpoint(_communicator.getProperties(), num, "");
    }

    public static String getTestEndpoint(Properties properties, int num) {
        return getTestEndpoint(properties, num, "");
    }

    public String getTestEndpoint(int num, String protocol) {
        return getTestEndpoint(_communicator.getProperties(), num, protocol);
    }

    public static String getTestEndpoint(
            Properties properties, int num, String prot) {
        String protocol = prot;
        if (protocol.isEmpty()) {
            protocol = properties.getIceProperty("Ice.Default.Protocol");
        }

        int basePort = properties.getPropertyAsIntWithDefault("Test.BasePort", 12010);

        if (protocol.indexOf("bt") == 0) {
            // For Bluetooth, there's no need to specify a port (channel) number.
            // The client locates the server using its address and a UUID.
            switch (num) {
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
                    assert false;
                }
            }
        }
        return protocol + " -p " + Integer.toString(basePort + num);
    }

    public String getTestHost() {
        return getTestHost(_communicator.getProperties());
    }

    public static String getTestHost(Properties properties) {
        return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
    }

    public String getTestProtocol() {
        return getTestProtocol(_communicator.getProperties());
    }

    public static String getTestProtocol(Properties properties) {
        return properties.getIceProperty("Ice.Default.Protocol");
    }

    public int getTestPort(int num) {
        return getTestPort(_communicator.getProperties(), num);
    }

    public static int getTestPort(Properties properties, int num) {
        return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;
    }

    public Properties createTestProperties(String[] args) {
        return createTestProperties(args, null);
    }

    public Properties createTestProperties(String[] args, List<String> rArgs) {
        rArgs = rArgs == null ? new ArrayList<String>() : rArgs;
        Properties properties = new Properties(args, rArgs);
        args = properties.parseCommandLineOptions("Test", rArgs.toArray(new String[rArgs.size()]));
        rArgs.clear();
        rArgs.addAll(Arrays.asList(args));
        return properties;
    }

    public Communicator initialize(String[] args) {
        InitializationData initData = new InitializationData();
        initData.properties = createTestProperties(args);
        return initialize(initData);
    }

    public Communicator initialize(Properties properties) {
        InitializationData initData = new InitializationData();
        initData.properties = properties;
        return initialize(initData);
    }

    public Communicator initialize(InitializationData initData) {
        if (_classLoader != null && initData.classLoader == null) {
            initData.classLoader = _classLoader;
        }

        if (isAndroid()) {
            if (initData.classLoader == null) {
                initData.classLoader = new AndroidClassLoader(getClass().getClassLoader());
            } else {
                initData.classLoader = new AndroidClassLoader(initData.classLoader);
            }
        }

        Communicator communicator = Util.initialize(initData);
        if (_communicator == null) {
            _communicator = communicator;
        }

        if (_controllerHelper != null) {
            _controllerHelper.communicatorInitialized(communicator);
        }

        return communicator;
    }

    public void setControllerHelper(ControllerHelper controllerHelper) {
        _controllerHelper = controllerHelper;
    }

    public void serverReady() {
        if (_controllerHelper != null) {
            _controllerHelper.serverReady();
        }
    }

    public static boolean isAndroid() {
        return Util.isAndroid();
    }

    public void setWriter(Writer writer) {
        _printWriter = new PrintWriter(writer);
    }

    public PrintWriter getWriter() {
        return _printWriter;
    }

    public void setClassLoader(ClassLoader classLoader) {
        _classLoader = classLoader;
    }

    public ClassLoader getClassLoader() {
        return _classLoader;
    }

    public Communicator communicator() {
        return _communicator;
    }

    public void shutdown() {
        if (_communicator != null) {
            _communicator.shutdown();
        }
    }

    public class AndroidClassLoader extends ClassLoader {
        public AndroidClassLoader(ClassLoader parent) {
            super(parent);
        }

        @Override
        public InputStream getResourceAsStream(String name) {
            InputStream resource = _controllerHelper.loadResource(name);
            if (resource == null) {
                resource = super.getResourceAsStream(name);
            }
            return resource;
        }
    }

    private ControllerHelper _controllerHelper;
    private ClassLoader _classLoader;
    private Communicator _communicator;
    private PrintWriter _printWriter =
        new PrintWriter(new OutputStreamWriter(System.out), true);
}
