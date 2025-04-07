// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBox;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.CommunicatorDestroyedException;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.Logger;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapterDeactivatedException;
import com.zeroc.Ice.ObjectAdapterDestroyedException;
import com.zeroc.Ice.Options;
import com.zeroc.Ice.ParseException;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Set;

// NOTE: the class isn't final on purpose to allow users to eventually extend it.
public class ServiceManagerI implements ServiceManager {
    public ServiceManagerI(Communicator communicator, String[] args) {
        _communicator = communicator;
        _logger = _communicator.getLogger();

        Properties props = _communicator.getProperties();

        if (props.getIceProperty("Ice.Admin.Enabled").isEmpty()) {
            _adminEnabled = !props.getIceProperty("Ice.Admin.Endpoints").isEmpty();
        } else {
            _adminEnabled = props.getIcePropertyAsInt("Ice.Admin.Enabled") > 0;
        }

        if (_adminEnabled) {
            String[] facetFilter = props.getIcePropertyAsList("Ice.Admin.Facets");
            if (facetFilter.length > 0) {
                _adminFacetFilter = new HashSet<>(Arrays.asList(facetFilter));
            } else {
                _adminFacetFilter = new HashSet<>();
            }
        }

        _argv = args;
        _traceServiceObserver = props.getIcePropertyAsInt("IceBox.Trace.ServiceObserver");
    }

    @Override
    public void startService(String name, Current current)
            throws AlreadyStartedException, NoSuchServiceException {
        ServiceInfo info = null;
        synchronized (this) {
            // Search would be more efficient if services were contained in a map, but order is
            // required for shutdown.
            for (ServiceInfo p : _services) {
                if (p.name.equals(name)) {
                    if (p.status == StatusStarted) {
                        throw new AlreadyStartedException();
                    }
                    p.status = StatusStarting;
                    info = p.clone();
                    break;
                }
            }
            if (info == null) {
                throw new NoSuchServiceException();
            }
            _pendingStatusChanges = true;
        }

        boolean started = false;
        try {
            info.service.start(
                    name,
                    info.communicator == null ? _sharedCommunicator : info.communicator,
                    info.args);
            started = true;
        } catch (Exception e) {
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            e.printStackTrace(pw);
            pw.flush();
            _logger.warning(
                    "ServiceManager: exception while starting service "
                            + info.name
                            + ":\n"
                            + sw.toString());
        }

        synchronized (this) {
            for (ServiceInfo p : _services) {
                if (p.name.equals(name)) {
                    if (started) {
                        p.status = StatusStarted;

                        List<String> services = new ArrayList<>();
                        services.add(name);
                        servicesStarted(services, _observers);
                    } else {
                        p.status = StatusStopped;
                    }
                    break;
                }
            }
            _pendingStatusChanges = false;
            notifyAll();
        }
    }

    @Override
    public void stopService(String name, Current current)
            throws AlreadyStoppedException, NoSuchServiceException {
        ServiceInfo info = null;
        synchronized (this) {
            // Search would be more efficient if services were contained in a map, but order is
            // required for shutdown.
            for (ServiceInfo p : _services) {
                if (p.name.equals(name)) {
                    if (p.status == StatusStopped) {
                        throw new AlreadyStoppedException();
                    }
                    p.status = StatusStopping;
                    info = p.clone();
                    break;
                }
            }
            if (info == null) {
                throw new NoSuchServiceException();
            }
            _pendingStatusChanges = true;
        }

        boolean stopped = false;
        try {
            info.service.stop();
            stopped = true;
        } catch (Exception e) {
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            e.printStackTrace(pw);
            pw.flush();
            _logger.warning(
                    "ServiceManager: exception while stopping service "
                            + info.name
                            + ":\n"
                            + sw.toString());
        }

        synchronized (this) {
            for (ServiceInfo p : _services) {
                if (p.name.equals(name)) {
                    if (stopped) {
                        p.status = StatusStopped;

                        List<String> services = new ArrayList<>();
                        services.add(name);
                        servicesStopped(services, _observers);
                    } else {
                        p.status = StatusStarted;
                    }
                    break;
                }
            }
            _pendingStatusChanges = false;
            notifyAll();
        }
    }

