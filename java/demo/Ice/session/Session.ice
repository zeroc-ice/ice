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

/** Hello object. */
interface Hello
{
    /** Send a greeting. */
    nonmutating void sayHello();
};

/**
 * The session object. This is used to create per-session objects on
 * behalf of the client. If it is not refreshed on a periodic basis it
 * will be automatically reclaimed by the session factory.
 */
interface Session
{
    /**
     * Create a new per-session hello object. The created object will
     * be automatically destroyed when the session is destroyed.
     */
    Hello* createHello();

    /**
     * Refresh a session. If a session is not refreshed on a regular
     * basis by the client it will be automatically destroyed.
     **/
    void refresh();

    /** Mark the session as destroyed. */
    void destroy();
};

/** The SessionFactory. */
interface SessionFactory
{
    /** Create a new session. */
    Session* create();

    /** Shutdown the server. */
    idempotent void shutdown();
};

};

#endif
