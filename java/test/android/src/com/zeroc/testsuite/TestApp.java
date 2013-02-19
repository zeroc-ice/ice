// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.testsuite;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.Writer;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import test.Util.Application.CommunicatorListener;
import Ice.Communicator;
import android.app.Application;
import android.os.Handler;
import android.os.Build.VERSION;

public class TestApp extends Application
{
    static private class TestSuiteEntry
    {
        TestSuiteEntry(String name, Class<? extends test.Util.Application> client,
                       Class<? extends test.Util.Application> server,
                       Class<? extends test.Util.Application> collocated)
        {
            _name = name;
            _client = client;
            _server = server;
            _collocated = collocated;
        }

        String getName()
        {
            return _name;
        }

        test.Util.Application getClient()
            throws IllegalAccessException, InstantiationException
        {
            if(_client == null)
            {
                return null;
            }
            return _client.newInstance();
        }

        test.Util.Application getServer()
            throws IllegalAccessException, InstantiationException
        {
            if(_server == null)
            {
                return null;
            }

            return _server.newInstance();
        }

        test.Util.Application getCollocated()
            throws IllegalAccessException, InstantiationException
        {
            if(_collocated == null)
            {
                return null;
            }

            return _collocated.newInstance();
        }

        private String _name;
        private Class<? extends test.Util.Application> _client;
        private Class<? extends test.Util.Application> _server;
        private Class<? extends test.Util.Application> _collocated;
    }

    static final private TestSuiteEntry[] _tests =
    {
        new TestSuiteEntry("adapterDeactivation", test.Ice.adapterDeactivation.Client.class,
                           test.Ice.adapterDeactivation.Server.class, test.Ice.adapterDeactivation.Collocated.class),
        new TestSuiteEntry("admin", test.Ice.admin.Client.class, test.Ice.admin.Server.class, null),
        new TestSuiteEntry("ami", test.Ice.ami.Client.class, test.Ice.ami.Server.class, null),
        new TestSuiteEntry("binding", test.Ice.binding.Client.class, test.Ice.binding.Server.class, null),
        new TestSuiteEntry("checksum", test.Ice.checksum.Client.class, test.Ice.checksum.Server.class, null),
        new TestSuiteEntry("classLoader", test.Ice.classLoader.Client.class, test.Ice.classLoader.Server.class, null),
        new TestSuiteEntry("custom", test.Ice.custom.Client.class, test.Ice.custom.Server.class,
                           test.Ice.custom.Collocated.class),
        new TestSuiteEntry("defaultServant", test.Ice.defaultServant.Client.class, null, null),
        new TestSuiteEntry("defaultValue", test.Ice.defaultValue.Client.class, null, null),
        new TestSuiteEntry("dispatcher", test.Ice.dispatcher.Client.class, test.Ice.dispatcher.Server.class, null),
        new TestSuiteEntry("enums", test.Ice.enums.Client.class, test.Ice.enums.Server.class, null),
        new TestSuiteEntry("exceptions", test.Ice.exceptions.Client.class, test.Ice.exceptions.Server.class,
                           test.Ice.exceptions.Collocated.class),
        new TestSuiteEntry("facets", test.Ice.facets.Client.class, test.Ice.facets.Server.class,
                           test.Ice.facets.Collocated.class),
        // Require SSL
        //new TestSuiteEntry("hash", test.Ice.hash.Client.class, null, null),
        new TestSuiteEntry("hold", test.Ice.hold.Client.class, test.Ice.hold.Server.class, null),
        // The info test is not currently enabled - it relies on sockets to accurately return
        // address and port information, which really only works in Android 2.3+.
        //new TestSuiteEntry("info", test.Ice.info.Client.class, test.Ice.info.Server.class, null),
        new TestSuiteEntry("inheritance", test.Ice.inheritance.Client.class, test.Ice.inheritance.Server.class,
                           test.Ice.inheritance.Collocated.class),
        new TestSuiteEntry("interceptor", test.Ice.interceptor.Client.class, null, null),
        new TestSuiteEntry("invoke", test.Ice.invoke.Client.class, test.Ice.invoke.Server.class, null),
        new TestSuiteEntry("location", test.Ice.location.Client.class, test.Ice.location.Server.class, null),
        // The metrics test uses too much memory.
        //new TestSuiteEntry("metrics", test.Ice.metrics.Client.class, test.Ice.metrics.Server.class, null),
        new TestSuiteEntry("objects", test.Ice.objects.Client.class, test.Ice.objects.Server.class,
                           test.Ice.objects.Collocated.class),
        new TestSuiteEntry("operations", test.Ice.operations.Client.class, test.Ice.operations.Server.class,
                           test.Ice.operations.Collocated.class),
        new TestSuiteEntry("optional", test.Ice.optional.Client.class, test.Ice.optional.Server.class, null),
        new TestSuiteEntry("packagemd", test.Ice.packagemd.Client.class, test.Ice.packagemd.Server.class, null),
        new TestSuiteEntry("proxy", test.Ice.proxy.Client.class, test.Ice.proxy.Server.class,
                           test.Ice.proxy.Collocated.class),
        new TestSuiteEntry("retry", test.Ice.retry.Client.class, test.Ice.retry.Server.class, null),
        new TestSuiteEntry("seqMapping", test.Ice.seqMapping.Client.class, test.Ice.seqMapping.Server.class,
                           test.Ice.seqMapping.Collocated.class),
        new TestSuiteEntry("serialize", test.Ice.serialize.Client.class, test.Ice.serialize.Server.class, null),
        new TestSuiteEntry("servantLocator", test.Ice.servantLocator.Client.class,
                           test.Ice.servantLocator.Server.class, test.Ice.servantLocator.Collocated.class),
        new TestSuiteEntry("slicing/exceptions", test.Ice.slicing.exceptions.Client.class,
                           test.Ice.slicing.exceptions.Server.class, null),
        new TestSuiteEntry("slicing/objects", test.Ice.slicing.objects.Client.class,
                           test.Ice.slicing.objects.Server.class, null),
        new TestSuiteEntry("stream", test.Ice.stream.Client.class, null, null),
        // The throughput test uses too much memory.
        //new TestSuiteEntry("throughput", test.Ice.throughput.Client.class, test.Ice.throughput.Server.class, null),
        new TestSuiteEntry("timeout", test.Ice.timeout.Client.class, test.Ice.timeout.Server.class, null),
    };

