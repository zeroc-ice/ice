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

// XXX Regarding the /** comments: Either do it correctly, or get rid
// of them (use normal comments, use //). If you want to use /**
// properly, then add parameter and return value descriptions. I do
// not think that this is necessary, so I recommend to only use normal
// comments. Then you can also get rid of trivial comments, like
// "Hello object".

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
    idempotent void refresh();

    // XXX "Mark" as destroyed? This should read "Destroy the
    // session". Whether or not it's marked internally is irrelevant
    // for the description of the interface.

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
