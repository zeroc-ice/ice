// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef FREEZE_SHARED_DB_H
#define FREEZE_SHARED_DB_H

#include <Freeze/ConnectionI.h>
#include <IceUtil/Handle.h>
#include <db_cxx.h>
#include <map>

namespace Freeze
{

class SharedDb;
typedef IceUtil::Handle<SharedDb> SharedDbPtr;

class SharedDb : public ::Db
{
public:
    
    using Db::get;

    static SharedDbPtr get(const ConnectionIPtr&, const std::string&, bool);

    ~SharedDb();

    void __incRef();
    void __decRef();

    const std::string& 
    dbName() const;

#ifdef __HP_aCC
    
    virtual int 
    get(DbTxn *txnid, Dbt *key, Dbt *data, u_int32_t flags)
    {
	::Db::get(txnid, key, data, flags);
    }
#endif

private:

    struct MapKey
    {
	std::string envName;
	Ice::CommunicatorPtr communicator;
	std::string dbName;

	inline bool
	operator<(const MapKey& rhs) const;
    };

    typedef std::map<MapKey, Freeze::SharedDb*> Map;
   
    SharedDb(const MapKey&, const ConnectionIPtr&, bool);
    
    MapKey _key;
    int _refCount;
    Ice::Int _trace;

    static Map* sharedDbMap;
};

inline const std::string& 
SharedDb::dbName() const
{
    return _key.dbName;
}

inline bool 
SharedDb::MapKey::operator<(const MapKey& rhs) const
{
    return (communicator < rhs.communicator) ||
	((communicator == rhs.communicator) && (dbName < rhs.dbName)) ||
	((communicator == rhs.communicator) && (dbName == rhs.dbName) && (envName < rhs.envName));
}
    
}
#endif
