// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_STRING_APPLICATION_INFO_DICT_H
#define SQL_STRING_APPLICATION_INFO_DICT_H

#include <Ice/CommunicatorF.h>
#include <IceGrid/Admin.h>
#include <IceSQL/SqlTypes.h>

namespace IceGrid
{

typedef std::map<std::string, ApplicationInfo> StringApplicationInfoDict;

class SqlStringApplicationInfoDict : public IceUtil::Shared
{
public:

    SqlStringApplicationInfoDict(const IceSQL::DatabaseConnectionPtr&, const std::string&, const Ice::CommunicatorPtr&);

    void put(const IceSQL::DatabaseConnectionPtr&, const std::string&, const ApplicationInfo&);

    ApplicationInfo find(const IceSQL::DatabaseConnectionPtr&, const std::string&);
    void getMap(const IceSQL::DatabaseConnectionPtr&, StringApplicationInfoDict&);

    void erase(const IceSQL::DatabaseConnectionPtr&, const std::string&);
    void clear(const IceSQL::DatabaseConnectionPtr&);

private:

    const Ice::CommunicatorPtr _communicator;
    const std::string _table;
};

typedef IceUtil::Handle<SqlStringApplicationInfoDict> SqlStringApplicationInfoDictPtr;

}

#endif
