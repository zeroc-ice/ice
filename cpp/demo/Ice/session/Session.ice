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

/** XXX Comment missing XXX **/

interface Session
{
    /**
     *
     * Refresh this session. If a session is not refreshed on a
     * regular basis by the client it will be automatically destroyed.
     *
     **/
    void refresh();

    /**
     *
     * Callback that the session has been destroyed. XXX I don't understand this method. Who calls this? Isn't this server-side only? If so, it doesn't belong into the contract. XXX
     *
     **/
    void destroyed();

    /**
     *
     * Destroy this session.
     *
     **/
    void destroy();
};

/** XXX Comment missing. Why "SessionManager"? From the contract perspective, this is a SessionFactory. XXX **/
interface SessionManager
{
    /**
     *
     * Create a new session.
     *
     **/
    Session* create();

    /**
     *
     * Shutdown the server.
     *
     **/
    void shutdown();
};

};

#endif
