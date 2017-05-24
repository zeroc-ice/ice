// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.testcontroller;

import java.io.*;
import java.util.Optional;
import java.util.OptionalInt;

import com.zeroc.Ice.Logger;
import com.zeroc.Ice.Communicator;
import com.zeroc.IceInternal.Time;

import dalvik.system.DexClassLoader;

import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.app.Application;

import Test.Common.ProcessControllerRegistryPrx;
import Test.Common.ProcessControllerPrx;
import test.Util.Application.CommunicatorListener;

public class ControllerApp extends Application
{
    private final String TAG = "ControllerApp";
    private ControllerHelper _helper;
    private ControllerActivity _controller;
    private java.util.Map<String, ClassLoader> _classLoaders = new java.util.HashMap<String, ClassLoader>();

    private ClassLoader getDEXClassLoader(String classDir, ClassLoader parent) throws IOException
    {
        ClassLoader classLoader = _classLoaders.get(classDir);
        if(classLoader == null)
        {
            if(parent == null)
            {
                parent = getClassLoader();
            }

            File dexInternalStoragePath = new java.io.File(getDir("dex", Context.MODE_PRIVATE), classDir);
            BufferedInputStream bis = new BufferedInputStream(getAssets().open(classDir));
            OutputStream dexWriter = new BufferedOutputStream(new FileOutputStream(dexInternalStoragePath));
            final int sz = 8 * 1024;
            byte[] buf = new byte[sz];
            int len;
            while((len = bis.read(buf, 0, sz)) > 0)
            {
                dexWriter.write(buf, 0, len);
            }
            dexWriter.close();
            bis.close();

            // Internal storage where the DexClassLoader writes the optimized dex file to
            final File optimizedDexOutputPath = getDir("outdex", Context.MODE_PRIVATE);

            classLoader = new DexClassLoader(
                    dexInternalStoragePath.getAbsolutePath(),
                    optimizedDexOutputPath.getAbsolutePath(),
                    null,
                    parent);
            _classLoaders.put(classDir, classLoader);
        }
        return classLoader;
    }

    static private class TestSuiteBundle
    {
        @SuppressWarnings("unchecked")
        TestSuiteBundle(String name, ClassLoader loader)
        {
            _loader = loader;
            try
            {
                _class = (Class<? extends test.Util.Application>)_loader.loadClass(name);
            }
            catch(ClassNotFoundException e)
            {
            }
        }

        test.Util.Application newInstance()
                throws IllegalAccessException, InstantiationException
        {
            if(_class == null)
            {
                return null;
            }
            return _class.newInstance();
        }

        ClassLoader getClassLoader()
        {
            return  _loader;
        }

        private String _name;
        private ClassLoader _loader;
        private Class<? extends test.Util.Application> _class;
    }

    class AndroidLogger implements Logger
    {
        private final String _prefix;

        AndroidLogger(String prefix)
        {
            _prefix = prefix;
        }

        @Override
        public void print(String message)
        {
            Log.d(TAG, message);
        }

        @Override
        public void trace(String category, String message)
        {
            Log.v(category, message);
        }

        @Override
        public void warning(String message)
        {
            Log.w(TAG, message);
        }

