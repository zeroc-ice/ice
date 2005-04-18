// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_REGISTRYI_H
#define ICE_GRID_REGISTRYI_H

#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class RegistryI : public Registry
{
public:

    RegistryI(const Ice::CommunicatorPtr&);
    ~RegistryI();

    bool start(bool);

    virtual Ice::StringSeq registerNode(const std::string& name, const NodePrx&, const Ice::Current&);
    virtual void unregisterNode(const std::string& name, const Ice::Current&);
    virtual void shutdown(const Ice::Current& current);

private:

    Ice::CommunicatorPtr _communicator;
    DatabasePtr _database;
};
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

}

#endif
