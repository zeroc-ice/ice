// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SHARED_DB_H
#define FREEZE_SHARED_DB_H

#include <IceUtil/Config.h>
#include <db_cxx.h>
#include <Freeze/ConnectionI.h>
#include <Freeze/Map.h>
#include <IceUtil/Handle.h>
#include <map>

namespace Freeze
{

class SharedDb;
typedef IceUtil::Handle<SharedDb> SharedDbPtr;

class MapIndexI
{
public:
    
    MapIndexI(const ConnectionIPtr&, SharedDb&,
	      DbTxn*, bool, const MapIndexBasePtr&);

    ~MapIndexI();
   
    IteratorHelper* untypedFind(const Key&, bool, const MapHelperI&) const;
    int untypedCount(const Key&, const ConnectionIPtr&) const;
    
    int
    secondaryKeyCreate(Db*, const Dbt*, const Dbt*, Dbt*);

    const std::string name() const
    {
	return _index->name();
    }
    
    Db* db() const
    {
	return _db.get();
    }

private:

    const MapIndexBasePtr _index;
    std::auto_ptr<Db> _db;
    std::string _dbName;
};


class SharedDb : public ::Db
{
public:
    
    using Db::get;

    static SharedDbPtr get(const ConnectionIPtr&, const std::string&, 
			   const std::vector<MapIndexBasePtr>&, bool);

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

    typedef std::map<std::string, MapIndexI*> IndexMap;

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
   
    SharedDb(const MapKey&, const ConnectionIPtr&, 
	     const std::vector<MapIndexBasePtr>&, bool);
    
    void connectIndices(const std::vector<MapIndexBasePtr>&) const;
    void cleanup(bool);

    MapKey _key;
    int _refCount;
    Ice::Int _trace;

    IndexMap _indices;

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
