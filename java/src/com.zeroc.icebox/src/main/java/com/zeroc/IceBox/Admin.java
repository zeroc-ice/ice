// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBox;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class Admin {
    private static void usage() {
        System.err.println(
                "Usage: com.zeroc.IceBox.Admin [options] [command...]\n"
                        + "Options:\n"
                        + "-h, --help          Show this message.\n"
                        + "-v, --version       Display the Ice version.\n"
                        + "\n"
                        + "Commands:\n"
                        + "start SERVICE       Start a service.\n"
                        + "stop SERVICE        Stop a service.\n"
                        + "shutdown            Shutdown the server.");
    }

    public static void main(String[] args) {
        int status = 0;
        List<String> commands = new ArrayList<>();

        InitializationData initData = new InitializationData();
        initData.properties =
                new Properties(Collections.singletonList("IceBoxAdmin"));

        try (Communicator communicator =
                Util.initialize(args, initData, commands)) {
            Runtime.getRuntime()
                    .addShutdownHook(
                            new Thread(
                                    () -> {
                                        communicator.destroy();
                                    }));

            status = run(communicator, commands);
        }

        System.exit(status);
    }

    public static int run(
            Communicator communicator, List<String> commands) {
        if (commands.isEmpty()) {
            usage();
            return 0;
        }

        for (String command : commands) {
            if ("-h".equals(command) || "--help".equals(command)) {
                usage();
                return 0;
            } else if ("-v".equals(command) || "--version".equals(command)) {
                System.out.println(Util.stringVersion());
                return 0;
            } else if (command.startsWith("-")) {
                System.err.println("IceBox.Admin: unknown option `" + command + "'");
                usage();
                return 1;
            }
        }

        ObjectPrx base =
                communicator.propertyToProxy("IceBoxAdmin.ServiceManager.Proxy");

        if (base == null) {
            System.err.println(
                    "IceBox.Admin: property 'IceBoxAdmin.ServiceManager.Proxy' is not set");
            return 1;
        }

        ServiceManagerPrx manager =
                ServiceManagerPrx.checkedCast(base);
        if (manager == null) {
            System.err.println(
                    "IceBox.Admin: '" + base.toString() + "' is not an IceBox::ServiceManager");
            return 1;
        }

        for (int i = 0; i < commands.size(); i++) {
            String command = commands.get(i);
            if ("shutdown".equals(command)) {
                manager.shutdown();
            } else if ("start".equals(command)) {
                if (++i >= commands.size()) {
                    System.err.println("IceBox.Admin: no service name specified.");
                    return 1;
                }

                String service = commands.get(i);
                try {
                    manager.startService(service);
                } catch (NoSuchServiceException ex) {
                    System.err.println("IceBox.Admin: unknown service `" + service + "'");
                    return 1;
                } catch (AlreadyStartedException ex) {
                    System.err.println("IceBox.Admin: service already started.");
                }
            } else if ("stop".equals(command)) {
                if (++i >= commands.size()) {
                    System.err.println("IceBox.Admin: no service name specified.");
                    return 1;
                }

                String service = commands.get(i);
                try {
                    manager.stopService(service);
                } catch (NoSuchServiceException ex) {
                    System.err.println("IceBox.Admin: unknown service `" + service + "'");
                    return 1;
                } catch (AlreadyStoppedException ex) {
                    System.err.println("IceBox.Admin: service already stopped.");
                }
            } else {
                System.err.println("IceBox.Admin: unknown command `" + command + "'");
                usage();
                return 1;
            }
        }

        return 0;
    }
}