    static final private TestSuiteEntry[] _ssltests =
    {
        new TestSuiteEntry("adapterDeactivation", test.Ice.adapterDeactivation.Client.class,
                           test.Ice.adapterDeactivation.Server.class, test.Ice.adapterDeactivation.Collocated.class),
        new TestSuiteEntry("ami", test.Ice.ami.Client.class, test.Ice.ami.Server.class, null),
        new TestSuiteEntry("binding", test.Ice.binding.Client.class, test.Ice.binding.Server.class, null),
        new TestSuiteEntry("checksum", test.Ice.checksum.Client.class, test.Ice.checksum.Server.class, null),
        new TestSuiteEntry("classLoader", test.Ice.classLoader.Client.class, test.Ice.classLoader.Server.class, null),
        new TestSuiteEntry("custom", test.Ice.custom.Client.class, test.Ice.custom.Server.class,
                           test.Ice.custom.Collocated.class),
        new TestSuiteEntry("defaultServant", test.Ice.defaultServant.Client.class, null, null),
        new TestSuiteEntry("defaultValue", test.Ice.defaultValue.Client.class, null, null),
        new TestSuiteEntry("dispatcher", test.Ice.dispatcher.Client.class, test.Ice.dispatcher.Server.class, null),
        new TestSuiteEntry("enums", test.Ice.enums.Client.class, test.Ice.enums.Server.class, null),
        new TestSuiteEntry("exceptions", test.Ice.exceptions.Client.class, test.Ice.exceptions.Server.class,
                           test.Ice.exceptions.Collocated.class),
        new TestSuiteEntry("facets", test.Ice.facets.Client.class, test.Ice.facets.Server.class,
                           test.Ice.facets.Collocated.class),
        // The hash test uses too much memory.
        //new TestSuiteEntry("hash", test.Ice.hash.Client.class, null, null),
        new TestSuiteEntry("hold", test.Ice.hold.Client.class, test.Ice.hold.Server.class, null),
        // The info test is not currently enabled - it relies on sockets to accurately return
        // address and port information, which really only works in Android 2.3+.
        //new TestSuiteEntry("info", test.Ice.info.Client.class, test.Ice.info.Server.class, null),
        new TestSuiteEntry("inheritance", test.Ice.inheritance.Client.class, test.Ice.inheritance.Server.class,
                           test.Ice.inheritance.Collocated.class),
        new TestSuiteEntry("interceptor", test.Ice.interceptor.Client.class, null, null),
        new TestSuiteEntry("invoke", test.Ice.invoke.Client.class, test.Ice.invoke.Server.class, null),
        new TestSuiteEntry("location", test.Ice.location.Client.class, test.Ice.location.Server.class, null),
        // The metrics test uses too much memory.
        //new TestSuiteEntry("metrics", test.Ice.metrics.Client.class, test.Ice.metrics.Server.class, null),
        new TestSuiteEntry("objects", test.Ice.objects.Client.class, test.Ice.objects.Server.class,
                           test.Ice.objects.Collocated.class),
        new TestSuiteEntry("operations", test.Ice.operations.Client.class, test.Ice.operations.Server.class,
                           test.Ice.operations.Collocated.class),
        new TestSuiteEntry("optional", test.Ice.optional.Client.class, test.Ice.optional.Server.class, null),
        new TestSuiteEntry("packagemd", test.Ice.packagemd.Client.class, test.Ice.packagemd.Server.class, null),
        new TestSuiteEntry("proxy", test.Ice.proxy.Client.class, test.Ice.proxy.Server.class,
                           test.Ice.proxy.Collocated.class),
        new TestSuiteEntry("retry", test.Ice.retry.Client.class, test.Ice.retry.Server.class, null),
        new TestSuiteEntry("serialize", test.Ice.serialize.Client.class, test.Ice.serialize.Server.class, null),
        new TestSuiteEntry("seqMapping", test.Ice.seqMapping.Client.class, test.Ice.seqMapping.Server.class,
                           test.Ice.seqMapping.Collocated.class),
        new TestSuiteEntry("servantLocator", test.Ice.servantLocator.Client.class,
                           test.Ice.servantLocator.Server.class, test.Ice.servantLocator.Collocated.class),
        new TestSuiteEntry("slicing/exceptions", test.Ice.slicing.exceptions.Client.class,
                           test.Ice.slicing.exceptions.Server.class, null),
        new TestSuiteEntry("slicing/objects", test.Ice.slicing.objects.Client.class,
                           test.Ice.slicing.objects.Server.class, null),
        new TestSuiteEntry("stream", test.Ice.stream.Client.class, null, null),
        new TestSuiteEntry("timeout", test.Ice.timeout.Client.class, test.Ice.timeout.Server.class, null),
    };
    private TestSuiteEntry[] _curtests = _tests;
    
