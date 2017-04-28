// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.testsuite;

import java.io.*;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.util.*;

import com.zeroc.Ice.Logger;
import com.zeroc.Ice.Communicator;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import dalvik.system.DexClassLoader;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.Log;
import android.app.Application;
import android.os.Handler;
import android.os.Build.VERSION;

import test.Util.Application.CommunicatorListener;

public class TestApp extends Application
{
    private final String TAG = "TestApp";

    static private class TestSuiteBundle
    {
        @SuppressWarnings("unchecked")
        TestSuiteBundle(String name, ClassLoader loader)
        {
            _name = name;
            _loader = loader;

            String className = "test.Ice." + _name.replace('_', '.');
            try
            {
                _client = (Class<? extends test.Util.Application>)_loader.loadClass(className + ".Client");
            }
            catch(ClassNotFoundException e)
            {
            }
            try
            {
                _server = (Class<? extends test.Util.Application>)_loader.loadClass(className + ".Server");
            }
            catch(ClassNotFoundException e)
            {
            }
            try
            {
                _collocated = (Class<? extends test.Util.Application>)_loader.loadClass(className + ".Collocated");
            }
            catch(ClassNotFoundException e)
            {
            }
        }

        ClassLoader getClassLoader()
        {
            return _loader;
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
        private ClassLoader _loader;
        private Class<? extends test.Util.Application> _client;
        private Class<? extends test.Util.Application> _server;
        private Class<? extends test.Util.Application> _collocated;
    }

    private Map<String, TestSuiteBundle> _bundles = new HashMap<String, TestSuiteBundle>();

    static final private String _allTests[] =
    {
        "acm",
        "adapterDeactivation",
        "admin",
        "ami",
        "background",
        "binding",
        "checksum",
        "classLoader",
        "custom",
        "defaultServant",
        "defaultValue",
        "dispatcher",
        // Echo test is only for client side only mappings.
        "enums",
        "exceptions",
        "facets",
        // Fault tolerance test requires a special setup, so we don't support it.
        // hash requires too much memory.
        //"hash",
        "hold",
        "info",
        "inheritance",
        "interceptor",
        "interrupt",
        "invoke",
        "location",
        // The metrics test uses too much memory.
        //"metrics",
        // networkProxy support isn't supported due to lack of SOCKS support.
        // "networkProxy",
        "objects",
        "operations",
        "optional",
        "packagemd",
        //"plugin",
        // Properties test isn't supported since it loads files.
        "proxy",
        "retry",
        "seqMapping",
        "serialize",
        "servantLocator",
        "slicing_exceptions",
        "slicing_objects",
        "stream",
        "threadPoolPriority",
        // The throughput test uses too much memory.
        "timeout",
        "udp"
    };

    static final private String _tcpUnsupportedTestsValues[] =
    {
    };
    static final private Set<String> _tcpUnsupportedTests =
        new HashSet<String>(Arrays.asList(_tcpUnsupportedTestsValues));

    static final private String _sslUnsupportedTestsValues[] =
    {
        "binding",
        "udp"
    };
    static final private Set<String> _sslUnsupportedTests =
        new HashSet<String>(Arrays.asList(_sslUnsupportedTestsValues));

    static final private String _ipv6UnsupportedTestsValues[] =
    {
        "info" // This test doesn't currently support IPv6.
    };
    static final private Set<String> _ipv6UnsupportedTests =
        new HashSet<String>(Arrays.asList(_ipv6UnsupportedTestsValues));

    private List<String> _tests = new ArrayList<String>();

    class LogOutput
    {
        private BufferedWriter _writer;

        LogOutput()
        {
            reopen();
        }

        void reopen()
        {
            try
            {
                if(_writer != null)
                {
                    _writer.close();
                    _writer = null;
                }
                File logFile = new File(getFilesDir(), "log.txt");
                if(!logFile.exists())
                {
                    logFile.createNewFile();
                }
                _writer = new BufferedWriter(new FileWriter(logFile, false));
            }
            catch(IOException e)
            {
                Log.i(TAG, "cannot open log file", e);
            }
        }

