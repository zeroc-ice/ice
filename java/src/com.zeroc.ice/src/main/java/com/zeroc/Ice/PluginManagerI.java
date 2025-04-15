// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

final class PluginManagerI implements PluginManager {
    private static final String _kindOfObject = "plugin";

    @Override
    public synchronized void initializePlugins() {
        if (_initialized) {
            throw new InitializationException("plug-ins already initialized");
        }

        //
        // Invoke initialize() on the plug-ins, in the order they were loaded.
        //
        List<Plugin> initializedPlugins = new ArrayList<>();
        try {
            for (PluginInfo p : _plugins) {
                try {
                    p.plugin.initialize();
                } catch (PluginInitializationException ex) {
                    throw ex;
                } catch (RuntimeException ex) {
                    throw new PluginInitializationException(
                        "plugin '" + p.name + "' initialization failed", ex);
                }
                initializedPlugins.add(p.plugin);
            }
        } catch (RuntimeException ex) {
            //
            // Destroy the plug-ins that have been successfully initialized, in the
            // reverse order.
            //
            ListIterator<Plugin> i =
                initializedPlugins.listIterator(initializedPlugins.size());
            while (i.hasPrevious()) {
                Plugin p = i.previous();
                try {
                    p.destroy();
                } catch (RuntimeException e) {
                    // Ignore.
                }
            }
            throw ex;
        }

        _initialized = true;
    }

    @Override
    public synchronized String[] getPlugins() {
        ArrayList<String> names = new ArrayList<>();
        for (PluginInfo p : _plugins) {
            names.add(p.name);
        }
        return names.toArray(new String[0]);
    }

    @Override
    public synchronized Plugin getPlugin(String name) {
        if (_communicator == null) {
            throw new CommunicatorDestroyedException();
        }

        Plugin p = findPlugin(name);
        if (p != null) {
            return p;
        }

        throw new NotRegisteredException(_kindOfObject, name);
    }

    @Override
    public synchronized void addPlugin(String name, Plugin plugin) {
        if (_communicator == null) {
            throw new CommunicatorDestroyedException();
        }

        if (findPlugin(name) != null) {
            throw new AlreadyRegisteredException(_kindOfObject, name);
        }

        PluginInfo info = new PluginInfo();
        info.name = name;
        info.plugin = plugin;
        _plugins.add(info);
    }

    @Override
    public synchronized void destroy() {
        if (_communicator != null) {
            if (_initialized) {
                ListIterator<PluginInfo> i = _plugins.listIterator(_plugins.size());
                while (i.hasPrevious()) {
                    PluginInfo p = i.previous();
                    try {
                        p.plugin.destroy();
                    } catch (RuntimeException ex) {
                        Util.getProcessLogger()
                            .warning(
                                "unexpected exception raised by plug-in `"
                                    + p.name
                                    + "' destruction:\n"
                                    + ex.toString());
                    }
                }
            }

            _communicator = null;
        }

        _plugins.clear();

        if (_classLoaders != null) {
            _classLoaders.clear();
        }
    }

    public PluginManagerI(Communicator communicator, Instance instance) {
        _communicator = communicator;
        _instance = instance;
        _initialized = false;
    }

