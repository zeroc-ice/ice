// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapter.h>
#include <IceGrid/SessionServantLocatorI.h>

using namespace std;
using namespace IceGrid;

SessionServantLocatorI::SessionServantLocatorI(const Ice::ObjectAdapterPtr& adapter, const string& instanceName) : 
    _adapter(adapter),
    _instanceName(instanceName)
{
}

Ice::ObjectPtr
SessionServantLocatorI::locate(const Ice::Current& current, Ice::LocalObjectPtr&)
{
    Lock sync(*this);
    map<Ice::Identity, SessionServant>::const_iterator p = _servants.find(current.id);
    if(p == _servants.end() || p->second.connection != current.con)
    {
        return 0;
    }
    return p->second.servant;
}

void
SessionServantLocatorI::finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&)
{
}

void
SessionServantLocatorI::deactivate(const std::string&)
{
    Lock sync(*this);
    _servants.clear();
}

Ice::ObjectPrx
SessionServantLocatorI::add(const Ice::ObjectPtr& servant, const Ice::ConnectionPtr& con)
{
    Lock sync(*this);
    Ice::Identity id;
    id.name = IceUtil::generateUUID();
    id.category = _instanceName;
    if(!_servants.insert(make_pair(id, SessionServant(servant, con))).second)
    {
        throw Ice::AlreadyRegisteredException(__FILE__, __LINE__, "servant", id.name);
    }
    return _adapter->createProxy(id);
}

void
SessionServantLocatorI::remove(const Ice::Identity& id)
{
    Lock sync(*this);
    _servants.erase(id);
}
