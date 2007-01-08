// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SESSION_I_H
#define SESSION_I_H

#include <Ice/Ice.h>
#include <Session.h>
#include <list>

class SessionI : public Demo::Session, public IceUtil::Mutex
{
public:

    SessionI(const std::string&);

    virtual Demo::HelloPrx createHello(const Ice::Current&);
    virtual void refresh(const Ice::Current&);
    virtual std::string getName(const Ice::Current&) const;
    virtual void destroy(const Ice::Current&);

    IceUtil::Time timestamp() const;

private:

    const std::string _name;
    IceUtil::Time _timestamp; // The last time the session was refreshed.
    int _nextId; // The per-session id of the next hello object. This is used for tracing purposes.
    std::list<Demo::HelloPrx> _objs; // List of per-session allocated hello objects.
    bool _destroy;
};
typedef IceUtil::Handle<SessionI> SessionIPtr;

#endif
