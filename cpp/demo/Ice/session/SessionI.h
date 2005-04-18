// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SESSION_I_H
#define SESSION_I_H

#include <Session.h>
#include <list>

// XXX Get rid of leading ::, i.e., use Demo::, not ::Demo::
// (everywhere).

class SessionI : public ::Demo::Session, public IceUtil::Mutex
{
public:

    // XXX Should be private, with SessionFactoryI being a friend.
    SessionI(const Ice::ObjectAdapterPtr&, const IceUtil::Time&);
    virtual ~SessionI();

    virtual ::Demo::HelloPrx createHello(const Ice::Current&);
    virtual void refresh(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    // XXX Should be private, with SessionFactoryI being a friend. (Or Reaper being a friend, see the other comments.)
    // Return true if the session is destroyed, false otherwise.
    bool destroyed() const;

    // XXX Should be private, with SessionFactoryI being a friend. (Or Reaper being a friend, see the other comments.)
    // XXX The name is wrong. It's not a callback, it's a call.
    // XXX Why have this function at all? Why not do whatever this function does directly in destroy()?
    // per-client allocated resources.
    void destroyCallback();

private:

    const Ice::ObjectAdapterPtr _adapter; // XXX Get rid of this (you can after all the other XXX's have been fixed).

    // XXX Get rid of this. Only the reaper has to know this. It can
    // call a timestamp() function, compare the elapsed time with its
    // _timeout value, and call destroy() and reap if timed out.

    const IceUtil::Time _timeout; // How long until the session times out.
    bool _destroy; // true if destroy() was called, false otherwise.

    // XXX Rename to _timestamp;
    IceUtil::Time _refreshTime; // The last time the session was refreshed.
    
    int _nextId; // The id of the next hello object. This is used for tracing purposes.
    std::list< ::Demo::HelloPrx> _objs; // List of per-client allocated Hello objects.
};
typedef IceUtil::Handle<SessionI> SessionIPtr;

#endif
