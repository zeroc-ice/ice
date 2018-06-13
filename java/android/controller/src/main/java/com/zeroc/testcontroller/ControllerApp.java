// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.testcontroller;

import java.io.*;
import java.util.*;

import com.zeroc.Ice.Logger;
import com.zeroc.Ice.Communicator;
import com.zeroc.IceInternal.Time;

import android.os.Build;
import android.util.Log;
import android.app.Application;

import Test.Common.ProcessControllerRegistryPrx;
import Test.Common.ProcessControllerPrx;

public class ControllerApp extends Application
{
    private final String TAG = "ControllerApp";
    private ControllerI _controllerI;
    private ControllerActivity _activity;
    private String _ipv4Address;
    private String _ipv6Address;

    static private class TestSuiteBundle
    {
        @SuppressWarnings("unchecked")
        TestSuiteBundle(String name, ClassLoader loader) throws ClassNotFoundException
        {
            _loader = loader;
            _class = (Class<? extends test.TestHelper>)_loader.loadClass(name);
        }

        test.TestHelper newInstance()
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
        private Class<? extends test.TestHelper> _class;
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

    synchronized public void setIpv4Address(String address)
    {
        _ipv4Address = address;
    }

    synchronized public void setIpv6Address(String address)
    {
        int i = address.indexOf("%");
        _ipv6Address = i == -1 ? address : address.substring(i);
    }

    public List<String> getAddresses(boolean ipv6)
    {
        List<String> addresses = new java.util.ArrayList<String>();
        try
        {
            java.util.Enumeration<java.net.NetworkInterface> ifaces = java.net.NetworkInterface.getNetworkInterfaces();
            while(ifaces.hasMoreElements())
            {
                java.net.NetworkInterface iface = ifaces.nextElement();
                java.util.Enumeration<java.net.InetAddress> addrs = iface.getInetAddresses();
                while(addrs.hasMoreElements())
                {
                    java.net.InetAddress addr = addrs.nextElement();
                    if((ipv6 && addr instanceof java.net.Inet6Address) ||
                       (!ipv6 && !(addr instanceof java.net.Inet6Address)))
                    {
                        addresses.add(addr.getHostAddress());
                    }
                }
            }
        }
        catch(java.net.SocketException ex)
        {
        }
        return addresses;
    }

    public synchronized void startController(ControllerActivity activity, boolean bluetooth)
    {
        _activity = activity;
        if(_controllerI == null)
        {
            _controllerI = new ControllerI(bluetooth);
        }
    }

