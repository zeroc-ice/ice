// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.hello;

import Ice.Communicator;
import android.app.Application;
import android.os.Build.VERSION;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import java.util.LinkedList;
import java.util.List;

public class HelloApp extends Application
{
    static class MessageReady
    {
        MessageReady(Communicator c, Ice.LocalException e)
        {
            communicator = c;
            ex = e;
        }

        Communicator communicator;
        Ice.LocalException ex;
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        _uiHandler = new Handler(Looper.getMainLooper())
        {
            @Override
            public void handleMessage(Message m)
            {
                if(m.what == MSG_READY)
                {
                    MessageReady ready = (MessageReady)m.obj;
                    _initialized = true;
                    _communicator = ready.communicator;
                    _ex = ready.ex;
                }
                else if(m.what == MSG_EXCEPTION || m.what == MSG_RESPONSE)
                {
                    _result = null;
                }

                Message copy = new Message();
                copy.copyFrom(m);

                if(_handler != null)
                {
                    _handler.sendMessage(copy);
                }
                else
                {
                    _queue.add(copy);
                }
            }
        };

        // SSL initialization can take some time. To avoid blocking the
        // calling thread, we perform the initialization in a separate thread.
        new Thread(new Runnable()
        {
            public void run()
            {
                try
                {
                    Ice.InitializationData initData = new Ice.InitializationData();
                    initData.properties = Ice.Util.createProperties();
                    initData.properties.setProperty("Ice.Trace.Network", "3");

                    initData.properties.setProperty("IceSSL.Trace.Security", "3");
                    initData.properties.setProperty("IceSSL.KeystoreType", "BKS");
                    initData.properties.setProperty("IceSSL.TruststoreType", "BKS");
                    initData.properties.setProperty("IceSSL.Password", "password");
                    initData.properties.setProperty("Ice.InitPlugins", "0");
                    initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");

                    // SDK versions < 21 only support TLSv1 with SSLEngine.
                    if(VERSION.SDK_INT < 21)
                    {
                        initData.properties.setProperty("IceSSL.Protocols", "tls1_0");
                    }

                    Ice.Communicator c = Ice.Util.initialize(initData);
                    IceSSL.Plugin plugin = (IceSSL.Plugin)c.getPluginManager().getPlugin("IceSSL");
                    //
                    // Be sure to pass the same input stream to the SSL plug-in for
                    // both the keystore and the truststore. This makes startup a
                    // little faster since the plugin will not initialize
                    // two keystores.
                    //
                    java.io.InputStream certs = getResources().openRawResource(R.raw.client);
                    plugin.setKeystoreStream(certs);
                    plugin.setTruststoreStream(certs);
                    c.getPluginManager().initializePlugins();

                    _uiHandler.sendMessage(Message.obtain(_uiHandler, MSG_READY, new MessageReady(c, null)));
                }
                catch(Ice.LocalException e)
                {
                    _uiHandler.sendMessage(Message.obtain(_uiHandler, MSG_READY, new MessageReady(null, e)));
                }
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

    void setHandler(Handler handler)
    {
        // Nothing to do in this case.
        if(_handler != handler)
        {
            _handler = handler;

            if(_handler != null)
            {
                if(!_initialized)
                {
                    _handler.sendMessage(_handler.obtainMessage(MSG_WAIT));
                }
                else
                {
                    // Send all the queued messages.
                    while(!_queue.isEmpty())
                    {
                        _handler.sendMessage(_queue.remove(0));
                    }
                }
            }
        }
    }

    void setHost(String host)
    {
        _host = host;
        _proxy = null;
    }

    void setTimeout(int timeout)
    {
        _timeout = timeout;
        _proxy = null;
    }

    void setDeliveryMode(DeliveryMode mode)
    {
        _mode = mode;
        _proxy = null;
    }

    void flush()
    {
        try
        {
            _communicator.begin_flushBatchRequests();
        }
        catch(final Ice.LocalException ex)
        {
            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_EXCEPTION, ex));
        }
    }

    void shutdown()
    {
        try
        {
            updateProxy();
            if(_proxy == null)
            {
                return;
            }
            _proxy.shutdown();
        }
        catch(Ice.LocalException ex)
        {
            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_EXCEPTION, ex));
        }

    }

