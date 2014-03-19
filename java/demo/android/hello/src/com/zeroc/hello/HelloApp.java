// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.hello;

import android.app.Application;
import android.os.Build.VERSION;

public class HelloApp extends Application
{
    interface CommunicatorCallback
    {
        void onWait();
        void onCreate(Ice.Communicator communicator);
        void onError(Ice.LocalException ex);
    }

    /** Called when the application is starting, before any other application objects have been created. */
    @Override
    public void onCreate()
    {
        super.onCreate();

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

        // SSL initialization can take some time. To avoid blocking the
        // calling thread, we perform the initialization in a separate thread.
        new Thread(new Runnable()
        {
            public void run()
            {
                initializeCommunicator();
            }
        }).start();
    }

    /** Called when the application is stopping. */
    @Override
    public void onTerminate()
    {
        super.onTerminate();
        if(_communicator != null)
        {
            try
            {
                _communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
            }
        }
    }

    public void
    setCommunicatorCallback(CommunicatorCallback cb)
    {
        if(_initialized)
        {
            if(_ex != null)
            {
                cb.onError(_ex);
            }
            else
            {
                cb.onCreate(_communicator);
            }
        }
        else
        {
            _cb = cb;
            _cb.onWait();
        }
    }

    private void initializeCommunicator()
    {
        try
        {
            Ice.Communicator communicator;
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Trace.Network", "3");

            //
            // Only configure IceSSL if we are using Froyo or later.
            //
            if(VERSION.SDK_INT >= 8) // android.os.Build.VERSION_CODES.FROYO (8)
            {
                initData.properties.setProperty("IceSSL.Trace.Security", "3");
                initData.properties.setProperty("IceSSL.KeystoreType", "BKS");
                initData.properties.setProperty("IceSSL.TruststoreType", "BKS");
                initData.properties.setProperty("IceSSL.Password", "password");
                initData.properties.setProperty("Ice.InitPlugins", "0");
                initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
            }

            communicator = Ice.Util.initialize(initData);

            if(VERSION.SDK_INT >= 8) // android.os.Build.VERSION_CODES.FROYO (8)
            {
                IceSSL.Plugin plugin = (IceSSL.Plugin)communicator.getPluginManager().getPlugin("IceSSL");
                //
                // Be sure to pass the same input stream to the SSL plug-in for
                // both the keystore and the truststore. This makes startup a
                // little faster since the plugin will not initialize
                // two keystores.
                //
                java.io.InputStream certs = getResources().openRawResource(R.raw.client);
                plugin.setKeystoreStream(certs);
                plugin.setTruststoreStream(certs);
                communicator.getPluginManager().initializePlugins();
            }

            synchronized(this)
            {
                _communicator = communicator;
                if(_cb != null)
                {
                    _cb.onCreate(_communicator);
                }
                _initialized = true;
            }
        }
        catch(Ice.LocalException ex)
        {
            synchronized(this)
            {
                if(_cb != null)
                {
                    _cb.onError(ex);
                }
                _ex = ex;
                _initialized = true;
            }
        }
    }

    private Ice.Communicator _communicator;
    private boolean _initialized;
    private Ice.LocalException _ex;
    private CommunicatorCallback _cb;
}