    public synchronized void println(final String data)
    {
        _activity.runOnUiThread(new Runnable()
                                {
                                    @Override
                                    public void run()
                                    {
                                        synchronized(ControllerApp.this)
                                        {
                                            _activity.println(data);
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

    class ControllerI
    {
        public ControllerI(boolean bluetooth)
        {
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = com.zeroc.Ice.Util.createProperties();
            initData.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "10");
            initData.properties.setProperty("ControllerAdapter.Endpoints", "tcp");
            initData.properties.setProperty("Ice.Trace.Network", "3");
            initData.properties.setProperty("Ice.Trace.Protocol", "1");
            initData.properties.setProperty("ControllerAdapter.AdapterId", java.util.UUID.randomUUID().toString());
            initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");
            if(!isEmulator())
            {
                if(bluetooth)
                {
                    initData.properties.setProperty("Ice.Plugin.IceBT", "com.zeroc.IceBT.PluginFactory");
                }
                initData.properties.setProperty("Ice.Plugin.IceDiscovery", "com.zeroc.IceDiscovery.PluginFactory");
                initData.properties.setProperty("IceDiscovery.DomainId", "TestController");
            }
            _communicator = com.zeroc.Ice.Util.initialize(initData);
            com.zeroc.Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("ControllerAdapter");
            ProcessControllerPrx processController = ProcessControllerPrx.uncheckedCast(
                    adapter.add(new ProcessControllerI(),
                                com.zeroc.Ice.Util.stringToIdentity("Android/ProcessController")));
            adapter.activate();
            if(isEmulator())
            {
                ProcessControllerRegistryPrx registry = ProcessControllerRegistryPrx.uncheckedCast(
                        _communicator.stringToProxy("Util/ProcessControllerRegistry:tcp -h 10.0.2.2 -p 15001"));
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

    class ControllerHelperI extends Thread implements test.TestHelper.ControllerHelper
    {
        public ControllerHelperI(TestSuiteBundle bundle, String[] args, String exe)
        {
            _bundle = bundle;
            _args = args;
        }

        public void communicatorInitialized(Communicator communicator)
        {
            com.zeroc.Ice.Properties properties = communicator.getProperties();
            if(properties.getProperty("Ice.Plugin.IceSSL").equals("com.zeroc.IceSSL.PluginFactory"))
            {
                com.zeroc.IceSSL.Plugin plugin =
                        (com.zeroc.IceSSL.Plugin)communicator.getPluginManager().getPlugin("IceSSL");
                String keystore = communicator.getProperties().getProperty("IceSSL.Keystore");
                properties.setProperty("IceSSL.Keystore", "");
                int resource = keystore.equals("client.bks") ? R.raw.client : R.raw.server;
                java.io.InputStream certs = getResources().openRawResource(resource);
                plugin.setKeystoreStream(certs);
                plugin.setTruststoreStream(certs);
                communicator.getPluginManager().initializePlugins();
            }
        }

        public void run()
        {
            try
            {
                _helper = _bundle.newInstance();
                _helper.setClassLoader(_bundle.getClassLoader());
                _helper.setControllerHelper(this);

                _helper.setWriter(new Writer()
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

                _helper.run(_args);
                synchronized(this)
                {
                    _status = 0;
                    _completed = true;
                    notifyAll();
                }
            }
            catch(Exception ex)
            {
                ex.printStackTrace(_helper.getWriter());
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
            if(_helper != null)
            {
                _helper.shutdown();
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
        private test.TestHelper _helper;
        private boolean _ready = false;
        private boolean _completed = false;
        private int _status = 0;
        private final StringBuffer _out = new StringBuffer();
    }

    class ProcessControllerI implements Test.Common.ProcessController
    {
        public Test.Common.ProcessPrx start(final String testsuite, final String exe, String[] args,
                                            com.zeroc.Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            println("starting " + testsuite + " " + exe + "... ");
            String className = "test." + testsuite.replace("/", ".") + "." + exe.substring(0, 1).toUpperCase() + exe.substring(1);
            try
            {
                TestSuiteBundle bundle = new TestSuiteBundle(className, getClassLoader());
                ControllerHelperI mainHelper = new ControllerHelperI(bundle, args, exe);
                mainHelper.start();
                return Test.Common.ProcessPrx.uncheckedCast(current.adapter.addWithUUID(new ProcessI(mainHelper)));
            }
            catch(ClassNotFoundException ex)
            {
                throw new Test.Common.ProcessFailedException(
                    "testsuite `" + testsuite + "' exe ` " + exe + "' start failed:\n" + ex.toString());
            }
        }

        public String getHost(String protocol, boolean ipv6, com.zeroc.Ice.Current current)
        {
            if(isEmulator())
            {
                return  "127.0.0.1";
            }
            else
            {
                synchronized(ControllerApp.this)
                {
                    return ipv6 ? _ipv6Address : _ipv4Address;
                }
            }
        }
    }

    class ProcessI implements Test.Common.Process
    {
        public ProcessI(ControllerHelperI controllerHelper)
        {
            _controllerHelper = controllerHelper;
        }

        public void waitReady(int timeout, com.zeroc.Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            _controllerHelper.waitReady(timeout);
        }

        public int waitSuccess(int timeout, com.zeroc.Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            return _controllerHelper.waitSuccess(timeout);
        }

        public String terminate(com.zeroc.Ice.Current current)
        {
            _controllerHelper.shutdown();
            current.adapter.remove(current.id);
            while(true)
            {
                try
                {
                    _controllerHelper.join();
                    break;
                }
                catch(InterruptedException ex)
                {
                }
            }
            return _controllerHelper.getOutput();
        }

        private ControllerHelperI _controllerHelper;
    }
}