    @Override
    public void addObserver(final ServiceObserverPrx observer, Current current) {
        List<String> activeServices = new LinkedList<>();

        // Null observers and duplicate registrations are ignored

        synchronized (this) {
            if (observer != null && _observers.add(observer)) {
                if (_traceServiceObserver >= 1) {
                    _logger.trace(
                            "IceBox.ServiceObserver",
                            "Added service observer " + _communicator.proxyToString(observer));
                }

                for (ServiceInfo info : _services) {
                    if (info.status == StatusStarted) {
                        activeServices.add(info.name);
                    }
                }
            }
        }

        if (!activeServices.isEmpty()) {
            observer.servicesStartedAsync(activeServices.toArray(new String[0]))
                    .exceptionally(
                            ex -> {
                                observerFailed(observer, ex);
                                return null;
                            });
        }
    }

    @Override
    public void shutdown(Current current) {
        _communicator.shutdown();
    }

    public int run() {
        try {
            Properties properties = _communicator.getProperties();

            // Parse the property set with the prefix "IceBox.Service.". These
            // properties should have the following format:
            //
            // IceBox.Service.Foo=[jar-or-dir:]Package.Foo [args]
            //
            // We parse the service properties specified in IceBox.LoadOrder first, then the ones
            // from remaining services.
            final String prefix = "IceBox.Service.";
            Map<String, String> services = properties.getPropertiesForPrefix(prefix);

            if (services.isEmpty()) {
                throw new FailureException(
                        "ServiceManager: configuration must include at least one IceBox service");
            }

            String[] loadOrder = properties.getIcePropertyAsList("IceBox.LoadOrder");
            List<StartServiceInfo> servicesInfo = new ArrayList<>();
            for (String name : loadOrder) {
                if (!name.isEmpty()) {
                    String key = prefix + name;
                    String value = services.get(key);
                    if (value == null) {
                        throw new FailureException(
                                "ServiceManager: no service definition for `" + name + "'");
                    }
                    servicesInfo.add(new StartServiceInfo(name, value, _argv));
                    services.remove(key);
                }
            }
            for (Map.Entry<String, String> p : services.entrySet()) {
                String name = p.getKey().substring(prefix.length());
                String value = p.getValue();
                servicesInfo.add(new StartServiceInfo(name, value, _argv));
            }

            // Check if some services are using the shared communicator in which case we create the
            // shared communicator now with a property set that is the union of all the service
            // properties (from services that use the shared communicator).
            if (properties.getPropertiesForPrefix("IceBox.UseSharedCommunicator.").size() > 0) {
                InitializationData initData = new InitializationData();
                initData.properties = createServiceProperties("SharedCommunicator");
                for (StartServiceInfo service : servicesInfo) {
                    if (properties.getIcePropertyAsInt(
                                    "IceBox.UseSharedCommunicator." + service.name)
                            <= 0) {
                        continue;
                    }

                    // Load the service properties using the shared communicator properties as the
                    // default properties.
                    List<String> remainingArgs = new ArrayList<>();
                    Properties serviceProps =
                            new Properties(service.args, initData.properties, remainingArgs);
                    service.args = remainingArgs.toArray(new String[remainingArgs.size()]);

                    // Remove properties from the shared property set that a service explicitly
                    // clears.
                    Map<String, String> allProps =
                            initData.properties.getPropertiesForPrefix("");
                    for (String key : allProps.keySet()) {
                        if (serviceProps.getProperty(key).isEmpty()) {
                            initData.properties.setProperty(key, "");
                        }
                    }

                    // Add the service properties to the shared communicator properties.
                    for (Map.Entry<String, String> p :
                            serviceProps.getPropertiesForPrefix("").entrySet()) {
                        initData.properties.setProperty(p.getKey(), p.getValue());
                    }

                    // Parse <service>.* command line options (the Ice command line options
                    // were parsed by the call to createProperties above).
                    service.args =
                            initData.properties.parseCommandLineOptions(service.name, service.args);
                }

                String facetNamePrefix = "IceBox.SharedCommunicator.";
                boolean addFacets = configureAdmin(initData.properties, facetNamePrefix);

                _sharedCommunicator = Util.initialize(initData);

                if (addFacets) {
                    // Add all facets created on shared communicator to the IceBox communicator but
                    // renamed <prefix>.<facet-name>, except for the Process facet which is never
                    // added.
                    for (Map.Entry<String, Object> p :
                            _sharedCommunicator.findAllAdminFacets().entrySet()) {
                        if (!"Process".equals(p.getKey())) {
                            _communicator.addAdminFacet(p.getValue(), facetNamePrefix + p.getKey());
                        }
                    }
                }
            }

            for (StartServiceInfo s : servicesInfo) {
                start(s.name, s.className, s.classDir, s.absolutePath, s.args);
            }

            // Start Admin (if enabled).
            _communicator.addAdminFacet(this, "IceBox.ServiceManager");
            _communicator.getAdmin();

            // We may want to notify external scripts that the services have started and that IceBox
            // is "ready".
            // This is done by defining the property IceBox.PrintServicesReady=bundleName
            //
            // bundleName is whatever you choose to call this set of services. It will be echoed
            // back as "bundleName ready".
            //
            // This must be done after start() has been invoked on the services.
            String bundleName = properties.getIceProperty("IceBox.PrintServicesReady");
            if (!bundleName.isEmpty()) {
                System.out.println(bundleName + " ready");
            }

            _communicator.waitForShutdown();
        } catch (FailureException ex) {
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            pw.println(ex.toString());
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error(sw.toString());
            return 1;
        } catch (CommunicatorDestroyedException ex) {
            // Expected if the communicator is shutdown by the shutdown hook
        } catch (ObjectAdapterDeactivatedException ex) {
            // Expected if the communicator is shutdown by the shutdown hook
        } catch (ObjectAdapterDestroyedException ex) {
            // Expected
        } catch (Throwable ex) {
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error("ServiceManager: caught exception:\n" + sw.toString());
            return 1;
        } finally {
            // Invoke stop() on the services.
            stopAll();
        }

        return 0;
    }

