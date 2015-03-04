// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library.controller;

import com.zeroc.library.R;

import android.content.res.Resources;
import android.os.Handler;

public class LoginController
{
    public interface Listener
    {
        void onLoginInProgress();
        void onLogin(SessionController controller);
        void onLoginError();
    }

    private boolean _destroyed = false;
    private Handler _handler;
    private Ice.Communicator _communicator;
    private String _loginError;
    private Listener _loginListener;
    private SessionController _sessionController;

    synchronized private void postLoginFailure(final String loginError)
    {
        _loginError = loginError;
        if(_loginListener != null)
        {
            final Listener listener = _loginListener;
            _handler.post(new Runnable()
            {
                public void run()
                {
                    listener.onLoginError();
                }
            });
        }
    }

    public LoginController(Resources resources, final String hostname, final boolean secure, final boolean glacier2,
                           Listener listener)
    {
        _handler = new Handler();
        _loginListener = listener;
        _loginListener.onLoginInProgress();

        Ice.InitializationData initData = new Ice.InitializationData();

        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.ACM.Client", "0");
        initData.properties.setProperty("Ice.RetryIntervals", "-1");
        initData.properties.setProperty("Ice.Trace.Network", "0");

        if(secure)
        {
            initData.properties.setProperty("IceSSL.Trace.Security", "0");
            initData.properties.setProperty("IceSSL.TruststoreType", "BKS");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("Ice.InitPlugins", "0");
            initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
            java.io.InputStream certStream;
            certStream = resources.openRawResource(R.raw.client);
            _communicator = Ice.Util.initialize(initData);

            IceSSL.Plugin plugin = (IceSSL.Plugin)_communicator.getPluginManager().getPlugin("IceSSL");
            plugin.setTruststoreStream(certStream);
            _communicator.getPluginManager().initializePlugins();
        }
        else
        {
            _communicator = Ice.Util.initialize(initData);
        }

        new Thread(new Runnable()
        {
            public void run()
            {
                try
                {
                    long refreshTimeout;
                    SessionAdapter session = null;

                    if(glacier2)
                    {
                        StringBuilder s = new StringBuilder();
                        s.append("DemoGlacier2/router:");
                        s.append(secure ? "ssl " : "tcp");
                        s.append(" -p ");
                        //
                        // BUGFIX: When connecting to demo2.zeroc.com in secure mode, we should connect to the
                        // glacier2 listening in port 5064 that use the new SSL certificates included in this
                        // distribution.
                        //
                        if(secure && hostname.equals("demo2.zeroc.com"))
                        {
                            s.append("5064");
                        }
                        else
                        {
                            s.append(secure ? "4064" : "4063");
                        }
                        s.append(" -h ");
                        s.append(hostname);
                        s.append(" -t 10000");

                        Ice.ObjectPrx proxy = _communicator.stringToProxy(s.toString());
                        Ice.RouterPrx r = Ice.RouterPrxHelper.uncheckedCast(proxy);

                        _communicator.setDefaultRouter(r);

                        final Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.checkedCast(r);
                        if(router == null)
                        {
                            postLoginFailure("Glacier2 proxy is invalid.");
                            return;
                        }
                        Glacier2.SessionPrx glacier2session = router.createSession("dummy", "none");

                        final Demo.Glacier2SessionPrx sess = Demo.Glacier2SessionPrxHelper
                                .uncheckedCast(glacier2session);
                        final Demo.LibraryPrx library = sess.getLibrary();
                        refreshTimeout = (router.getSessionTimeout() * 1000) / 2;
                        session = new SessionAdapter()
                        {
                            public void destroy()
                            {
                                try
                                {
                                    router.destroySession();
                                }
                                catch(Glacier2.SessionNotExistException e)
                                {
                                }
                            }

                            public void refresh()
                            {
                                sess.refresh();
                            }

                            public Demo.LibraryPrx getLibrary()
                            {
                                return library;
                            }
                        };
                    }
                    else
                    {
                        StringBuilder s = new StringBuilder();
                        s.append("SessionFactory:");
                        s.append(secure ? "ssl " : "tcp");
                        s.append(" -p ");
                        //
                        // BUGFIX: When connecting to demo2.zeroc.com in secure mode, we should connect to the
                        // server listening in port 20001 that use the new SSL certificates included in 3.5.1
                        //
                        if(secure && hostname.equals("demo2.zeroc.com"))
                        {
                            s.append("20001");
                        }
                        else
                        {
                            s.append(secure ? "10001" : "10000");
                        }
                        s.append(" -h ");
                        s.append(hostname);
                        s.append(" -t 10000");
                        Ice.ObjectPrx proxy = _communicator.stringToProxy(s.toString());
                        Demo.SessionFactoryPrx factory = Demo.SessionFactoryPrxHelper.checkedCast(proxy);
                        if(factory == null)
                        {
                            postLoginFailure("SessionFactory proxy is invalid.");
                            return;
                        }

                        final Demo.SessionPrx sess = factory.create();
                        final Demo.LibraryPrx library = sess.getLibrary();
                        refreshTimeout = (factory.getSessionTimeout() * 1000) / 2;
                        session = new SessionAdapter()
                        {
                            public void destroy()
                            {
                                sess.destroy();
                            }

                            public void refresh()
                            {
                                sess.refresh();
                            }

                            public Demo.LibraryPrx getLibrary()
                            {
                                return library;
                            }
                        };
                    }

                    synchronized(LoginController.this)
                    {
                        if(_destroyed)
                        {
                            // Here the app was terminated while session
                            // establishment was in progress.
                            try
                            {
                                session.destroy();
                            }
                            catch(Exception e)
                            {
                            }

                            try
                            {
                                _communicator.destroy();
                            }
                            catch(Exception e)
                            {
                            }
                            return;
                        }

                        _sessionController = new SessionController(_handler, _communicator, session, refreshTimeout);
                        if(_loginListener != null)
                        {
                            final Listener listener = _loginListener;
                            _handler.post(new Runnable()
                            {
                                public void run()
                                {
                                    listener.onLogin(_sessionController);
                                }
                            });
                        }
                    }
                }
                catch(final Glacier2.CannotCreateSessionException ex)
                {
                    ex.printStackTrace();
                    postLoginFailure(String.format("Session creation failed: %s", ex.reason));
                }
                catch(Glacier2.PermissionDeniedException ex)
                {
                    ex.printStackTrace();
                    postLoginFailure(String.format("Login failed: %s", ex.reason));
                }
                catch(Ice.LocalException ex)
                {
                    ex.printStackTrace();
                    postLoginFailure(String.format("Login failed: %s", ex.toString()));
                }
            }
        }).start();
    }

    synchronized public void destroy()
    {
        if(_destroyed)
        {
            return;
        }
        _destroyed = true;
        //
        // There are three cases:
        //
        // 1. A session has been created. In this case the communicator is owned
        // by the session controller.
        //
        // 2. The session creation failed. In this case _loginError is non-null.
        // Destroy the communicator.
        //
        // 3. The session creation is in progress. In which case, things will be
        // cleaned up once the session establishment is complete.
        //
        if(_sessionController == null && _loginError != null)
        {
            try
            {
                _communicator.destroy();
            }
            catch(Exception e)
            {
            }
        }
    }

    synchronized public String getLoginError()
    {
        return _loginError;
    }

    synchronized public void setLoginListener(Listener listener)
    {
        _loginListener = listener;
        if(listener != null)
        {
            if(_loginError != null)
            {
                listener.onLoginError();
            }
            else if(_sessionController == null)
            {
                listener.onLoginInProgress();
            }
            else
            {
                listener.onLogin(_sessionController);
            }
        }
    }
}
