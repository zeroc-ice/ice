// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Test.Common;

import Test.Common._ControllerDisp;
import Test.Common._ServerDisp;
import Test.Common.ServerPrx;

import java.io.File;
import java.io.BufferedReader;
import java.io.InputStreamReader;

import java.util.regex.Pattern;

public class ControllerServer extends Ice.Application
{
    class ServerI extends _ServerDisp
    {
        public ServerI(Process process, String name)
        {
            _process = process;
            _processOutput = new StringBuffer();
            _name = name;
            _started = 0;
            _terminated = false;

            class Reader extends Thread
            {
                Reader(java.io.InputStream is)
                {
                    _is = is;
                }

                public void run()
                {
                    try
                    {
                        final BufferedReader reader = new BufferedReader(new InputStreamReader(_is));

                        String line = reader.readLine();
                        while(line != null)
                        {
                            if(_started == 0)
                            {
                                _processOutput.append(line + "\n");
                            }

                            if(line.matches(Pattern.quote("starting server...") + ".*ok.*") ||
                               line.matches(Pattern.quote("starting serveramd...") + ".*ok.*") ||
                               line.matches(Pattern.quote("starting servertie...") + ".*ok.*") ||
                               line.matches(Pattern.quote("starting serveramdtie...") + ".*ok.*") ||
                               line.matches("starting test.*" + Pattern.quote("Server...") + ".*ok.*"))
                            {
                                synchronized(ServerI.this)
                                {
                                    _started++;
                                    ServerI.this.notifyAll();
                                }
                            }
                            else if(line.matches(Pattern.quote("starting server...") + ".*") ||
                                    line.matches(Pattern.quote("starting serveramd...") + ".*") ||
                                    line.matches(Pattern.quote("starting servertie...") + ".*") ||
                                    line.matches(Pattern.quote("starting serveramdtie...") + ".*") ||
                                    line.matches("starting test.*" + Pattern.quote("Server...")  + ".*"))
                            {
                                String s = reader.readLine();
                                if(s == null)
                                {
                                    System.out.println(line);
                                    break;
                                }
                                line += s;
                                continue;
                            }
                            System.out.println(line);
                            line = reader.readLine();
                        }
                    }
                    catch(java.io.IOException ex)
                    {
                        System.out.println("server start failed!\n" + ex);
                    }

                    // Make sure to unblock thread waiting for server start.
                    synchronized(ServerI.this)
                    {
                        _terminated = true;
                        ServerI.this.notifyAll();
                    }
                }

                private java.io.InputStream _is;
            }
            new Reader(_process.getInputStream()).start();
        }

        public synchronized void terminate(Ice.Current current)
        {
            try
            {
                _process.exitValue();
                return;
            }
            catch(IllegalThreadStateException ex)
            {
                //
                // process is still running.
                //
                System.out.print("terminating " + _name + "... ");
                System.out.flush();

                _process.destroy();
                while(true)
                {
                    try
                    {
                        _process.waitFor();
                        break;
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
                current.adapter.remove(current.id);
                System.out.println("ok");
            }
        }

        public void waitTestSuccess(Ice.Current current)
        {
            Process p = null;
            synchronized(this)
            {
                p = _process;
            }

            if(p != null)
            {
                while(true)
                {
                    try
                    {
                        p.waitFor();
                        break;
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }
            }
        }

        public synchronized void waitForServer(Ice.Current current)
            throws ServerFailedException
        {
            while(!_terminated)
            {
                try
                {
                    if(_started > 0)
                    {
                        _started--;
                        break;
                    }
                    wait();
                }
                catch(InterruptedException ex)
                {
                    continue;
                }
            }
            if(_terminated && _started == 0)
            {
                throw new ServerFailedException(_processOutput.toString());
            }
        }

        private Process _process;
        private StringBuffer _processOutput;
        private String _name;
        private int _started;
        private boolean _terminated;
    }

    public class ControllerI extends _ControllerDisp
    {
        public ControllerI(String[] args)
        {
            _args = args;
        }

        @Override
        public ServerPrx runServer(String lang, final String name, String protocol, String host,
                                   boolean winrt, String[] options, Ice.Current current)
        {
            if(_server != null)
            {
                try
                {
                    _server.terminate();
                }
                catch(Ice.LocalException ex)
                {
                }
            }

            String script = lang + (lang.equals("java") ? "/test/src/main/java/" : "/") + "test/" + name + "/run.py";

            java.util.List<String> args = new java.util.ArrayList<String>();
            args.add("python");
            args.add(script);
            args.add("--server");
            args.add("--protocol");
            args.add(protocol);
            args.add("--host");
            args.add(host);

            if(winrt)
            {
                args.add("--winrt");
            }

            for(String option : options)
            {
                args.add("--arg");
                args.add(option);
            }

            for(String a : _args)
            {
                args.add(a);
            }

            try
            {
                System.out.print("starting " + name + "... ");
                System.out.flush();

                final Process process = new ProcessBuilder(args)
                    .directory(_toplevel)
                    .redirectErrorStream(true)
                    .start();
                _server = ServerPrxHelper.uncheckedCast(current.adapter.addWithUUID(new ServerI(process, name)));
            }
            catch(java.io.IOException ex)
            {
                throw new RuntimeException("failed to start server `" + name + "'", ex);
            }
            return _server;
        }

        private ServerPrx _server;
        private String[] _args;
    }

    @Override
    public int
    run(String[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("ControllerAdapter");
        adapter.add(new ControllerI(args), communicator().stringToIdentity("controller"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public ControllerServer(File toplevel)
    {
        _toplevel = toplevel;
    }

    public static void
    main(String[] args)
    {
        try
        {
            File toplevel = new File(
                new File(ControllerServer.class.getProtectionDomain().getCodeSource().getLocation().toURI()).getParent(),
                "../../../../../");

            ControllerServer app = new ControllerServer(toplevel);
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(args);
            initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
            initData.properties.setProperty("IceSSL.DefaultDir", new File(toplevel, "certs").getCanonicalPath());
            initData.properties.setProperty("IceSSL.Keystore", "server.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            initData.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "10");
            initData.properties.setProperty("ControllerAdapter.Endpoints",
                                            "tcp -p 15000:ssl -p 15001: ws -p 15002:wss -p 15003");

            int status = app.main("ControllerServer", args, initData);
            System.exit(status);
        }
        catch(java.net.URISyntaxException ex)
        {
            ex.printStackTrace();
            System.exit(1);
        }
        catch(java.io.IOException ex)
        {
            ex.printStackTrace();
            System.exit(1);
        }
    }

    private final File _toplevel;
}
