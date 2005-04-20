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

#include <Ice/Ice.h>
#include <IceUtil/StaticMutex.h>
#include <Session.h>
#include <list>

class SessionI : public Demo::Session, public IceUtil::Mutex
{
public:

    virtual Demo::HelloPrx createHello(const Ice::Current&);
    virtual void refresh(const Ice::Current&);
    virtual void destroy(const Ice::Current&);
    virtual std::string getName(const Ice::Current&) const;

private:

    // Only the ReapThread is interested in the timestamp.
    friend class ReapThread;
    IceUtil::Time timestamp() const;

    // Only the session factory can create sessions.
    friend class SessionFactoryI;
    SessionI(const std::string&);

    const std::string _name;
    IceUtil::Time _timestamp; // The last time the session was refreshed.

    //
    // XXX This needs to be a static, otherwise hello objects from
    // different client have the same ID.
    //
    // The reason I didn't make the _nextId static in the first place
    // was because the client also displays the index of the hello
    // object. I think displaying a different id on the client &
    // server will be confusing, and I didn't think it was worth
    // making the client keep a real id->hello object client map
    // because a) it would make the client more complicated and b) it
    // would require the user to type bigger and bigger numbers all
    // the time which is a pain in the ass.
    //
    int _nextId; // The per-session id of the next hello object. This is used for tracing purposes.
    std::list< Demo::HelloPrx> _objs; // List of per-client allocated Hello objects.
    bool _destroy;
};

typedef IceUtil::Handle<SessionI> SessionIPtr;

#endif
