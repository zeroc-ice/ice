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

interface Hello
{
    nonmutating void sayHello();
};

//
// The session object. This is used to create per-session objects on
// behalf of the client. If it is not refreshed on a periodic basis, it
// will be automatically reclaimed by the session factory.
//
// XXX Comment is wrong. First, sessions are not reclaimed - they are
// destroyed. Second, the factory doesn't do this. Third, it is
// irrelevant for the interface description to explain what
// programming language construct exactly destroys expired
// sessions. So this should simply read: "If it is not refreshed on a
// periodic basis, it will be automatically destroyed."
//
interface Session
{
    //
    // Create a new per-session hello object. The created object will
    // be automatically destroyed when the session is destroyed.
    //
    Hello* createHello();

    //
    // Refresh a session. If a session is not refreshed on a regular
    // basis by the client, it will be automatically destroyed.
    //
    idempotent void refresh();

    //
    // Destroy the session explicitly.
    //
    void destroy();
};

interface SessionFactory
{
    // XXX I think you should pass a user name parameter in here. The
    // client could then first ask "What is your name"? Then the
    // server could print messages like "Hello object #XXX says `Hello
    // Foo!'"
    Session* create();

    idempotent void shutdown();
};

};

#endif
