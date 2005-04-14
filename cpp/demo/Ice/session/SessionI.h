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

class SessionI : public ::Demo::Session, public IceUtil::Mutex
{
public:

    SessionI(const Ice::ObjectAdapterPtr&, const IceUtil::Time&);
    virtual ~SessionI();

    virtual ::Demo::HelloPrx createHello(const Ice::Current&);
    virtual void refresh(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    // Return true if the session is destroyed, false otherwise.
    bool destroyed() const;

    // Called when the session is destroyed. This should release any
    // per-client allocated resources.
    void destroyCallback();

private:

    const Ice::ObjectAdapterPtr _adapter;
    const IceUtil::Time _timeout; // How long until the session times out.
    bool _destroy; // true if destroy() was called, false otherwise.
    IceUtil::Time _refreshTime; // The last time the session was refreshed.
    
    int _nextId; // The id of the next hello object. This is used for tracing purposes.
    std::list< ::Demo::HelloPrx> _objs; // List of per-client allocated Hello objects.
};
typedef IceUtil::Handle<SessionI> SessionIPtr;

#endif