    void shutdownAsync()
    {
        try
        {
            updateProxy();
            if(_proxy == null || _result != null)
            {
                return;
            }

            _resultMode = _mode;
            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_SENDING));
            _result = _proxy.begin_shutdown(new Demo.Callback_Hello_shutdown()
            {
                @Override
                synchronized public void exception(final Ice.LocalException ex)
                {
                    _response = true;
                    _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_EXCEPTION, ex));
                }

                @Override
                synchronized public void response()
                {
                    _response = true;
                    _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_RESPONSE));
                }

                @Override
                synchronized public void sent(boolean sentSynchronously)
                {
                    if(!_response)
                    {
                        _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_SENT, _resultMode));
                    }
                }
                // There is no ordering guarantee between sent, response/exception.
                private boolean _response = false;
            });
        }
        catch(Ice.LocalException ex)
        {
            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_EXCEPTION, ex));
        }
    }

    void sayHello(int delay)
    {
        try
        {
            updateProxy();
            if(_proxy == null || _result != null)
            {
                return;
            }

            _proxy.begin_sayHello(delay);
        }
        catch(Ice.LocalException ex)
        {
            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_EXCEPTION, ex));
        }
    }

    void sayHelloAsync(int delay)
    {
        try
        {
            updateProxy();
            if(_proxy == null || _result != null)
            {
                return;
            }

            _resultMode = _mode;
            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_SENDING));
            _result = _proxy.begin_sayHello(delay,
                    new Demo.Callback_Hello_sayHello()
                    {
                        @Override
                        synchronized public void exception(final Ice.LocalException ex)
                        {
                            _response = true;
                            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_EXCEPTION, ex));
                        }

                        @Override
                        synchronized public void response()
                        {
                            _response = true;
                            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_RESPONSE));
                        }

                        @Override
                        synchronized public void sent(boolean sentSynchronously)
                        {
                            if(!_response)
                            {
                                _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_SENT, _resultMode));
                            }
                        }
                        // There is no ordering guarantee between sent, response/exception.
                        private boolean _response = false;
                    });
        }
        catch(Ice.LocalException ex)
        {
            _uiHandler.sendMessage(_uiHandler.obtainMessage(MSG_EXCEPTION, ex));
        }
    }

    private void updateProxy()
    {
        if(_proxy != null)
        {
            return;
        }

        String s = "hello:tcp -h " + _host + " -p 10000:ssl -h " + _host + " -p 10001:udp -h " + _host  + " -p 10000";
        Ice.ObjectPrx prx = _communicator.stringToProxy(s);
        prx = _mode.apply(prx);
        if(_timeout != 0)
        {
            prx = prx.ice_invocationTimeout(_timeout);
        }

        _proxy = Demo.HelloPrxHelper.uncheckedCast(prx);
    }

    DeliveryMode getDeliveryMode()
    {
        return _mode;
    }

    public static final int MSG_WAIT = 0;
    public static final int MSG_READY = 1;
    public static final int MSG_EXCEPTION = 2;
    public static final int MSG_RESPONSE = 3;
    public static final int MSG_SENDING = 4;
    public static final int MSG_SENT = 5;

    private List<Message> _queue = new LinkedList<Message>();
    private Handler _uiHandler;

    private boolean _initialized;
    private Ice.Communicator _communicator;
    private Demo.HelloPrx _proxy = null;

    // The current request if any.
    private Ice.AsyncResult _result;
    // The mode of the current request.
    private DeliveryMode _resultMode;

    private Ice.LocalException _ex;
    private Handler _handler;

    // Proxy settings.
    private String _host;
    private int _timeout;
    private DeliveryMode _mode;
}
