// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBox;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

final class Server {
    static class ShutdownHook extends Thread {
        private Communicator _communicator;
        private final java.lang.Object _doneMutex = new java.lang.Object();
        private boolean _done;

        ShutdownHook(Communicator communicator) {
            _communicator = communicator;
        }

        @Override
        public void run() {
            _communicator.shutdown();

            synchronized (_doneMutex) {
                // Wait on the server to finish shutting down before exiting the ShutdownHook. This
                // ensures that all IceBox services have had a chance to shutdown cleanly before the
                // JVM terminates.
                while (!_done) {
                    try {
                        _doneMutex.wait();
                    } catch (InterruptedException ex) {
                        break;
                    }
                }
            }
        }

        public void done() {
            synchronized (_doneMutex) {
                _done = true;
                _doneMutex.notify();
            }
        }
    }

    private static void usage() {
        System.err.println("Usage: com.zeroc.IceBox.Server [options] --Ice.Config=<file>\n");
        System.err.println(
            "Options:\n"
                + "-h, --help           Show this message.\n"
                + "-v, --version        Display the Ice version.");
    }

    private static int run(Communicator communicator, List<String> argSeq) {
        final String prefix = "IceBox.Service.";
        Properties properties = communicator.getProperties();
        Map<String, String> services = properties.getPropertiesForPrefix(prefix);

        List<String> iceBoxArgs = new ArrayList<String>(argSeq);

        for (String key : services.keySet()) {
            String name = key.substring(prefix.length());
            iceBoxArgs.removeIf(v -> v.startsWith("--" + name));
        }

        for (String arg : iceBoxArgs) {
            if ("-h".equals(arg) || "--help".equals(arg)) {
                usage();
                return 0;
            } else if ("-v".equals(arg) || "--version".equals(arg)) {
                System.out.println(Util.stringVersion());
                return 0;
            } else {
                System.err.println("IceBox.Server: unknown option `" + arg + "'");
                usage();
                return 1;
            }
        }

        ServiceManagerI serviceManagerImpl =
            new ServiceManagerI(communicator, argSeq.toArray(new String[0]));
        return serviceManagerImpl.run();
    }

    public static void main(String[] args) {
        int status = 0;
        List<String> argSeq = new ArrayList<>();

        InitializationData initData = new InitializationData();
        initData.properties = new Properties(Collections.singletonList("IceBox"));
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        ShutdownHook shutdownHook = null;

        try (Communicator communicator =
            Util.initialize(args, initData, argSeq)) {
            shutdownHook = new ShutdownHook(communicator);
            Runtime.getRuntime().addShutdownHook(shutdownHook);

            status = run(communicator, argSeq);
        } finally {
            if (shutdownHook != null) {
                shutdownHook.done();
            }
        }

        System.exit(status);
    }
}
