// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build.VERSION;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;

import com.zeroc.library.controller.LoginController;
import com.zeroc.library.controller.SessionController;

public class LoginActivity extends Activity
{
    private static final String DEFAULT_HOST = "demo2.zeroc.com";
    private static final boolean DEFAULT_SECURE = false;
    private static final boolean DEFAULT_GLACIER2 = false;
    private static final String HOSTNAME_KEY = "host";
    private static final String SECURE_KEY = "secure";
    private static final String GLACIER2_KEY = "glacier2";

    private static final int DIALOG_ERROR = 1;
    private static final int DIALOG_INVALID_HOST = 2;

    private Button _login;
    private EditText _hostname;
    private CheckBox _glacier2;
    private CheckBox _secure;
    private SharedPreferences _prefs;
    private LoginController _loginController;
    private boolean _loginInProgress = false;

    private LoginController.Listener _listener = new LoginController.Listener()
    {
        public void onLoginInProgress()
        {
            _loginInProgress = true;
            setLoginState();
        }

        public void onLogin(SessionController sessionController)
        {
            if(isFinishing())
            {
                return;
            }

            LibraryApp app = (LibraryApp)getApplication();
            app.loggedIn(sessionController);
            startActivity(new Intent(LoginActivity.this, LibraryActivity.class));
        }

        public void onLoginError()
        {
            _loginInProgress = false;
            setLoginState();

            showDialog(DIALOG_ERROR);
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
            _login.setEnabled(host.length() > 0);
        }
    }

    private void login()
    {
        final String hostname = _hostname.getText().toString().trim();
        final boolean secure = _secure.isChecked();
        final boolean glacier2 = _glacier2.isChecked();

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
        if(_prefs.getBoolean(SECURE_KEY, DEFAULT_SECURE) != secure)
        {
            edit.putBoolean(SECURE_KEY, secure);
        }
        if(_prefs.getBoolean(GLACIER2_KEY, DEFAULT_GLACIER2) != glacier2)
        {
            edit.putBoolean(GLACIER2_KEY, glacier2);
        }

        edit.commit();

        LibraryApp app = (LibraryApp)getApplication();
        _loginController = app.login(hostname, secure, glacier2, _listener);
    }

    /** Called when the activity is first created. */
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

        _login = (Button) findViewById(R.id.login);
        _login.setOnClickListener(new android.view.View.OnClickListener()
        {
            public void onClick(android.view.View v)
            {
                login();
            }
        });
        _hostname = (EditText) findViewById(R.id.hostname);
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

        _secure = (android.widget.CheckBox) findViewById(R.id.secure);
        _secure.setEnabled(VERSION.SDK_INT >= 8); // android.os.Build.VERSION_CODES.FROYO (8)

        _glacier2 = (android.widget.CheckBox) findViewById(R.id.glacier2);

        _prefs = getPreferences(MODE_PRIVATE);

        _prefs = getPreferences(MODE_PRIVATE);
        if(savedInstanceState == null)
        {
            _hostname.setText(_prefs.getString(HOSTNAME_KEY, DEFAULT_HOST));
            _secure.setChecked(_prefs.getBoolean(SECURE_KEY, DEFAULT_SECURE));
            _glacier2.setChecked(_prefs.getBoolean(GLACIER2_KEY, DEFAULT_GLACIER2));
        }
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        LibraryApp app = (LibraryApp)getApplication();
        _loginInProgress = false;
        _loginController = app.getLoginController();
        if(_loginController != null)
        {
            _loginController.setLoginListener(_listener);
        }
        else
        {
            setLoginState();
        }
    }

    @Override
    protected void onStop()
    {
        super.onStop();
        if(_loginController != null)
        {
            _loginController.setLoginListener(null);
            _loginController = null;
        }
    }

    @Override
    protected Dialog onCreateDialog(int id)
    {
        switch(id)
        {
        case DIALOG_ERROR:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage(""); // The message is filled in onPrepareDialog.
            builder.setCancelable(false);
            builder.setPositiveButton("Ok", new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    // Clean up the login controller upon login failure.
                    if(_loginController != null)
                    {
                        LibraryApp app = (LibraryApp)getApplication();
                        app.loginFailure();
                    }
                }
            });
            return builder.create();
        }

        case DIALOG_INVALID_HOST:
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage("The hostname is invalid");
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
            alert.setMessage(_loginController.getLoginError());
        }
    }
}
