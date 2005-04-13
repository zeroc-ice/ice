// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SESSION_ICE
#define SESSION_ICE

module Demo
{

interface Session
{
    /**
     * Refresh a session. If a session is not refreshed on a regular
     * basis by the client it will be automatically destroyed.
     *
     **/
    void refresh();

    /**
     * Callback that the session has been destroyed.
     *
     **/
    void destroyed();

    /**
     * Destroy a session.
     *
     **/
    void destroy();
};

interface SessionManager
{
    /**
     * Create a new session.
     *
     **/
    Session* create();

    /**
     * Shutdown the server
     *
     **/
    void shutdown();
};

};

#endif