    class MyWriter extends Writer
    {
        @Override
        public void close()
            throws IOException
        {
            flush();
        }

        @Override
        public void flush()
            throws IOException
        {
            final String s = _data.toString();
            if(s.length() > 0)
            {
                postOnOutput(s);
            }
            _data = new StringBuffer();
        }

        @Override
        public void write(char[] buf, int offset, int count)
            throws IOException
        {
            _data.append(buf, offset, count);
        }

        private StringBuffer _data = new StringBuffer();
    }

    private Handler _handler;
    private LinkedList<String> _strings = new LinkedList<String>();

    public interface TestListener
    {
        public void onStartTest(String test);

        public void onOutput(String s);

        public void onComplete(int status);
    }

    private TestListener _listener = null;

    private boolean _complete = false;
    private int _status = 0;
    private int _currentTest = -1;

    private boolean _ssl = false;
    private boolean _sslInitialized = false;
    private boolean _sslSupported = false;
    private boolean _ipv6 = false;
    private SSLContext _clientContext = null;
    private SSLContext _serverContext = null;
    private SSLInitializationListener _ssllistener;

    static abstract class TestThread extends Thread
    {
        test.Util.Application _app;
        protected int _status;

        TestThread(test.Util.Application app)
        {
            _app = app;
        }