        void write(StringBuilder message, boolean indent)
        {
            if(_writer == null)
            {
                return;
            }

            if(indent)
            {
                int idx = 0;
                while((idx = message.indexOf("\n", idx)) != -1)
                {
                    message.insert(idx + 1, "   ");
                    ++idx;
                }
            }
            synchronized(this)
            {
                try
                {
                    _writer.newLine();
                    _writer.append(message.toString());
                    _writer.flush();
                }
                catch(java.io.IOException ex)
                {
                }
            }
        }
    }
    private LogOutput _logOutput;

    class AndroidLogger implements com.zeroc.Ice.Logger
    {
        private final String _prefix;
        private String _formattedPrefix = "";
        private final java.text.DateFormat _date = java.text.DateFormat.getDateInstance(java.text.DateFormat.SHORT);
        private final java.text.SimpleDateFormat _time = new java.text.SimpleDateFormat(" HH:mm:ss:SSS", Locale.US);

        AndroidLogger(String prefix)
        {
            _prefix = prefix;
            if(prefix.length() > 0)
            {
                _formattedPrefix = prefix + ": ";
            }
        }

        @Override
        public void print(String message)
        {
            if(_logOutput != null)
            {
                StringBuilder s = new StringBuilder(256);
                s.append("-- ");
                synchronized(this)
                {
                    s.append(_date.format(new java.util.Date()));
                    s.append(_time.format(new java.util.Date()));
                }
                s.append(' ');
                s.append(_formattedPrefix);
                s.append(message);
                _logOutput.write(s, false);
            }

            Log.d(TAG, message);
        }

        @Override
        public void trace(String category, String message)
        {
            if(_logOutput != null)
            {
                StringBuilder s = new StringBuilder(256);
                s.append("-- ");
                synchronized(this)
                {
                    s.append(_date.format(new java.util.Date()));
                    s.append(_time.format(new java.util.Date()));
                }
                s.append(' ');
                s.append(_formattedPrefix);
                s.append(category);
                s.append(": ");
                s.append(message);
                _logOutput.write(s, true);
            }

            Log.v(category, message);
        }

        @Override
        public void warning(String message)
        {
            if(_logOutput != null)
            {
                StringBuilder s = new StringBuilder(256);
                s.append("-! ");
                synchronized(this)
                {
                    s.append(_date.format(new java.util.Date()));
                    s.append(_time.format(new java.util.Date()));
                }
                s.append(' ');
                s.append(_formattedPrefix);
                s.append("warning: ");
                s.append(Thread.currentThread().getName());
                s.append(": ");
                s.append(message);
                _logOutput.write(s, true);
            }

            Log.w(TAG, message);
        }

        @Override
        public void error(String message)
        {
            if(_logOutput != null)
            {
                StringBuilder s = new StringBuilder(256);
                s.append("!! ");
                synchronized(this)
                {
                    s.append(_date.format(new java.util.Date()));
                    s.append(_time.format(new java.util.Date()));
                }
                s.append(' ');
                s.append(_formattedPrefix);
                s.append("error: ");
                s.append(Thread.currentThread().getName());
                s.append(": ");
                s.append(message);
                _logOutput.write(s, true);
            }

            Log.e(TAG, message);
        }

        @Override
        public String getPrefix()
        {
            return _prefix;
        }

        @Override
        public Logger cloneWithPrefix(String s)
        {
            return new AndroidLogger(s);
        }
    }

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

    enum Mode
    {
        TCP,
        SSL,
        WS,
        WSS;

        public boolean isSSL()
        {
            return this == SSL || this == WSS;
        }

        public void setupProtocol(List<String> args)
        {
            switch(this)
            {
                case TCP:
                    break;

                case SSL:
                    args.add("--Ice.Default.Protocol=ssl");
                    break;

                case WS:
                    args.add("--Ice.Default.Protocol=ws");
                    break;

                case WSS:
                    args.add("--Ice.Default.Protocol=wss");
                    break;
            }
        }
    }

    private Mode _mode = Mode.TCP;
    private boolean _sslInitialized = false;
    private boolean _ipv6 = false;
    private SSLContext _clientContext = null;
    private SSLContext _serverContext = null;
    private SSLInitializationListener _sslListener;

    static abstract class TestThread extends Thread
    {
        String _testName;
        test.Util.Application _app;
        protected int _status;

        TestThread(String testName, ClassLoader classLoader, test.Util.Application app)
        {
            _testName = testName;
            _app = app;
            _app.setClassLoader(classLoader);
        }

        public int getStatus()
        {
            return _status;
        }