    private synchronized void start(
            String service, String className, String classDir, boolean absolutePath, String[] args)
            throws FailureException {
        // Load the class.

        // Use a class loader if the user specified a JAR file or class directory.
        Class<?> c = null;
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

                if (!classDir.endsWith(File.separator) && !classDir.endsWith(".jar")) {
                    classDir += File.separator;
                }
                classDir = URLEncoder.encode(classDir, "UTF-8");

                // Reuse an existing class loader if we have already loaded a plug-in with the same
                // value for classDir, otherwise create a new one.
                ClassLoader cl = null;

                if (_classLoaders == null) {
                    _classLoaders = new HashMap<>();
                } else {
                    cl = _classLoaders.get(classDir);
                }

                if (cl == null) {
                    final URL[] url =
                            new URL[]{new URL("file:///" + classDir)};

                    cl = new URLClassLoader(url);

                    _classLoaders.put(classDir, cl);
                }

                c = cl.loadClass(className);
            } catch (MalformedURLException ex) {
                throw new FailureException(
                        "ServiceManager: invalid entry point format `" + classDir + "'", ex);
            } catch (IOException ex) {
                throw new FailureException(
                        "ServiceManager: invalid path in plug-in entry point `" + classDir + "'",
                        ex);
            } catch (ClassNotFoundException ex) {
                // Ignored
            }
        } else {
            c = Util.findClass(className, null);
        }

        if (c == null) {
            throw new FailureException("ServiceManager: class " + className + " not found");
        }

        ServiceInfo info = new ServiceInfo();
        info.name = service;
        info.status = StatusStopped;
        info.args = args;

        // If IceBox.UseSharedCommunicator.<name> is defined, create a
        // communicator for the service. The communicator inherits from the shared communicator
        // properties. If it's not defined, add the service properties to the shared communicator
        // property set.
        Communicator communicator;
        if (_communicator
                        .getProperties()
                        .getIcePropertyAsInt("IceBox.UseSharedCommunicator." + service)
                > 0) {
            assert (_sharedCommunicator != null);
            communicator = _sharedCommunicator;
        } else {
            try {
                // Create the service properties. We use the communicator properties as the default
                // properties if IceBox.InheritProperties is set.
                InitializationData initData = new InitializationData();
                initData.properties = createServiceProperties(service);
                String[] serviceArgs = info.args;
                if (serviceArgs.length > 0) {
                    // Create the service properties with the given service arguments. This should
                    // read the service config file if it's specified with --Ice.Config.
                    List<String> remainingArgs = new ArrayList<>();
                    initData.properties =
                            new Properties(serviceArgs, initData.properties, remainingArgs);
                    serviceArgs = remainingArgs.toArray(new String[remainingArgs.size()]);

                    // Next, parse the service "<service>.*" command line options (the Ice command
                    // line options were parsed by the createProperties above).
                    serviceArgs = initData.properties.parseCommandLineOptions(service, serviceArgs);
                }

                // Clone the logger to assign a new prefix. If one of the built-in loggers is
                // configured don't set any logger.
                if (initData.properties.getIceProperty("Ice.LogFile").isEmpty()
                        && (initData.properties.getIcePropertyAsInt("Ice.UseSyslog") <= 0
                                || System.getProperty("os.name").startsWith("Windows"))) {
                    initData.logger =
                            _logger.cloneWithPrefix(
                                    initData.properties.getIceProperty("Ice.ProgramName"));
                }

                // If Admin is enabled on the IceBox communicator, for each service that does not
                // set
                // Ice.Admin.Enabled, we set Ice.Admin.Enabled=1 to have this service create facets;
                // then
                // we add these facets to the IceBox Admin object as
                // IceBox.Service.<service>.<facet>.
                String serviceFacetNamePrefix = "IceBox.Service." + service + ".";
                boolean addFacets = configureAdmin(initData.properties, serviceFacetNamePrefix);

                // Remaining command line options are passed to the communicator. This is necessary
                // for Ice plug-in properties (e.g.: Ice.SSL).
                List<String> remainingArgs = new ArrayList<>();
                info.communicator = Util.initialize(serviceArgs, initData, remainingArgs);
                info.args = remainingArgs.toArray(new String[remainingArgs.size()]);
                communicator = info.communicator;

                if (addFacets) {
                    // Add all facets created on the service communicator to the IceBox communicator
                    // but renamed IceBox.Service.<service>.<facet-name>, except for the Process
                    // facet which is never added
                    for (Map.Entry<String, Object> p :
                            communicator.findAllAdminFacets().entrySet()) {
                        if (!"Process".equals(p.getKey())) {
                            _communicator.addAdminFacet(
                                    p.getValue(), serviceFacetNamePrefix + p.getKey());
                        }
                    }
                }
            } catch (Throwable ex) {
                throw new FailureException(
                        "ServiceManager: exception while starting service " + service, ex);
            }
        }

        try {
            // Instantiate the service.
            try {
                // If the service class provides a constructor that accepts an
                // com.zeroc.Ice.Communicator argument,
                // use that in preference to the default constructor.
                java.lang.Object obj = null;
                try {
                    java.lang.reflect.Constructor<?> con =
                            c.getDeclaredConstructor(Communicator.class);
                    obj = con.newInstance(_communicator);
                } catch (IllegalAccessException ex) {
                    throw new FailureException(
                            "ServiceManager: unable to access service constructor "
                                    + className
                                    + "(com.zeroc.Ice.Communicator)",
                            ex);
                } catch (NoSuchMethodException ex) {
                    // Ignore.
                } catch (java.lang.reflect.InvocationTargetException ex) {
                    if (ex.getCause() != null) {
                        throw ex.getCause();
                    } else {
                        throw new FailureException(
                                "ServiceManager: exception in service constructor for " + className,
                                ex);
                    }
                }

                if (obj == null) {
                    // Fall back to the default constructor.
                    try {
                        obj = c.getDeclaredConstructor().newInstance();
                    } catch (IllegalAccessException ex) {
                        throw new FailureException(
                                "ServiceManager: unable to access default service constructor in"
                                        + " class "
                                        + className,
                                ex);
                    }
                }

                try {
                    info.service = (Service) obj;
                } catch (ClassCastException ex) {
                    throw new FailureException(
                            "ServiceManager: class "
                                    + className
                                    + " does not implement com.zeroc.IceBox.Service");
                }
            } catch (InstantiationException ex) {
                throw new FailureException(
                        "ServiceManager: unable to instantiate class " + className, ex);
            } catch (FailureException ex) {
                throw ex;
            } catch (Throwable ex) {
                throw new FailureException(
                        "ServiceManager: exception in service constructor for " + className, ex);
            }

            try {
                info.service.start(service, communicator, info.args);

                // There is no need to notify the observers since the 'start all'
                // (that indirectly calls this method) occurs before the creation of
                // the Server Admin object, and before the activation of the main
                // object adapter (so before any observer can be registered)
            } catch (FailureException ex) {
                throw ex;
            } catch (Throwable ex) {
                throw new FailureException(
                        "ServiceManager: exception while starting service " + service, ex);
            }

            info.status = StatusStarted;
            _services.add(info);
        } catch (RuntimeException ex) {
            if (info.communicator != null) {
                destroyServiceCommunicator(service, info.communicator);
            }

            throw ex;
        }
    }

    private synchronized void stopAll() {
        // First wait for any active startService/stopService calls to complete.
        while (_pendingStatusChanges) {
            try {
                wait();
            } catch (InterruptedException ex) {
            }
        }

        // For each service, we call stop on the service and flush its database environment to
        // the disk. Services are stopped in the reverse order of the order they were started.
        List<String> stoppedServices = new ArrayList<>();
        ListIterator<ServiceInfo> p = _services.listIterator(_services.size());
        while (p.hasPrevious()) {
            ServiceInfo info = p.previous();
            if (info.status == StatusStarted) {
                try {
                    info.service.stop();
                    info.status = StatusStopped;
                    stoppedServices.add(info.name);
                } catch (Throwable e) {
                    StringWriter sw = new StringWriter();
                    PrintWriter pw = new PrintWriter(sw);
                    e.printStackTrace(pw);
                    pw.flush();
                    _logger.warning(
                            "ServiceManager: exception while stopping service "
                                    + info.name
                                    + ":\n"
                                    + sw.toString());
                }
            }

            if (info.communicator != null) {
                destroyServiceCommunicator(info.name, info.communicator);
            }
        }

        if (_sharedCommunicator != null) {
            removeAdminFacets("IceBox.SharedCommunicator.");

            try {
                _sharedCommunicator.destroy();
            } catch (Exception e) {
                StringWriter sw = new StringWriter();
                PrintWriter pw = new PrintWriter(sw);
                e.printStackTrace(pw);
                pw.flush();
                _logger.warning(
                        "ServiceManager: exception while destroying shared communicator:\n"
                                + sw.toString());
            }
            _sharedCommunicator = null;
        }

        _services.clear();
        servicesStopped(stoppedServices, _observers);
    }

    private void servicesStarted(
            List<String> services, Set<ServiceObserverPrx> observers) {
        if (!services.isEmpty()) {
            String[] servicesArray = services.toArray(new String[0]);

            for (final ServiceObserverPrx observer : observers) {
                observer.servicesStartedAsync(servicesArray)
                        .exceptionally(
                                ex -> {
                                    observerFailed(observer, ex);
                                    return null;
                                });
            }
        }
    }

    private void servicesStopped(
            List<String> services, Set<ServiceObserverPrx> observers) {
        if (!services.isEmpty()) {
            String[] servicesArray = services.toArray(new String[0]);

            for (final ServiceObserverPrx observer : observers) {
                observer.servicesStoppedAsync(servicesArray)
                        .exceptionally(
                                ex -> {
                                    observerFailed(observer, ex);
                                    return null;
                                });
            }
        }
    }

    private synchronized void observerFailed(ServiceObserverPrx observer, Throwable ex) {
        if (ex instanceof LocalException) {
            if (_observers.remove(observer)) {
                observerRemoved(observer, (LocalException) ex);
            }
        }
    }

    private void observerRemoved(ServiceObserverPrx observer, RuntimeException ex) {
        if (_traceServiceObserver >= 1) {
            // CommunicatorDestroyedException may occur during shutdown. The observer notification
            // has been sent, but the communicator was destroyed before the reply was received. We
            // do not log a message for this exception.
            if (!(ex instanceof CommunicatorDestroyedException)) {
                _logger.trace(
                        "IceBox.ServiceObserver",
                        "Removed service observer "
                                + _communicator.proxyToString(observer)
                                + "\nafter catching "
                                + ex.toString());
            }
        }
    }

    public static final int StatusStopping = 0;
    public static final int StatusStopped = 1;
    public static final int StatusStarting = 2;
    public static final int StatusStarted = 3;

    static final class ServiceInfo implements Cloneable {
        @Override
        public ServiceInfo clone() {
            ServiceInfo c = null;
            try {
                c = (ServiceInfo) super.clone();
            } catch (CloneNotSupportedException ex) {
            }
            return c;
        }

        public String name;
        public Service service;
        public Communicator communicator;
        public int status;
        public String[] args;
    }

    static class StartServiceInfo {
        StartServiceInfo(String service, String value, String[] serverArgs) {
            name = service;

            // We support the following formats:
            //
            // <class-name> [args]
            // <jar-file>:<class-name> [args]
            // <class-dir>:<class-name> [args]
            // "<path with spaces>":<class-name> [args]
            // "<path with spaces>:<class-name>" [args]

            try {
                args = Options.split(value);
            } catch (ParseException ex) {
                throw new FailureException(
                        "ServiceManager: invalid arguments for service `" + name + "'", ex);
            }

            assert (args.length > 0);

            final String entryPoint = args[0];

            final boolean isWindows = System.getProperty("os.name").startsWith("Windows");
            absolutePath = false;

            // Find first ':' that isn't part of the file path.
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
                // Class name is missing.
                throw new FailureException(
                        "ServiceManager: invalid entry point for service `"
                                + name
                                + "':\n"
                                + entryPoint);
            }

            // Extract the JAR file or subdirectory, if any.
            classDir = null; // Path name of JAR file or subdirectory.

            if (pos == -1) {
                className = entryPoint;
            } else {
                classDir = entryPoint.substring(0, pos).trim();
                className = entryPoint.substring(pos + 1).trim();
            }

            // Shift the arguments.
            String[] tmp = new String[args.length - 1];
            System.arraycopy(args, 1, tmp, 0, args.length - 1);
            args = tmp;

            if (serverArgs.length > 0) {
                List<String> l = new ArrayList<>(Arrays.asList(args));
                for (String arg : serverArgs) {
                    if (arg.startsWith("--" + service + ".")) {
                        l.add(arg);
                    }
                }
                args = l.toArray(args);
            }
        }

        String name;
        String[] args;
        String className;
        String classDir;
        boolean absolutePath;
    }

    private Properties createServiceProperties(String service) {
        Properties properties = new Properties();
        Properties communicatorProperties = _communicator.getProperties();
        if (communicatorProperties.getIcePropertyAsInt("IceBox.InheritProperties") > 0) {

            // Inherit all except IceBox. and Ice.Admin. properties
            for (Map.Entry<String, String> p :
                    communicatorProperties.getPropertiesForPrefix("").entrySet()) {
                String key = p.getKey();
                if (!key.startsWith("IceBox.") && !key.startsWith("Ice.Admin.")) {
                    properties.setProperty(key, p.getValue());
                }
            }
        }

        String programName = communicatorProperties.getIceProperty("Ice.ProgramName");
        if (programName.isEmpty()) {
            properties.setProperty("Ice.ProgramName", service);
        } else {
            properties.setProperty("Ice.ProgramName", programName + "-" + service);
        }
        return properties;
    }

    private void destroyServiceCommunicator(
            String service, Communicator communicator) {
        try {
            communicator.shutdown();
            communicator.waitForShutdown();
        } catch (CommunicatorDestroyedException e) {
            // Ignore, the service might have already destroyed the communicator for its own
            // reasons.
        } catch (Exception e) {
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            e.printStackTrace(pw);
            pw.flush();
            _logger.warning(
                    "ServiceManager: exception in shutting down communicator for service "
                            + service
                            + "\n"
                            + sw.toString());
        }

        removeAdminFacets("IceBox.Service." + service + ".");
        communicator.destroy();
    }

    private boolean configureAdmin(Properties properties, String prefix) {
        if (_adminEnabled && properties.getIceProperty("Ice.Admin.Enabled").isEmpty()) {
            List<String> facetNames = new LinkedList<>();
            for (String p : _adminFacetFilter) {
                if (p.startsWith(prefix)) {
                    facetNames.add(p.substring(prefix.length()));
                }
            }

            if (_adminFacetFilter.isEmpty() || !facetNames.isEmpty()) {
                properties.setProperty("Ice.Admin.Enabled", "1");

                if (!facetNames.isEmpty()) {
                    // TODO: need join with escape!
                    properties.setProperty("Ice.Admin.Facets", String.join(" ", facetNames));
                }
                return true;
            }
        }
        return false;
    }

    private void removeAdminFacets(String prefix) {
        try {
            for (String p : _communicator.findAllAdminFacets().keySet()) {
                if (p.startsWith(prefix)) {
                    _communicator.removeAdminFacet(p);
                }
            }
        } catch (CommunicatorDestroyedException ex) {
            // Ignored
        } catch (ObjectAdapterDeactivatedException ex) {
            // Ignored
        } catch (ObjectAdapterDestroyedException ex) {
            // Ignored
        }
    }

    private final Communicator _communicator;
    private boolean _adminEnabled;
    private Set<String> _adminFacetFilter;
    private Communicator _sharedCommunicator;
    private final Logger _logger;
    private final String[] _argv; // Filtered server argument vector
    private final List<ServiceInfo> _services = new LinkedList<>();
    private boolean _pendingStatusChanges;
    private final HashSet<ServiceObserverPrx> _observers = new HashSet<>();
    private final int _traceServiceObserver;
    private Map<String, ClassLoader> _classLoaders;
}
