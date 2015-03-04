// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library;

import com.zeroc.library.controller.LoginController;
import com.zeroc.library.controller.SessionController;
import com.zeroc.library.controller.LoginController.Listener;

import android.app.Application;

public class LibraryApp extends Application
{
    private LoginController _loginController;
    private SessionController _sessionController;

    @Override
    public void onCreate()
    {
    }

    @Override
    public void onTerminate()
    {
        if(_loginController != null)
        {
            _loginController.destroy();
            _loginController = null;
        }

        if(_sessionController != null)
        {
            _sessionController.destroy();
            _sessionController = null;
        }
    }

    public void loggedIn(SessionController sessionController)
    {
        assert _sessionController == null && _loginController != null;
        _sessionController = sessionController;

        _loginController.destroy();
        _loginController = null;
    }

    public void logout()
    {
        if(_sessionController != null)
        {
            _sessionController.destroy();
            _sessionController = null;
        }
    }

    public LoginController login(String hostname, boolean secure, boolean glacier2, Listener listener)
    {
        assert _loginController == null && _sessionController == null;
        _loginController = new LoginController(getResources(), hostname, secure, glacier2, listener);
        return _loginController;
    }

    public void loginFailure()
    {
        if(_loginController != null)
        {
            _loginController.destroy();
            _loginController = null;
        }
    }

    public SessionController getSessionController()
    {
        return _sessionController;
    }

    public LoginController getLoginController()
    {
        return _loginController;
    }
}