        public int getStatus()
        {
            return _status;
        }
        
        protected String[] setupAddress(String[] args, boolean ipv6)
        {
            if(ipv6)
            {
                String[] ipv6Args =
                {
                    "--Ice.Default.Host=0:0:0:0:0:0:0:1",
                    "--Ice.IPv4=1",
                    "--Ice.IPv6=1",
                    "--Ice.PreferIPv6Address=1",
                };
                
                String[] nargs = new String[args.length + ipv6Args.length];
                System.arraycopy(args, 0, nargs, 0, args.length);
                System.arraycopy(ipv6Args, 0, nargs, args.length, ipv6Args.length);
                return nargs;
            }
            else
            {
                String[] ipv4Args =
                {
                    "--Ice.Default.Host=127.0.0.1",
                    "--Ice.IPv4=1",
                    "--Ice.IPv6=0"
                };
                
                String[] nargs = new String[args.length + ipv4Args.length];
                System.arraycopy(args, 0, nargs, 0, args.length);
                System.arraycopy(ipv4Args, 0, nargs, args.length, ipv4Args.length);
                return nargs;
            }
        }

        protected String[] setupssl(String[] args, final SSLContext context)
        {
            String[] sslargs =
            {
                "--Ice.Plugin.IceSSL=IceSSL.PluginFactory", "--Ice.Default.Protocol=ssl", "--Ice.InitPlugins=0"
            };

            //
            // Froyo apparently still suffers from Harmony bug 6047, requiring that we
            // disable server-side verification of client certificates.
            //
            if(VERSION.SDK_INT == 8) // android.os.Build.VERSION_CODES.FROYO (8)
            {
                String[] arr = new String[sslargs.length + 1];
                System.arraycopy(sslargs, 0, arr, 0, sslargs.length);
                arr[arr.length - 1] = "--IceSSL.VerifyPeer=0";
                sslargs = arr;
            }

            String[] nargs = new String[args.length + sslargs.length];
            System.arraycopy(args, 0, nargs, 0, args.length);
            System.arraycopy(sslargs, 0, nargs, args.length, sslargs.length);
            args = nargs;
            _app.setCommunicatorListener(new CommunicatorListener()
            {
                public void communicatorInitialized(Communicator c)
                {
                    IceSSL.Plugin plugin = (IceSSL.Plugin)c.getPluginManager().getPlugin("IceSSL");
                    plugin.setContext(context);
                    c.getPluginManager().initializePlugins();
                }
            });
            return args;
        }
    }

    class ClientThread extends TestThread
    {
        private test.Util.Application _server;

        ClientThread(test.Util.Application c, test.Util.Application s)
        {
            super(c);
            _server = s;
            setName("ClientThread");
        }

        public void run()
        {
            String[] args =
            {
                "--Ice.NullHandleAbort=1", "--Ice.Warn.Connections=1"
            };
            
            args = setupAddress(args, _ipv6);
            
            if(_ssl)
            {
                args = setupssl(args, _clientContext);
            }
            _status = _app.main("Client", args);
            // If the client failed, then stop the server -- the test is over.
            if(_status != 0 && _server != null)
            {
                _server.stop();
            }
        }
    }

    class ServerThread extends TestThread
    {
        private test.Util.Application _client;
        private ClientThread _clientThread;

        ServerThread(test.Util.Application c, test.Util.Application s)
        {
            super(s);
            setName("ServerThread");
            _client = c;
        }

