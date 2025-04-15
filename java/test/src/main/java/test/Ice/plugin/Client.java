// Copyright (c) ZeroC, Inc.

package test.Ice.plugin;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginInitializationException;
import com.zeroc.Ice.PluginManager;
import com.zeroc.Ice.Properties;

import test.TestHelper;

import java.io.PrintWriter;

public class Client extends TestHelper {
    public void run(String[] args) {
        //
        // Under Android the class comes from the communicators classloader which
        // is setup in the android test driver.
        //
        String jarFile = isAndroid() ? "" : "../../../../../../../lib/IceTestPlugins.jar";
        PrintWriter printWriter = getWriter();
        {
            printWriter.print("testing a simple plug-in... ");
            printWriter.flush();
            Properties properties = createTestProperties(args);
            properties.setProperty(
                "Ice.Plugin.Test",
                jarFile
                    + ":test.Ice.plugin.plugins.PluginFactory "
                    + "'C:\\Program Files\\' --DatabasePath 'C:\\Program Files\\Application\\db'");
            try (Communicator communicator = initialize(properties)) {}
            printWriter.println("ok");
        }

        {
            printWriter.print("testing a simple plug-in that fails to initialize... ");
            printWriter.flush();
            Properties properties = createTestProperties(args);
            properties.setProperty(
                "Ice.Plugin.Test",
                jarFile + ":test.Ice.plugin.plugins.PluginInitializeFailFactory");
            try (Communicator communicator = initialize(properties)) {
                test(false);
            } catch (PluginInitializationException ex) {
                test("PluginInitializeFailException".equals(ex.getCause().getMessage()));
            }
            printWriter.println("ok");
        }

        {
            printWriter.print("testing plug-in load order... ");
            printWriter.flush();

            Properties properties = createTestProperties(args);
            properties.setProperty(
                "Ice.Plugin.PluginOne", jarFile + ":test.Ice.plugin.plugins.PluginOneFactory");
            properties.setProperty(
                "Ice.Plugin.PluginTwo", jarFile + ":test.Ice.plugin.plugins.PluginTwoFactory");
            properties.setProperty(
                "Ice.Plugin.PluginThree",
                jarFile + ":test.Ice.plugin.plugins.PluginThreeFactory");
            properties.setProperty(
                "Ice.PluginLoadOrder", "PluginOne, PluginTwo"); // Exclude PluginThree
            try (Communicator communicator = initialize(properties)) {}
            printWriter.println("ok");
        }

        {
            printWriter.print("testing plug-in manager... ");
            printWriter.flush();

            Properties properties = createTestProperties(args);
            properties.setProperty(
                "Ice.Plugin.PluginOne", jarFile + ":test.Ice.plugin.plugins.PluginOneFactory");
            properties.setProperty(
                "Ice.Plugin.PluginTwo", jarFile + ":test.Ice.plugin.plugins.PluginTwoFactory");
            properties.setProperty(
                "Ice.Plugin.PluginThree",
                jarFile + ":test.Ice.plugin.plugins.PluginThreeFactory");
            properties.setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo");
            properties.setProperty("Ice.InitPlugins", "0");

            try (Communicator communicator = initialize(properties)) {
                PluginManager pm = communicator.getPluginManager();
                test(pm.getPlugin("PluginOne") != null);
                test(pm.getPlugin("PluginTwo") != null);
                test(pm.getPlugin("PluginThree") != null);

                MyPlugin p4 = new MyPlugin();
                pm.addPlugin("PluginFour", p4);
                test(pm.getPlugin("PluginFour") != null);

                pm.initializePlugins();

                test(p4.isInitialized());

                communicator.destroy();

                test(p4.isDestroyed());
            }
            printWriter.println("ok");
        }

        {
            printWriter.print("testing destroy when a plug-in fails to initialize... ");
            printWriter.flush();

            Properties properties = createTestProperties(args);
            properties.setProperty(
                "Ice.Plugin.PluginOneFail",
                jarFile + ":test.Ice.plugin.plugins.PluginOneFailFactory");
            properties.setProperty(
                "Ice.Plugin.PluginTwoFail",
                jarFile + ":test.Ice.plugin.plugins.PluginTwoFailFactory");
            properties.setProperty(
                "Ice.Plugin.PluginThreeFail",
                jarFile + ":test.Ice.plugin.plugins.PluginThreeFailFactory");
            properties.setProperty(
                "Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
            try (Communicator communicator = initialize(properties)) {
                test(false);
            } catch (PluginInitializationException ex) {
                test("PluginInitializeFailException".equals(ex.getCause().getMessage()));
            }
            printWriter.println("ok");
        }

        {
            printWriter.print("testing that IceLocatorDiscovery can't be loaded under a different name... ");
            printWriter.flush();
            try {
                Properties properties = new Properties();
                properties.setProperty(
                    "Ice.Plugin.Loco",
                    "IceLocatorDiscovery:com.zeroc.IceLocatorDiscovery.PluginFactory");
                initialize(properties);
                test(false);
            } catch (PluginInitializationException ex) {
                test("The Locator Discovery plug-in must be named 'IceLocatorDiscovery'.".equals(ex.getMessage()));
            }
            printWriter.println("ok");
        }
    }

    static class MyPlugin implements Plugin {
        public boolean isInitialized() {
            return _initialized;
        }

        public boolean isDestroyed() {
            return _destroyed;
        }

        @Override
        public void initialize() {
            _initialized = true;
        }

        @Override
        public void destroy() {
            _destroyed = true;
        }

        private boolean _initialized;
        private boolean _destroyed;
    }
}
