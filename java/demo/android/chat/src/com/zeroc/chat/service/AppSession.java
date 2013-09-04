// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.chat.service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.UUID;

import com.zeroc.chat.R;

import android.content.res.Resources;
import android.os.Handler;

public class AppSession
{
    private Ice.Communicator _communicator;
    private Chat.ChatSessionPrx _session;
    private List<ChatRoomListener> _listeners = new LinkedList<ChatRoomListener>();
    private LinkedList<ChatEventReplay> _replay = new LinkedList<ChatEventReplay>();
    private List<String> _users = new LinkedList<String>();

    static final int MAX_MESSAGES = 200;
    private static final int INACTIVITY_TIMEOUT = 5 * 60 * 1000; // 5 Minutes

    private boolean _destroyed = false;
    private long _lastSend;
    private long _refreshTimeout;
    private Glacier2.RouterPrx _router;
    private Handler _handler;
    private String _hostname;
    private String _error;

    public AppSession(Resources resources, Handler handler, String hostname, String username,
                      String password, boolean secure)
        throws Glacier2.CannotCreateSessionException, Glacier2.PermissionDeniedException
    {
        _handler = handler;

        Ice.InitializationData initData = new Ice.InitializationData();

        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.ACM.Client", "0");
        initData.properties.setProperty("Ice.RetryIntervals", "-1");
        initData.properties.setProperty("Ice.Trace.Network", "0");
        initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
        initData.properties.setProperty("Ice.InitPlugins", "0");
        initData.properties.setProperty("IceSSL.TruststoreType", "BKS");
        initData.properties.setProperty("IceSSL.Password", "password");

        _communicator = Ice.Util.initialize(initData);
        _hostname = hostname;

        String s;
        if(secure)
        {
            java.io.InputStream certStream;
            certStream = resources.openRawResource(R.raw.client);

            IceSSL.Plugin plugin = (IceSSL.Plugin)_communicator.getPluginManager().getPlugin("IceSSL");
            plugin.setTruststoreStream(certStream);
            _communicator.getPluginManager().initializePlugins();
            
            //
            // BUGFIX: When connecting to demo.zeroc.com in secure mode, we should connect to the
            // glacier2 listening in port 5064 that use the new SSL certificates included in this
            // distribution.
            //
            if(hostname.equals("demo.zeroc.com"))
            {
                s = "Glacier2/router:ssl -p 5064 -h " + hostname + " -t 10000";
            }
            else
            {
                s = "Glacier2/router:ssl -p 4064 -h " + hostname + " -t 10000";
            }
        }
        else
        {
            s = "Glacier2/router:tcp -p 4502 -h " + hostname + " -t 10000";
        }

        Ice.ObjectPrx proxy = _communicator.stringToProxy(s);
        Ice.RouterPrx r = Ice.RouterPrxHelper.uncheckedCast(proxy);

        _communicator.setDefaultRouter(r);

        _router = Glacier2.RouterPrxHelper.checkedCast(r);
        Glacier2.SessionPrx glacier2session = _router.createSession(username, password);
        _session = Chat.ChatSessionPrxHelper.uncheckedCast(glacier2session);

        Ice.ObjectAdapter adapter = _communicator.createObjectAdapterWithRouter("ChatDemo.Client", _router);

        Ice.Identity callbackId = new Ice.Identity();
        callbackId.name = UUID.randomUUID().toString();
        callbackId.category = _router.getCategoryForClient();

        Ice.ObjectPrx cb = adapter.add(new ChatCallbackI(), callbackId);
        _session.setCallback(Chat.ChatRoomCallbackPrxHelper.uncheckedCast(cb));

        adapter.activate();
        _lastSend = System.currentTimeMillis();

        _refreshTimeout = (_router.getSessionTimeout() * 1000) / 2;
    }

    synchronized public long getRefreshTimeout()
    {
        return _refreshTimeout;
    }

    synchronized public void destroy()
    {
        if(_destroyed)
        {
            return;
        }
        _destroyed = true;

        _session = null;
        new Thread(new Runnable()
        {
            public void run()
            {
                try
                {
                    _router.destroySession();
                }
                catch(Exception ex)
                {
                }

                try
                {
                    _communicator.destroy();
                }
                catch(Ice.LocalException e)
                {
                }
                _communicator = null;
            }
        }).start();
    }

    synchronized public String getError()
    {
        return _error;
    }

    // This method is only called by the UI thread.
    synchronized public void send(String t)
    {
        if(_destroyed)
        {
            return;
        }

        _lastSend = System.currentTimeMillis();
        Chat.Callback_ChatSession_send cb = new Chat.Callback_ChatSession_send()
        {
            @Override
            public void exception(Ice.LocalException ex)
            {
                destroyWithError(ex.toString());
            }

            @Override
            public void exception(Ice.UserException ex)
            {
                destroyWithError(ex.toString());
            }

            @Override
            public void response(long ret)
            {
            }
        };
        _session.begin_send(t, cb);
    }