        protected void setupAddress(List<String> args, boolean ipv6)
        {
            if(ipv6)
            {
                String[] ipv6Args =
                {
                    "--Ice.Default.Host=0:0:0:0:0:0:0:1",
                    "--Ice.IPv4=1",
                    "--Ice.IPv6=1",
                    "--Ice.PreferIPv6Address=1"
                };

                args.addAll(Arrays.asList(ipv6Args));
            }
            else
            {
                String[] ipv4Args =
                {
                    "--Ice.Default.Host=127.0.0.1",
                    "--Ice.IPv4=1",
                    "--Ice.IPv6=0",
                };
                args.addAll(Arrays.asList(ipv4Args));
            }
        }

        protected void setupSSL(List<String> args, final SSLContext context)
        {
            String[] sslargs =
            {
                "--Ice.Plugin.IceSSL=com.zeroc.IceSSL.PluginFactory",
                "--Ice.InitPlugins=0"
            };
            args.addAll(Arrays.asList(sslargs));

            // SDK versions < 21 only support TLSv1 with SSLEngine.
            if(VERSION.SDK_INT < 21)
            {
                args.add("--IceSSL.Protocols=tls1_0");
            }

            _app.setCommunicatorListener(new CommunicatorListener()
            {
                public void communicatorInitialized(Communicator c)
                {
                    com.zeroc.IceSSL.Plugin plugin = (com.zeroc.IceSSL.Plugin)c.getPluginManager().getPlugin("IceSSL");
                    plugin.setContext(context);
                    c.getPluginManager().initializePlugins();
                }
            });
        }
    }

    class ClientThread extends TestThread
    {
        private test.Util.Application _server;

        ClientThread(String testName, ClassLoader classLoader, test.Util.Application c, test.Util.Application s)
        {
            super(testName, classLoader, c);
            _server = s;
            setName("ClientThread");
        }

