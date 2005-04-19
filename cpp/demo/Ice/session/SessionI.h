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

// XXX Missing #includes. The #includes for the header must be
// self-contained, i.e. there must be no problem if you just do
// #include<SessionFactoryI.h> from an empty .cpp file. (I know that
// some other demos don't follow this rule, but they need to be
// fixed. We shouldn't propagate such mistakes into new demos.)
#include <Session.h>
#include <list>

class SessionI : public Demo::Session, public IceUtil::Mutex
{
public:

    virtual Demo::HelloPrx createHello(const Ice::Current&);
    virtual void refresh(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

private:

    // Only the ReapThread is interested in the timestamp.
    friend class ReapThread;
    IceUtil::Time timestamp() const;

    // Only the session factory can create sessions.
    friend class SessionFactoryI;
    SessionI();

    IceUtil::Time _timestamp; // The last time the session was refreshed.

    // XXX This needs to be a static, otherwise hello objects from different client have the same ID.
    int _nextId; // The id of the next hello object. This is used for tracing purposes.
    std::list< Demo::HelloPrx> _objs; // List of per-client allocated Hello objects.
    bool _destroy;
};

typedef IceUtil::Handle<SessionI> SessionIPtr;

#endif
