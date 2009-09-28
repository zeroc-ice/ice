// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_STRING_ADAPTER_INFO_DICT_H
#define SQL_STRING_ADAPTER_INFO_DICT_H

#include <Ice/CommunicatorF.h>
#include <IceGrid/Admin.h>
#include <IceSQL/SqlTypes.h>

namespace IceGrid
{

typedef std::map<std::string, AdapterInfo> StringAdapterInfoDict;

class SqlStringAdapterInfoDict : public IceUtil::Shared
{
public:

    SqlStringAdapterInfoDict(const IceSQL::DatabaseConnectionPtr&, const std::string&, const Ice::CommunicatorPtr&);

    void put(const IceSQL::DatabaseConnectionPtr&, const AdapterInfo&);

    AdapterInfo find(const IceSQL::DatabaseConnectionPtr&, const std::string&);
    AdapterInfoSeq findByReplicaGroupId(const IceSQL::DatabaseConnectionPtr&, const std::string&);
    void getMap(const IceSQL::DatabaseConnectionPtr&, StringAdapterInfoDict&);

    void erase(const IceSQL::DatabaseConnectionPtr&, const std::string&);
    void clear(const IceSQL::DatabaseConnectionPtr&);

private:

    const Ice::CommunicatorPtr _communicator;
    const std::string _table;
};

typedef IceUtil::Handle<SqlStringAdapterInfoDict> SqlStringAdapterInfoDictPtr;

}

#endif
