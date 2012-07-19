// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/CommunicatorF.h>
#include <IceGrid/Admin.h>
#include <IceDB/SqlTypes.h>

namespace IceGrid
{

typedef std::map<std::string, AdapterInfo> StringAdapterInfoDict;

class SqlStringAdapterInfoDict : public IceUtil::Shared
{
public:

    SqlStringAdapterInfoDict(const SqlDB::DatabaseConnectionPtr&, const std::string&, const Ice::CommunicatorPtr&);

    void put(const SqlDB::DatabaseConnectionPtr&, const std::string&, const AdapterInfo&);

    AdapterInfo find(const SqlDB::DatabaseConnectionPtr&, const std::string&);
    AdapterInfoSeq findByReplicaGroupId(const SqlDB::DatabaseConnectionPtr&, const std::string&);
    void getMap(const SqlDB::DatabaseConnectionPtr&, StringAdapterInfoDict&);

    void erase(const SqlDB::DatabaseConnectionPtr&, const std::string&);
    void clear(const SqlDB::DatabaseConnectionPtr&);

private:

    const Ice::CommunicatorPtr _communicator;
    const std::string _table;
};

typedef IceUtil::Handle<SqlStringAdapterInfoDict> SqlStringAdapterInfoDictPtr;

}