    public String[] loadPlugins(String[] cmdArgs) {
        assert (_communicator != null);

        final String prefix = "Ice.Plugin.";
        Properties properties = _communicator.getProperties();
        Map<String, String> plugins = properties.getPropertiesForPrefix(prefix);

        // First, create plug-ins using the plug-in factories from initData, in order.
        for (PluginFactory pluginFactory : _communicator.getInstance().initializationData().pluginFactories) {
            String name = pluginFactory.getPluginName();
            String key = "Ice.Plugin." + name;
            if (plugins.containsKey(key)) {
                cmdArgs = loadPlugin(pluginFactory, name, plugins.get(key), cmdArgs);
                plugins.remove(key);
            } else {
                cmdArgs = loadPlugin(pluginFactory, name, "", cmdArgs);
            }
        }

        //
        // Load and initialize the plug-ins defined in the property set
        // with the prefix "Ice.Plugin.". These properties should
        // have the following format:
        //
        // Ice.Plugin.<name>=entry_point [args]
        //
        // If the Ice.PluginLoadOrder property is defined, load the
        // specified plug-ins in the specified order, then load any
        // remaining plug-ins.
        //

        final String[] loadOrder = properties.getIcePropertyAsList("Ice.PluginLoadOrder");
        for (String name : loadOrder) {
            if (findPlugin(name) != null) {
                throw new PluginInitializationException("plug-in '" + name + "' already loaded");
            }

            String key = "Ice.Plugin." + name;
            boolean hasKey = plugins.containsKey(key);
            if (hasKey) {
                final String value = plugins.get(key);
                cmdArgs = loadPlugin(null, name, value, cmdArgs);
                plugins.remove(key);
            } else {
                throw new PluginInitializationException("plug-in '" + name + "' not defined");
            }
        }

        //
        // Load any remaining plug-ins that weren't specified in PluginLoadOrder.
        //
        for (var entry : plugins.entrySet()) {
            cmdArgs =
                loadPlugin(
                    null, entry.getKey().substring(prefix.length()), entry.getValue(), cmdArgs);
        }

        return cmdArgs;
    }