    // This method is only called by the UI thread.
    public synchronized String addChatRoomListener(ChatRoomListener cb, boolean replay)
    {
        _listeners.add(cb);
        cb.init(_users);

        if(replay)
        {
            // Replay the entire state.
            for(ChatEventReplay r : _replay)
            {
                r.replay(cb);
            }
        }

        if(_error != null)
        {
            cb.error();
        }

        return _hostname;
    }

    // This method is only called by the UI thread.
    synchronized public void removeChatRoomListener(ChatRoomListener cb)
    {
        _listeners.remove(cb);
    }

    // Returns false if the session has been destroyed, true otherwise.
    synchronized public boolean refresh()
    {
        if(_destroyed)
        {
            return false;
        }

        // If the user has not sent a message in the INACTIVITY_TIMEOUT interval
        // then drop the session.
        if(System.currentTimeMillis() - _lastSend > INACTIVITY_TIMEOUT)
        {
            destroy();
            _error = "The session was dropped due to inactivity.";

            final List<ChatRoomListener> copy = new ArrayList<ChatRoomListener>(_listeners);
            _handler.post(new Runnable()
            {
                public void run()
                {
                    for(ChatRoomListener listener : copy)
                    {
                        listener.inactivity();
                    }
                }
            });
            return false;
        }

        try
        {
            _session.begin_ice_ping(new Ice.Callback_Object_ice_ping()
                {
                    @Override
                    public void response()
                    {
                    }

                    @Override
                    public void exception(final Ice.LocalException ex)
                    {
                        destroyWithError(ex.toString());
                    }
                });
        }
        catch(Ice.LocalException e)
        {
            destroyWithError(e.toString());
            return false;
        }

        return true;
    }

    private interface ChatEventReplay
    {
        public void replay(ChatRoomListener cb);
    }

    private class ChatCallbackI extends Chat._ChatRoomCallbackDisp
    {
        synchronized public void init(String[] users, Ice.Current current)
        {
            final List<String> u = Arrays.asList(users);
            _users.clear();
            _users.addAll(u);

            // No replay event for init.
            final List<ChatRoomListener> copy = new ArrayList<ChatRoomListener>(_listeners);
            _handler.post(new Runnable()
            {
                public void run()
                {
                    for(ChatRoomListener listener : copy)
                    {
                        listener.init(u);
                    }
                }
            });
        }

        synchronized public void join(final long timestamp, final String name, Ice.Current current)
        {
            _replay.add(new ChatEventReplay()
            {
                public void replay(ChatRoomListener cb)
                {
                    cb.join(timestamp, name);
                }
            });
            if(_replay.size() > MAX_MESSAGES)
            {
                _replay.removeFirst();
            }

            _users.add(name);

            final List<ChatRoomListener> copy = new ArrayList<ChatRoomListener>(_listeners);
            _handler.post(new Runnable()
            {
                public void run()
                {
                    for(ChatRoomListener listener : copy)
                    {
                        listener.join(timestamp, name);
                    }
                }
            });
        }

        synchronized public void leave(final long timestamp, final String name, Ice.Current current)
        {
            _replay.add(new ChatEventReplay()
            {
                public void replay(ChatRoomListener cb)
                {
                    cb.leave(timestamp, name);
                }
            });
            if(_replay.size() > MAX_MESSAGES)
            {
                _replay.removeFirst();
            }

            _users.remove(name);

            final List<ChatRoomListener> copy = new ArrayList<ChatRoomListener>(_listeners);
            _handler.post(new Runnable()
            {
                public void run()
                {
                    for(ChatRoomListener listener : copy)
                    {
                        listener.leave(timestamp, name);
                    }
                }
            });
        }

        synchronized public void send(final long timestamp, final String name, final String message, Ice.Current current)
        {
            _replay.add(new ChatEventReplay()
            {
                public void replay(ChatRoomListener cb)
                {
                    cb.send(timestamp, name, message);
                }
            });
            if(_replay.size() > MAX_MESSAGES)
            {
                _replay.removeFirst();
            }

            final List<ChatRoomListener> copy = new ArrayList<ChatRoomListener>(_listeners);
            _handler.post(new Runnable()
            {
                public void run()
                {
                    for(ChatRoomListener listener : copy)
                    {
                        listener.send(timestamp, name, message);
                    }
                }
            });
        }
        
        public static final long serialVersionUID = 1;
    }

    // Any exception destroys the session.
    synchronized private void destroyWithError(final String msg)
    {
        destroy();
        _error = msg;

        final List<ChatRoomListener> copy = new ArrayList<ChatRoomListener>(_listeners);
        _handler.post(new Runnable()
        {
            public void run()
            {
                for(ChatRoomListener listener : copy)
                {
                    listener.error();
                }
            }
        });
    }
}