        public void run()
        {
            String[] args =
            {
                "--Ice.NullHandleAbort=1",
                "--Ice.Warn.Connections=1",
                "--Ice.ThreadPool.Server.Size=1",
                "--Ice.ThreadPool.Server.SizeMax=3",
                "--Ice.ThreadPool.Server.SizeWarn=0"
            };
            
            args = setupAddress(args, _ipv6);
            
            if(_ssl)
            {
                args = setupssl(args, _serverContext);
            }
            _app.setServerReadyListener(new test.Util.Application.ServerReadyListener()
            {
                public void serverReady()
                {
                    if(_client != null)
                    {
                        _clientThread = new ClientThread(_client, _app);
                        _clientThread.start();
                    }
                }
            });

            _status = _app.main("Server", args);
            if(_clientThread != null)
            {
                while(_clientThread.isAlive())
                {
                    try
                    {
                        _clientThread.join();
                    }
                    catch(InterruptedException e1)
                    {
                    }
                }
                if(_clientThread.getStatus() != 0)
                {
                    _status = _clientThread.getStatus();
                }
            }
        }
    }

    class CollocatedThread extends TestThread
    {
        CollocatedThread(test.Util.Application c)
        {
            super(c);
            setName("CollocatedThread");
        }

        public void run()
        {
            String[] args =
            {
                "--Ice.NullHandleAbort=1"
            };
            
            args = setupAddress(args, _ipv6);
            
            if(_ssl)
            {
                args = setupssl(args, _clientContext);
            }
            _status = _app.main("Collocated", args);
        }
    }

    class TestRunner extends Thread
    {
        private List<TestThread> _threads;

        TestRunner(List<TestThread> l)
        {
            _threads = l;
        }

