// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_IDENTITY_OBJECT_INFO_DICT_H
#define SQL_IDENTITY_OBJECT_INFO_DICT_H

#include <Ice/CommunicatorF.h>
#include <IceGrid/Admin.h>
#include <IceDB/SqlTypes.h>

namespace IceGrid
{

typedef std::map<Ice::Identity, ObjectInfo> IdentityObjectInfoDict;

class SqlIdentityObjectInfoDict : public IceUtil::Shared
{
public:

    SqlIdentityObjectInfoDict(const SqlDB::DatabaseConnectionPtr&, const std::string&, const Ice::CommunicatorPtr&);

    void put(const SqlDB::DatabaseConnectionPtr&, const Ice::Identity&, const ObjectInfo&);

    ObjectInfo find(const SqlDB::DatabaseConnectionPtr&, const Ice::Identity&);
    ObjectInfoSeq findByType(const SqlDB::DatabaseConnectionPtr&, const std::string&);
    void getMap(const SqlDB::DatabaseConnectionPtr&, IdentityObjectInfoDict&);

    void erase(const SqlDB::DatabaseConnectionPtr&, const Ice::Identity&);
    void clear(const SqlDB::DatabaseConnectionPtr&);

private:

    const Ice::CommunicatorPtr _communicator;
    const std::string _table;
};

typedef IceUtil::Handle<SqlIdentityObjectInfoDict> SqlIdentityObjectInfoDictPtr;

}

#endif
