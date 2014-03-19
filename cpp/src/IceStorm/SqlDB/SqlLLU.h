// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_LLU_H
#define SQL_LLU_H

#include <IceDB/SqlTypes.h>

#include <IceStorm/LLURecord.h>

namespace IceStorm
{

class SqlLLU : public IceUtil::Shared
{
public:

    SqlLLU(const SqlDB::DatabaseConnectionPtr&, const std::string&);

    void put(const SqlDB::DatabaseConnectionPtr&, const IceStormElection::LogUpdate&);
    IceStormElection::LogUpdate get(const SqlDB::DatabaseConnectionPtr&) const;

private:

    const std::string _table;
};

typedef IceUtil::Handle<SqlLLU> SqlLLUPtr;

}

#endif