    private String[] loadPlugin(PluginFactory pluginFactory, String name, String pluginSpec, String[] cmdArgs) {
        assert (_communicator != null);

        if (findPlugin(name) != null) {
            throw new AlreadyRegisteredException(_kindOfObject, name);
        }

        //
        // We support the following formats:
        //
        // <class-name> [args]
        // <jar-file>:<class-name> [args]
        // <class-dir>:<class-name> [args]
        // "<path with spaces>":<class-name> [args]
        // "<path with spaces>:<class-name>" [args]
        //

        String[] args = new String[0];
        String entryPoint = "";
        String classDir = null; // Path name of JAR file or subdirectory.
        String className = "InvalidClass";
        boolean absolutePath = false;

        if (!pluginSpec.isEmpty()) {
            try {
                args = Options.split(pluginSpec);
            } catch (ParseException ex) {
                throw new PluginInitializationException(
                    "invalid arguments for plug-in `" + name + "'", ex);
            }

            assert (args.length > 0);

            entryPoint = args[0];

            final boolean isWindows = System.getProperty("os.name").startsWith("Windows");

            //
            // Find first ':' that isn't part of the file path.
            //
            int pos = entryPoint.indexOf(':');
            if (isWindows) {
                final String driveLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
                if (pos == 1
                    && entryPoint.length() > 2
                    && driveLetters.indexOf(entryPoint.charAt(0)) != -1
                    && (entryPoint.charAt(2) == '\\' || entryPoint.charAt(2) == '/')) {
                    absolutePath = true;
                    pos = entryPoint.indexOf(':', pos + 1);
                }
                if (!absolutePath) {
                    absolutePath = entryPoint.startsWith("\\\\");
                }
            } else {
                absolutePath = entryPoint.startsWith("/");
            }

            if ((pos == -1 && absolutePath) || (pos != -1 && entryPoint.length() <= pos + 1)) {
                //
                // Class name is missing.
                //
                throw new PluginInitializationException(
                    "invalid entry point for plug-in `" + name + "':\n" + entryPoint);
            }

            //
            // Extract the JAR file or subdirectory, if any.
            //

            if (pos == -1) {
                className = entryPoint;
            } else {
                classDir = entryPoint.substring(0, pos).trim();
                className = entryPoint.substring(pos + 1).trim();
            }

            //
            // Shift the arguments.
            //
            String[] tmp = new String[args.length - 1];
            System.arraycopy(args, 1, tmp, 0, args.length - 1);
            args = tmp;

            //
            // Convert command-line options into properties. First we
            // convert the options from the plug-in configuration, then
            // we convert the options from the application command-line.
            //
            Properties properties = _communicator.getProperties();
            args = properties.parseCommandLineOptions(name, args);
            cmdArgs = properties.parseCommandLineOptions(name, cmdArgs);
        }

        if (pluginFactory == null) {
            try {
                Class<?> c = null;

                //
                // Use a class loader if the user specified a JAR file or class directory.
                //
                if (classDir != null) {
                    try {
                        if (!absolutePath) {
                            classDir =
                                new File(
                                    System.getProperty("user.dir")
                                        + File.separator
                                        + classDir)
                                    .getCanonicalPath();
                        }

                        if (!classDir.endsWith(File.separator)
                            && !classDir.toLowerCase().endsWith(".jar")) {
                            classDir += File.separator;
                        }
                        classDir = URLEncoder.encode(classDir, "UTF-8");

                        //
                        // Reuse an existing class loader if we have already loaded a plug-in with
                        // the same value for classDir, otherwise create a new one.
                        //
                        ClassLoader cl = null;

                        if (_classLoaders == null) {
                            _classLoaders = new HashMap<>();
                        } else {
                            cl = _classLoaders.get(classDir);
                        }

                        if (cl == null) {
                            final URL[] url =
                                new URL[]{new URL("file", null, classDir)};

                            //
                            // Use the custom class loader (if any) as the parent.
                            //
                            if (_instance.initializationData().classLoader != null) {
                                cl =
                                    new URLClassLoader(
                                        url, _instance.initializationData().classLoader);
                            } else {
                                cl = new URLClassLoader(url);
                            }

                            _classLoaders.put(classDir, cl);
                        }

                        c = cl.loadClass(className);
                    } catch (MalformedURLException ex) {
                        throw new PluginInitializationException(
                            "invalid entry point format '" + pluginSpec + "'", ex);
                    } catch (IOException ex) {
                        throw new PluginInitializationException(
                            "invalid path in entry point '" + pluginSpec + "'", ex);
                    } catch (ClassNotFoundException ex) {
                        // Ignored
                    }
                } else {
                    c = _communicator.getInstance().findClass(className);
                }

                if (c == null) {
                    throw new PluginInitializationException("class " + className + " not found");
                }

                java.lang.Object obj = c.getDeclaredConstructor().newInstance();
                try {
                    pluginFactory = (PluginFactory) obj;
                } catch (ClassCastException ex) {
                    throw new PluginInitializationException(
                        "class " + className + " does not implement PluginFactory", ex);
                }
            } catch (NoSuchMethodException ex) {
                throw new PluginInitializationException("unable to instantiate class " + className, ex);
            } catch (java.lang.reflect.InvocationTargetException ex) {
                throw new PluginInitializationException("unable to instantiate class " + className, ex);
            } catch (IllegalAccessException ex) {
                throw new PluginInitializationException(
                    "unable to access default constructor in class " + className, ex);
            } catch (InstantiationException ex) {
                throw new PluginInitializationException("unable to instantiate class " + className, ex);
            }
        }

        //
        // Invoke the factory.
        //
        Plugin plugin = null;
        try {
            plugin = pluginFactory.create(_communicator, name, args);
        } catch (PluginInitializationException ex) {
            throw ex;
        } catch (Throwable ex) {
            throw new PluginInitializationException("exception in factory " + className, ex);
        }

        if (plugin == null) {
            throw new PluginInitializationException("failure in factory " + className);
        }

        PluginInfo info = new PluginInfo();
        info.name = name;
        info.plugin = plugin;
        _plugins.add(info);

        return cmdArgs;
    }

    private Plugin findPlugin(String name) {
        for (PluginInfo p : _plugins) {
            if (name.equals(p.name)) {
                return p.plugin;
            }
        }
        return null;
    }

    static class PluginInfo {
        String name;
        Plugin plugin;
    }

    private Communicator _communicator;
    private final Instance _instance;
    private final List<PluginInfo> _plugins = new ArrayList<>();
    private boolean _initialized;
    private Map<String, ClassLoader> _classLoaders;
}
