// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package com.zeroc.chat.service;

import android.app.AlarmManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.SystemClock;

import com.zeroc.chat.ChatActivity;
import com.zeroc.chat.R;

public class ChatService extends Service implements com.zeroc.chat.service.Service
{
    private static final int CHATACTIVE_NOTIFICATION = 0;
    private static final String REFRESH_EXTRA = "refresh";
    private AppSession _session = null;
    private boolean _confirmConnectionInProgress = false;
    private SessionListener _listener;
    private boolean _loginInProgress;
    private Handler _handler;
    private String _loginError;

    public class LocalBinder extends Binder
    {
        public com.zeroc.chat.service.Service getService()
        {
            return ChatService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent)
    {
        return new LocalBinder();
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        _handler = new Handler();
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
        logout();
    }

    @Override
    synchronized public void onStart(Intent intent, int startId)
    {
        // Find out if we were started by the alarm manager
        // to refresh the current session.
        if(intent.hasExtra(REFRESH_EXTRA))
        {
            // If there is no associated session, or the refresh failed then
            // mark the session as destroyed.
            if(_session == null || !_session.refresh())
            {
                sessionDestroyed();
            }
        }
    }

    // Called only from the UI thread.
    synchronized public boolean setSessionListener(SessionListener cb)
    {
        _listener = cb;
        if(_listener == null)
        {
            return false;
        }
        if(_loginInProgress)
        {
            if(_confirmConnectionInProgress)
            {
                _listener.onConnectConfirm();
            }
            else
            {
                _listener.onLoginInProgress();
            }
        }
        else if(_session != null)
        {
            _listener.onLogin();
        }
        else if(_loginError != null)
        {
            _listener.onLoginError();
        }
        return _loginInProgress;
    }

    synchronized public void logout()
    {
        if(_session != null)
        {
            _session.destroy();
            _session = null;

            sessionDestroyed();
        }
    }

    synchronized public void login(final String hostname, final String username, final String password,
                                   final boolean secure)
    {
        assert _session == null;
        assert !_loginInProgress;

        _loginError = null;
        _loginInProgress = true;

        new Thread(new Runnable()
        {
            public void run()
            {
                try
                {
                    loginComplete(new AppSession(getResources(), _handler, hostname, username, password, secure), hostname);
                }
                catch(final Glacier2.CannotCreateSessionException ex)
                {
                    ex.printStackTrace();
                    postLoginFailure(String.format("Session creation failed: %s", ex.toString()));
                }
                catch(final Glacier2.PermissionDeniedException ex)
                {
                    ex.printStackTrace();
                    postLoginFailure(String.format("Login failed: %s", ex.toString()));
                }
                catch(final Ice.LocalException ex)
                {
                    ex.printStackTrace();
                    postLoginFailure(String.format("Login failed: %s", ex.toString()));
                }
                finally
                {
                    synchronized(ChatService.this)
                    {
                        _loginInProgress = false;
                    }
                }
            }
        }).start();
    }

    synchronized public String addChatRoomListener(ChatRoomListener listener, boolean replay) throws NoSessionException
    {
        if(_session == null)
        {
            throw new NoSessionException();
        }
        return _session.addChatRoomListener(listener, replay);
    }

    synchronized public void removeChatRoomListener(ChatRoomListener listener)
    {
        if(_session != null)
        {
            _session.removeChatRoomListener(listener);
        }
    }

    synchronized public void send(String message)
    {
        if(_session != null)
        {
            _session.send(message);
        }
    }

    synchronized public String getLoginError()
    {
        return _loginError;
    }

    synchronized public String getSessionError()
    {
        return _session.getError();
    }

    synchronized private void postLoginFailure(final String loginError)
    {
        _loginError = loginError;
        if(_listener != null)
        {
            final SessionListener listener = _listener;
            _handler.post(new Runnable()
            {
                public void run()
                {
                    listener.onLoginError();
                }
            });
        }
    }

    synchronized private void loginComplete(AppSession session, String hostname)
    {
        _session = session;

        // Set up an alarm to refresh the session.
        Intent intent = new Intent(ChatService.this, ChatService.class);
        intent.putExtra(REFRESH_EXTRA, true);
        PendingIntent sender = PendingIntent.getService(ChatService.this, 0, intent, 0);

        long refreshTimeout = _session.getRefreshTimeout();

        long firstTime = SystemClock.elapsedRealtime() + refreshTimeout;

        AlarmManager am = (AlarmManager)getSystemService(ALARM_SERVICE);
        am.setRepeating(AlarmManager.ELAPSED_REALTIME_WAKEUP, firstTime, refreshTimeout, sender);

        // Display a notification that the user is logged in.
        Notification notification = new Notification(R.drawable.stat_notify, "Logged In", System.currentTimeMillis());
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, new Intent(this, ChatActivity.class), 0);
        notification.setLatestEventInfo(this, "Chat Demo", "You are logged into " + hostname, contentIntent);
        NotificationManager n = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        n.notify(CHATACTIVE_NOTIFICATION, notification);

        if(_listener != null)
        {
            final SessionListener listener = _listener;

            _handler.post(new Runnable()
            {
                public void run()
                {
                    listener.onLogin();
                }
            });
        }
    }

    private void cancelRefreshTimer()
    {
        Intent intent = new Intent(ChatService.this, ChatService.class);
        intent.putExtra(REFRESH_EXTRA, true);
        PendingIntent sender = PendingIntent.getService(ChatService.this, 0, intent, 0);

        // And cancel the alarm.
        AlarmManager am = (AlarmManager)getSystemService(ALARM_SERVICE);
        am.cancel(sender);
    }

    private void sessionDestroyed()
    {
        cancelRefreshTimer();
        NotificationManager n = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        // Cancel the notification -- we use the same ID that we had used to
        // start it
        n.cancel(CHATACTIVE_NOTIFICATION);
    }
}