        @Override
        public void error(String message)
        {
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

    @Override
    public void onCreate()
    {
        super.onCreate();
        com.zeroc.Ice.Util.setProcessLogger(new AndroidLogger(""));
    }

    public synchronized void startController(ControllerActivity controller)
    {
        if(_helper == null)
        {
            _controller = controller;
            _helper = new ControllerHelper();
        }
        else
        {
            _controller = controller;
        }
    }

    public synchronized void println(final String data)
    {
        _controller.runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                synchronized(ControllerApp.this)
                {
                    _controller.println(data);
                }
            }
        });
    }

    public static boolean isEmulator()
    {
        return Build.FINGERPRINT.startsWith("generic") ||
               Build.FINGERPRINT.startsWith("unknown") ||
               Build.MODEL.contains("google_sdk") ||
               Build.MODEL.contains("Emulator") ||
               Build.MODEL.contains("Android SDK built for x86") ||
               Build.MANUFACTURER.contains("Genymotion") ||
               (Build.BRAND.startsWith("generic") && Build.DEVICE.startsWith("generic")) ||
               Build.PRODUCT.equals("google_sdk");
    }

    class ControllerHelper
    {
        public ControllerHelper()
        {
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = com.zeroc.Ice.Util.createProperties();
            initData.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "10");
            initData.properties.setProperty("ControllerAdapter.Endpoints", "tcp");
            initData.properties.setProperty("Ice.Trace.Network", "3");
            initData.properties.setProperty("Ice.Trace.Protocol", "1");

            initData.properties.setProperty("ControllerAdapter.AdapterId", java.util.UUID.randomUUID().toString());
            if(!isEmulator())
            {
                initData.properties.setProperty("Ice.Plugin.IceDiscovery", "IceDiscovery.PluginFactory");
                initData.properties.setProperty("IceDiscovery.DomainId", "TestController");
            }
            _communicator = com.zeroc.Ice.Util.initialize(initData);
            com.zeroc.Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("ControllerAdapter");
            ProcessControllerPrx processController = ProcessControllerPrx.uncheckedCast(
                    adapter.add(new ProcessControllerI("localhost"),
                                com.zeroc.Ice.Util.stringToIdentity("Android/ProcessController")));
            adapter.activate();
            if(isEmulator())
            {
                ProcessControllerRegistryPrx registry = ProcessControllerRegistryPrx.uncheckedCast(
                        _communicator.stringToProxy("Util/ProcessControllerRegistry:tcp -h 10.0.2.2 -p 15001").ice_invocationTimeout(300));
                registerProcessController(adapter, registry, processController);
            }
            println("Android/ProcessController");
        }

        public void
        registerProcessController(final com.zeroc.Ice.ObjectAdapter adapter,
                                  final ProcessControllerRegistryPrx registry,
                                  final ProcessControllerPrx processController)
        {
            registry.ice_pingAsync().whenCompleteAsync(
                (r1, e1) ->
                {
                    if(e1 != null)
                    {
                        handleException(e1, adapter, registry, processController);
                    }
                    else
                    {
                        com.zeroc.Ice.Connection connection = registry.ice_getConnection();
                        connection.setAdapter(adapter);
                        connection.setACM(OptionalInt.of(5),
                                Optional.of(com.zeroc.Ice.ACMClose.CloseOff),
                                Optional.of(com.zeroc.Ice.ACMHeartbeat.HeartbeatAlways));
                        connection.setCloseCallback(
                                con ->
                                {
                                    println("connection with process controller registry closed");
                                    while (true) {
                                        try
                                        {
                                            Thread.sleep(500);
                                            break;
                                        }
                                        catch(InterruptedException e)
                                        {
                                        }
                                    }
                                    registerProcessController(adapter, registry, processController);
                                });

                        registry.setProcessControllerAsync(processController).whenCompleteAsync(
                                (r2, e2) ->
                                {
                                    if(e2 != null)
                                    {
                                        handleException(e2, adapter, registry, processController);
                                    }
                                });
                    }
                });
        }

        public void handleException(Throwable ex,
                                    final com.zeroc.Ice.ObjectAdapter adapter,
                                    final ProcessControllerRegistryPrx registry,
                                    final ProcessControllerPrx processController)
        {
            if(ex instanceof com.zeroc.Ice.ConnectFailedException || ex instanceof com.zeroc.Ice.TimeoutException)
            {
                while(true)
                {
                    try
                    {
                        Thread.sleep(500);
                        break;
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
                registerProcessController(adapter, registry, processController);
            }
            else
            {
                println(ex.toString());
            }
        }

        public void destroy()
        {
            _communicator.destroy();
        }

        private ProcessControllerRegistryPrx _registry;
        private com.zeroc.Ice.Communicator _communicator;
    }

    class MainHelperI extends Thread implements test.Util.Application.ServerReadyListener
    {
        public MainHelperI(TestSuiteBundle bundle, String[] args, String exe)
        {
            _bundle = bundle;
            _args = args;
        }

        public void run()
        {
            try
            {
                _app = _bundle.newInstance();
                _app.setClassLoader(_bundle.getClassLoader());
                _app.setCommunicatorListener(new CommunicatorListener()
                {
                    public void communicatorInitialized(Communicator communicator)
                    {
                        if(communicator.getProperties().getProperty("Ice.Plugin.IceSSL").equals("com.zeroc.IceSSL.PluginFactory"))
                        {
                            com.zeroc.IceSSL.Plugin plugin = (com.zeroc.IceSSL.Plugin)communicator.getPluginManager().getPlugin("IceSSL");
                            String keystore = communicator.getProperties().getProperty("IceSSL.Keystore");
                            communicator.getProperties().setProperty("IceSSL.Keystore", "");
                            java.io.InputStream certs = getResources().openRawResource(
                                keystore.equals("client.bks") ? R.raw.client : R.raw.server);
                            plugin.setKeystoreStream(certs);
                            plugin.setTruststoreStream(certs);
                            communicator.getPluginManager().initializePlugins();
                        }
                    }
                });
                _app.setWriter(new Writer()
                    {
                        @Override
                        public void close() throws IOException
                        {
                        }

                        @Override
                        public void flush() throws IOException
                        {
                        }

                        @Override
                        public void write(char[] buf, int offset, int count)
                            throws IOException
                        {
                            _out.append(buf, offset, count);
                        }
                    });
                _app.setServerReadyListener(this);

                int status = _app.main(_exe, _args);
                synchronized(this)
                {
                    _status = status;
                    _completed = true;
                    notifyAll();
                }
            }
            catch(Exception ex)
            {
                _out.append(ex.toString());
                synchronized(this)
                {
                    _status = -1;
                    _completed = true;
                    notifyAll();
                }
            }
        }

        public void shutdown()
        {
            if(_app != null)
            {
                _app.stop();
            }
        }

        public String getOutput()
        {
            return _out.toString();
        }

        synchronized private void completed(int status)
        {
            _completed = true;
            _status = status;
            notifyAll();
        }

        synchronized public void serverReady()
        {
            _ready = true;
            notifyAll();
        }

        synchronized private void waitReady(int timeout)
            throws Test.Common.ProcessFailedException
        {
            long now = Time.currentMonotonicTimeMillis();
            while(!_ready && !_completed)
            {
                try
                {
                    wait(timeout * 1000);
                    if(Time.currentMonotonicTimeMillis() - now > timeout * 1000)
                    {
                        throw new Test.Common.ProcessFailedException("timed out waiting for the process to be ready");
                    }
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

            if(_completed && _status != 0)
            {
                throw new Test.Common.ProcessFailedException(_out.toString());
            }
        }

        synchronized private int waitSuccess(int timeout)
            throws Test.Common.ProcessFailedException
        {
            long now = Time.currentMonotonicTimeMillis();
            while(!_completed)
            {
                try
                {
                    wait(timeout * 1000);
                    if(Time.currentMonotonicTimeMillis() - now > timeout * 1000)
                    {
                        throw new Test.Common.ProcessFailedException("timed out waiting for the process to be ready");
                    }
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            return _status;
        }

        private TestSuiteBundle _bundle;
        private String[] _args;
        private String _exe;
        private test.Util.Application _app;
        private boolean _ready = false;
        private boolean _completed = false;
        private int _status = 0;
        private final StringBuffer _out = new StringBuffer();
    }

    class ProcessControllerI implements Test.Common.ProcessController
    {
        public ProcessControllerI(String hostname)
        {
            _hostname = hostname;
        }

        public Test.Common.ProcessPrx start(final String testsuite, final String exe, String[] args,
                                            com.zeroc.Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            println("starting " + testsuite + " " + exe + "... ");
            String className = "test." + testsuite.replace("/", ".") + "." + exe.substring(0, 1).toUpperCase() + exe.substring(1);
            String dexFile = "test_" + testsuite.replace("/", "_") + ".dex";
            try
            {
                TestSuiteBundle bundle = new TestSuiteBundle(className, getDEXClassLoader(dexFile, null));
                MainHelperI mainHelper = new MainHelperI(bundle, args, exe);
                mainHelper.start();
                return Test.Common.ProcessPrx.uncheckedCast(current.adapter.addWithUUID(new ProcessI(mainHelper)));
            }
            catch(IOException ex)
            {
                throw new Test.Common.ProcessFailedException(
                    "testsuite `" + testsuite + "' exe ` " + exe + "' start failed:\n" + ex.toString());
            }
        }

        public String getHost(String protocol, boolean ipv6,
                              com.zeroc.Ice.Current current)
        {
            return _hostname;
        }

        private String _hostname;
    }

    class ProcessI implements Test.Common.Process
    {
        public ProcessI(MainHelperI helper)
        {
            _helper = helper;
        }

        public void waitReady(int timeout, com.zeroc.Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            _helper.waitReady(timeout);
        }

        public int waitSuccess(int timeout, com.zeroc.Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            return _helper.waitSuccess(timeout);
        }

        public String terminate(com.zeroc.Ice.Current current)
        {
            _helper.shutdown();
            current.adapter.remove(current.id);
            while(true)
            {
                try
                {
                    _helper.join();
                    break;
                }
                catch(InterruptedException ex)
                {
                }
            }
            return _helper.getOutput();
        }

        private MainHelperI _helper;
    }
}