        public void run()
        {
            for(TestThread t : _threads)
            {
                t.start();
                while(t.isAlive())
                {
                    try
                    {
                        t.join();
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
                int status = t.getStatus();
                if(status != 0)
                {
                    postOnComplete(status);
                    return;
                }
            }
            postOnComplete(0);
        }
    }

    synchronized private void postOnOutput(final String s)
    {
        _strings.add(s);
        if(_listener != null)
        {
            final TestListener l = _listener;
            _handler.post(new Runnable()
            {
                public void run()
                {
                    l.onOutput(s);
                }
            });
        }
    }

    synchronized private void postOnComplete(final int status)
    {
        _status = status;
        _complete = true;
        if(_listener != null)
        {
            final TestListener l = _listener;
            _handler.post(new Runnable()
            {
                public void run()
                {
                    l.onComplete(status);
                }
            });
        }
    }
    
    public interface SSLInitializationListener
    {
        public void onComplete();

        public void onError();

        public void onWait();
    }

    // This is called from the SSL initialization thread.
    synchronized private void sslContextInitialized(SSLContext clientContext, SSLContext serverContext)
    {
        _clientContext = clientContext;
        _serverContext = serverContext;
        _sslInitialized = true;

        if(_ssllistener != null)
        {
            final SSLInitializationListener listener = _ssllistener;
            if(_clientContext == null | _serverContext == null)
            {
                _handler.post(new Runnable()
                {
                    public void run()
                    {
                        listener.onError();
                    }
                });
            }
            else
            {
                _handler.post(new Runnable()
                {
                    public void run()
                    {
                        listener.onComplete();
                    }
                });
            }
        }
    }

    @Override
    public void onCreate()
    {
        _handler = new Handler();

        if(VERSION.SDK_INT == 8) // android.os.Build.VERSION_CODES.FROYO (8)
        {
            //
            // Workaround for a bug in Android 2.2 (Froyo).
            //
            // See http://code.google.com/p/android/issues/detail?id=9431
            //
            java.lang.System.setProperty("java.net.preferIPv4Stack", "true");
            java.lang.System.setProperty("java.net.preferIPv6Addresses", "false");
        }

        //
        // The SSLEngine class only works properly in Froyo (or later).
        //
        _sslSupported = VERSION.SDK_INT >= 8;
    }

    @Override
    public void onTerminate()
    {
    }

    public List<String> getTestNames()
    {
        List<String> s = new ArrayList<String>();
        for(TestSuiteEntry t : _curtests)
        {
            s.add(t.getName());
        }
        return s;
    }

    synchronized public void setTestListener(TestListener listener)
    {
        _listener = listener;
        if(_listener != null && _currentTest != -1)
        {
            _listener.onStartTest(_curtests[_currentTest].getName());
            for(String s : _strings)
            {
                _listener.onOutput(s);
            }
            if(_complete)
            {
                _listener.onComplete(_status);
            }
        }
    }

    public void startNextTest()
    {
        assert _complete;
        startTest((_currentTest + 1) % _curtests.length);
    }

    synchronized public void startTest(int position)
    {
        assert !_ssl || (_ssl && _sslInitialized);

        PrintWriter pw = new PrintWriter(new MyWriter());

        _currentTest = position;
        _complete = false;
        _strings.clear();

        TestSuiteEntry entry = _curtests[position];
        test.Util.Application client;
        test.Util.Application server;
        test.Util.Application collocated;

        if(_listener != null)
        {
            _listener.onStartTest(entry.getName());
        }

        try
        {
            client = entry.getClient();
            server = entry.getServer();
            collocated = entry.getCollocated();
        }
        catch(IllegalAccessException e)
        {
            e.printStackTrace(pw);
            postOnComplete(-1);
            return;
        }
        catch(InstantiationException e)
        {
            e.printStackTrace(pw);
            postOnComplete(-1);
            return;
        }

        List<TestThread> l = new ArrayList<TestThread>();
        if(server != null)
        {
            server.setWriter(new MyWriter());
            // All servers must have a client.
            assert client != null;
            client.setWriter(new MyWriter());    
            l.add(new ServerThread(client, server));
            if(collocated != null)
            {
                collocated.setWriter(new MyWriter());
                l.add(new CollocatedThread(collocated));
            }
        }
        else
        {
            client.setWriter(new MyWriter());
            l.add(new ClientThread(client, null));
        }
        TestRunner r = new TestRunner(l);
        r.setDaemon(true);
        r.start();
    }

    public boolean isSSLSupported()
    {
        return _sslSupported;
    }
    
    public void setIPv6(boolean ipv6)
    {
        _ipv6 = ipv6;
    }

    public void setSSL(boolean ssl)
    {
        assert(!ssl || (ssl && _sslSupported));
        _ssl = ssl;
        if(_ssl)
        {
            _curtests = _ssltests;
        }
        else
        {
            _curtests = _tests;
        }
        if(_currentTest > _curtests.length-1)
        {
            _currentTest = _curtests.length-1;
        }

        if(_ssl && !_sslInitialized)
        {
            if(_ssllistener != null)
            {
                _ssllistener.onWait();
            }
            Runnable r = new Runnable()
            {
                private SSLContext initializeContext(java.io.InputStream cert)
                    throws NoSuchAlgorithmException, KeyStoreException, IOException, CertificateException,
                    FileNotFoundException, UnrecoverableKeyException, KeyManagementException
                {
                    SSLContext context = SSLContext.getInstance("TLS");
                    KeyManagerFactory kmf = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());

                    KeyStore ks = KeyStore.getInstance("BKS");
                    char[] passphrase = "password".toCharArray();
                    ks.load(cert, passphrase);
                    kmf.init(ks, passphrase);

                    TrustManagerFactory tmf =
                        TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
                    tmf.init(ks);

                    context.init(kmf.getKeyManagers(), tmf.getTrustManagers(), null);
                    return context;
                }

                public void run()
                {
                    SSLContext clientContext = null;
                    SSLContext serverContext = null;
                    try
                    {
                        clientContext = initializeContext(getResources().openRawResource(R.raw.client));
                        serverContext = initializeContext(getResources().openRawResource(R.raw.server));
                    }
                    catch(Exception ex)
                    {
                        ex.printStackTrace();
                    }
                    sslContextInitialized(clientContext, serverContext);
                }
            };

            Thread t = new Thread(r);
            t.setName("Initialize SSL Thread");
            t.setDaemon(true);
            t.start();
        }
    }

    synchronized public void setSSLInitializationListener(SSLInitializationListener listener)
    {
        _ssllistener = listener;
        if(_ssl)
        {
            if(!_sslInitialized)
            {
                listener.onWait();
            }
            else if(_clientContext == null || _serverContext == null)
            {
                listener.onError();
            }
            else
            {
                listener.onComplete();
            }
        }
    }
}
