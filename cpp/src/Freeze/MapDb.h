// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_MAP_DB_H
#define FREEZE_MAP_DB_H

#include <IceUtil/Config.h>
#include <db_cxx.h>
#include <Freeze/ConnectionI.h>
#include <Freeze/Map.h>


namespace Freeze
{

class SharedDbEnv;
class MapDb;

class MapIndexI
{
public:
    
    MapIndexI(const ConnectionIPtr&, MapDb&, DbTxn*, bool, const MapIndexBasePtr&);

    ~MapIndexI();
   
    IteratorHelper* begin(bool, const MapHelperI&) const;
    IteratorHelper* untypedFind(const Key&, bool, const MapHelperI&, bool) const;
    IteratorHelper* untypedLowerBound(const Key&, bool, const MapHelperI&) const;
    IteratorHelper* untypedUpperBound(const Key&, bool, const MapHelperI&) const;

    int untypedCount(const Key&, const ConnectionIPtr&) const;
    
    int secondaryKeyCreate(Db*, const Dbt*, const Dbt*, Dbt*);

    const std::string& name() const
    {
        return _index->name();
    }
    
    Db* db() const
    {
        return _db.get();
    }

    const MapIndexBasePtr& getKeyCompare() const
    {
        return _index;
    }

private:

    const MapIndexBasePtr _index;
    IceUtil::UniquePtr<Db> _db;
    std::string _dbName;
};


//
// A MapDb represents the Db object underneath Freeze Maps. Several Freeze Maps often
// share the very same MapDb object; SharedDbEnv manages these shared MapDb objects.
//

class MapDb : public ::Db
{
public:
    
    MapDb(const ConnectionIPtr&, const std::string&, const std::string&, const std::string&,
          const KeyCompareBasePtr&, const std::vector<MapIndexBasePtr>&, bool);
    

    //
    // The constructor for catalogs
    //
    MapDb(const Ice::CommunicatorPtr&, const Ice::EncodingVersion&, const std::string&, const std::string&, 
          const std::string&, DbEnv*);
    
    ~MapDb();

    void connectIndices(const std::vector<MapIndexBasePtr>&) const;
    void clearIndices();

    void checkTypes(const std::string&, const std::string&) const;

    const std::string& dbName() const;

    const KeyCompareBasePtr& getKeyCompare() const;

    typedef std::map<std::string, MapIndexI*> IndexMap;

private:

    const Ice::CommunicatorPtr _communicator;
    const Ice::EncodingVersion _encoding;
    const std::string _dbName;
    std::string _key;
    std::string _value;
    const int _trace;

    KeyCompareBasePtr _keyCompare;
    IndexMap _indices;
};

inline const std::string& 
MapDb::dbName() const
{
    return _dbName;
}

inline const Freeze::KeyCompareBasePtr&
MapDb::getKeyCompare() const
{
    return _keyCompare;
}

}
#endif