        public void run()
        {
            String[] defaultArgs =
            {
                "--Ice.NullHandleAbort=1",
                "--Ice.Warn.Connections=1"
            };
            List<String> args = new ArrayList<String>(Arrays.asList(defaultArgs));

            setupAddress(args, _ipv6);

            if(_mode.isSSL())
            {
                setupSSL(args, _clientContext);
            }
            _mode.setupProtocol(args);
            _status = _app.main("Client", args.toArray(new String[0]));
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

        ServerThread(String testName, ClassLoader classLoader, test.Util.Application c, test.Util.Application s)
        {
            super(testName, classLoader, s);
            setName("ServerThread");
            _client = c;
        }

        public void run()
        {
            String[] defaultArgs =
            {
                "--Ice.NullHandleAbort=1",
                "--Ice.Warn.Connections=1",
                "--Ice.ThreadPool.Server.Size=1",
                "--Ice.ThreadPool.Server.SizeMax=3",
                "--Ice.ThreadPool.Server.SizeWarn=0"
            };
            List<String> args = new ArrayList<String>(Arrays.asList(defaultArgs));

            setupAddress(args, _ipv6);

            if(_mode.isSSL())
            {
                setupSSL(args, _serverContext);
            }
            _mode.setupProtocol(args);
            _app.setServerReadyListener(new test.Util.Application.ServerReadyListener()
            {
                public void serverReady()
                {
                    if(_client != null)
                    {
                        _clientThread = new ClientThread(_testName, _app.getClassLoader(), _client, _app);
                        _clientThread.start();
                    }
                }
            });

            _status = _app.main("Server", args.toArray(new String[0]));
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
        CollocatedThread(String testName, ClassLoader classLoader, test.Util.Application c)
        {
            super(testName, classLoader, c);
            setName("CollocatedThread");
        }

        public void run()
        {
            String[] defaultArgs =
            {
                "--Ice.NullHandleAbort=1"
            };
            List<String> args = new ArrayList<String>(Arrays.asList(defaultArgs));

            setupAddress(args, _ipv6);

            if(_mode.isSSL())
            {
                setupSSL(args, _clientContext);
            }
            _mode.setupProtocol(args);
            _status = _app.main("Collocated", args.toArray(new String[0]));
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

        if(_sslListener != null)
        {
            final SSLInitializationListener listener = _sslListener;
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
        // By default we don't log to a file.
        //_logOutput = new LogOutput();
        com.zeroc.Ice.Util.setProcessLogger(new AndroidLogger(""));
        _handler = new Handler();
        configureTests();
    }

    @Override
    public void onTerminate()
    {
    }

    public List<String> getTestNames()
    {
        return _tests;
    }

    synchronized public void setTestListener(TestListener listener)
    {
        _listener = listener;
        if(_listener != null && _currentTest != -1)
        {
            _listener.onStartTest(_tests.get(_currentTest));
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

    @SuppressLint("Assert")
    public void startNextTest()
    {
        assert _complete;
        startTest((_currentTest + 1) % _tests.size());
    }

    synchronized public void startTest(int position)
    {
        _currentTest = position;
        _complete = false;
        _strings.clear();

        String testName = _tests.get(position);

        test.Util.Application client;
        test.Util.Application server;
        test.Util.Application collocated;
        ClassLoader classLoader = TestApp.class.getClassLoader();
        if(_listener != null)
        {
            _listener.onStartTest(testName);
        }

        try
        {
            TestSuiteBundle bundle = _bundles.get(testName);
            if(bundle == null)
            {
                bundle = new TestSuiteBundle(testName, classLoader);
                _bundles.put(testName, bundle);
            }
            client = bundle.getClient();
            server = bundle.getServer();
            collocated = bundle.getCollocated();
        }
        catch(IllegalAccessException e)
        {
            Log.w(TAG, "IllegalAccessException: test " + testName, e);
            postOnComplete(-1);
            return;
        }
        catch(InstantiationException e)
        {
            Log.w(TAG, "InstantiationException: test " + testName, e);
            postOnComplete(-1);
            return;
        }

        // Uncomment this if you want to reopen the logfile for each test.
        //if(_logOutput != null)
        //{
        //    _logOutput.reopen();
        //}

        List<TestThread> l = new ArrayList<TestThread>();
        if(server != null)
        {
            server.setLogger(new AndroidLogger(testName + "_server"));
            server.setWriter(new MyWriter());
            // All servers must have a client.
            assert client != null;
            client.setWriter(new MyWriter());
            client.setLogger(new AndroidLogger(testName + "_client"));
            l.add(new ServerThread(testName, classLoader, client, server));
            if(collocated != null)
            {
                collocated.setLogger(new AndroidLogger(testName + "_collocated"));
                collocated.setWriter(new MyWriter());
                l.add(new CollocatedThread(testName, classLoader, collocated));
            }
        }
        else
        {
            client.setWriter(new MyWriter());
            client.setLogger(new AndroidLogger(testName + "_client"));
            l.add(new ClientThread(testName, classLoader, client, null));
        }
        TestRunner r = new TestRunner(l);
        r.setDaemon(true);
        r.start();
    }

    public void setIPv6(boolean ipv6)
    {
        _ipv6 = ipv6;

        configureTests();
        if(_currentTest > _tests.size() - 1)
        {
            _currentTest = _tests.size() - 1;
        }
    }

    public void setMode(int mode)
    {
        switch(mode)
        {
            case 0:
                _mode = Mode.TCP;
                break;
            case 1:
                _mode = Mode.SSL;
                break;
            case 2:
                _mode = Mode.WS;
                break;
            case 3:
                _mode = Mode.WSS;
                break;
        }

        configureTests();
        if(_currentTest > _tests.size() - 1)
        {
            _currentTest = _tests.size() - 1;
        }

        if(_mode.isSSL() && !_sslInitialized)
        {
            if(_sslListener != null)
            {
                _sslListener.onWait();
            }
            Runnable r = new Runnable()
            {
                private SSLContext initializeContext(java.io.InputStream cert)
                    throws NoSuchAlgorithmException, KeyStoreException, IOException, CertificateException,
                    UnrecoverableKeyException, KeyManagementException
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
                        Log.w(TAG, "Exception", ex);
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

    private void configureTests()
    {
        _tests.clear();
        for(String s : _allTests)
        {
            if(_mode.isSSL() && _sslUnsupportedTests.contains(s))
            {
                continue;
            }
            if(!_mode.isSSL() && _tcpUnsupportedTests.contains(s))
            {
                continue;
            }
            if(_ipv6 && _ipv6UnsupportedTests.contains(s))
            {
                continue;
            }
            _tests.add(s);
        }
    }

    synchronized public void setSSLInitializationListener(SSLInitializationListener listener)
    {
        _sslListener = listener;
        if(_mode.isSSL())
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
