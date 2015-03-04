// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.chat;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.IBinder;
import android.text.Editable;
import android.text.TextWatcher;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.os.Build.VERSION;

import com.zeroc.chat.service.ChatService;
import com.zeroc.chat.service.Service;
import com.zeroc.chat.service.SessionListener;

public class LoginActivity extends Activity
{
    private static final int DIALOG_ERROR = 1;
    private static final int DIALOG_CONFIRM = 2;
    private static final int DIALOG_INVALID_HOST = 3;

    private static final String DEFAULT_HOST = "demo.zeroc.com";
    private static final boolean DEFAULT_SECURE = false;
    private static final String HOSTNAME_KEY = "host";
    private static final String USERNAME_KEY = "username";
    private static final String PASSWORD_KEY = "password";
    private static final String SECURE_KEY = "secure";

    private Button _login;
    private EditText _hostname;
    private EditText _username;
    private EditText _password;
    private CheckBox _secure;
    private SharedPreferences _prefs;

    private boolean _loginInProgress = false;
    private Service _service;
    private Intent _chatServiceIntent;

    private SessionListener _listener = new SessionListener()
    {
        public void onLoginInProgress()
        {
            _loginInProgress = true;
            setLoginState();
        }

        public void onConnectConfirm()
        {
            showDialog(DIALOG_CONFIRM);
        }

        public void onLogin()
        {
            startActivity(new Intent(LoginActivity.this, ChatActivity.class));
        }

        public void onLoginError()
        {
            setLoginState();
            showDialog(DIALOG_ERROR);
        }
    };

    final private ServiceConnection _connection = new ServiceConnection()
    {
        public void onServiceConnected(ComponentName name, IBinder service)
        {
            // This is called when the connection with the service has been
            // established, giving us the service object we can use to
            // interact with the service. Because we have bound to a explicit
            // service that we know is running in our own process, we can
            // cast its IBinder to a concrete class and directly access it.
            setLoginState();

            _service = ((com.zeroc.chat.service.ChatService.LocalBinder)service).getService();
            _service.setSessionListener(_listener);
        }

        public void onServiceDisconnected(ComponentName name)
        {
        }
    };

    private void setLoginState()
    {
        if(_loginInProgress)
        {
            _login.setEnabled(false);
        }
        else
        {
            String host = _hostname.getText().toString().trim();
            String username = _username.getText().toString().trim();
            _login.setEnabled(host.length() > 0 && username.length() > 0);
        }
    }

    private void login()
    {
        final String hostname = _hostname.getText().toString().trim();
        final String username = _username.getText().toString().trim();
        final String password = _password.getText().toString().trim();
        final boolean secure = _secure.isChecked();

        // We don't want to save obviously bogus hostnames in the application
        // preferences. These two regexp validates that the hostname is well
        // formed.
        // Note that this regexp doesn't handle IPv6 addresses.
        final String hostre = "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\\-]*[A-Za-z0-9])$";
        final String ipre = "^([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";
        if(!hostname.matches(hostre) && !hostname.matches(ipre))
        {
            showDialog(DIALOG_INVALID_HOST);
            return;
        }

        // Update preferences.
        SharedPreferences.Editor edit = _prefs.edit();
        if(!_prefs.getString(HOSTNAME_KEY, DEFAULT_HOST).equals(hostname))
        {
            edit.putString(HOSTNAME_KEY, hostname);
        }
        if(!_prefs.getString(USERNAME_KEY, "").equals(username))
        {
            edit.putString(USERNAME_KEY, username);
        }
        if(!_prefs.getString(PASSWORD_KEY, "").equals(password))
        {
            edit.putString(PASSWORD_KEY, password);
        }
        if(_prefs.getBoolean(SECURE_KEY, DEFAULT_SECURE) != secure)
        {
            edit.putBoolean(SECURE_KEY, secure);
        }
        edit.commit();

        _login.setEnabled(false);

        // Kick off the login process. The activity is notified of changes
        // in the login process through calls to the SessionListener.
        _service.login(hostname, username, password, secure);
    }

    @Override
    protected void onResume()
    {
        super.onResume();

        bindService(_chatServiceIntent, _connection, BIND_AUTO_CREATE);
    }

    @Override
    public void onStop()
    {
        super.onStop();
        unbindService(_connection);

        if(_service != null)
        {
            _service.setSessionListener(null);
            _service = null;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.login);

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

        _login = (Button)findViewById(R.id.login);
        _login.setOnClickListener(new android.view.View.OnClickListener()
        {
            public void onClick(android.view.View v)
            {
                login();
            }
        });
        _login.setEnabled(false);

        _hostname = (EditText)findViewById(R.id.hostname);
        _hostname.addTextChangedListener(new TextWatcher()
        {
            public void afterTextChanged(Editable s)
            {
                setLoginState();
            }

            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {
            }

            public void onTextChanged(CharSequence s, int start, int count, int after)
            {
            }
        });

        _username = (EditText)findViewById(R.id.username);
        _username.addTextChangedListener(new TextWatcher()
        {
            public void afterTextChanged(Editable s)
            {
                setLoginState();
            }

            public void beforeTextChanged(CharSequence s, int start, int count, int after)
            {
            }

            public void onTextChanged(CharSequence s, int start, int count, int after)
            {
            }
        });
        _password = (EditText)findViewById(R.id.password);

        _secure = (CheckBox)findViewById(R.id.secure);
        _secure.setEnabled(VERSION.SDK_INT >= 8); // android.os.Build.VERSION_CODES.FROYO (8)

        _prefs = getPreferences(MODE_PRIVATE);

        if(savedInstanceState == null)
        {
            _hostname.setText(_prefs.getString(HOSTNAME_KEY, DEFAULT_HOST));
            _username.setText(_prefs.getString(USERNAME_KEY, ""));
            _password.setText(_prefs.getString(PASSWORD_KEY, ""));
            _secure.setChecked(_prefs.getBoolean(SECURE_KEY, DEFAULT_SECURE));
        }

        // Start the ChatService.
        _chatServiceIntent = new Intent(LoginActivity.this, ChatService.class);
        startService(_chatServiceIntent);
    }

    @Override
    protected Dialog onCreateDialog(int id)
    {
        switch (id)
        {
        case DIALOG_ERROR:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage(""); // Details provided in onPrepareDialog
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    _loginInProgress = false;
                    setLoginState();
                }
            });
            return builder.create();
        }

        case DIALOG_INVALID_HOST:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage("The hostname is invalid.");
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                }
            });
            return builder.create();
        }

        }

        return null;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog)
    {
        super.onPrepareDialog(id, dialog);
        if(id == DIALOG_ERROR)
        {
            AlertDialog alert = (AlertDialog)dialog;
            alert.setMessage(_service.getLoginError());
        }
    }
}
