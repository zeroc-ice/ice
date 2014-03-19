// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package com.zeroc.chat.service;

public interface SessionListener
{
    /** Called when a login is in progress. */
    void onLoginInProgress();

    /** Called when a new session has been established. */
    void onLogin();

    /** Called to confirm a connection to a server. */
    void onConnectConfirm();

    /**
     * Called if the establishment of a session failed.
     *
     * @param ex
     *            The error.
     */
    void onLoginError();
}
