// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SESSIONSERVANTLOCATOR_H
#define ICE_GRID_SESSIONSERVANTLOCATOR_H

#include <IceUtil/Mutex.h>
#include <Ice/ServantLocator.h>

namespace IceGrid
{

class SessionServantLocatorI : public Ice::ServantLocator, public IceUtil::Mutex
{
public:

    SessionServantLocatorI(const Ice::ObjectAdapterPtr&, const std::string&);

    Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    void deactivate(const std::string&);

    Ice::ObjectPrx add(const Ice::ObjectPtr&, const Ice::ConnectionPtr&);
    void remove(const Ice::Identity&);

private:

    struct SessionServant
    {
        SessionServant(const Ice::ObjectPtr& s, const Ice::ConnectionPtr& con) : servant(s), connection(con) { }

        const Ice::ObjectPtr servant;
        const Ice::ConnectionPtr connection;
    };

    const Ice::ObjectAdapterPtr _adapter;
    const std::string _instanceName;
    std::map<Ice::Identity, SessionServant> _servants;
};
typedef IceUtil::Handle<SessionServantLocatorI> SessionServantLocatorIPtr;

};

#endif
