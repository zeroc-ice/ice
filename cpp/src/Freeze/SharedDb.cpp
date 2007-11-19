// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/SharedDb.h>
#include <IceUtil/StaticMutex.h>
#include <Freeze/Exception.h>
#include <Freeze/Util.h>
#include <Freeze/Catalog.h>
#include <Freeze/CatalogIndexList.h>
#include <algorithm>

using namespace std;
using namespace IceUtil;
using namespace Ice;
using namespace Freeze;

namespace
{

StaticMutex _mapMutex = ICE_STATIC_MUTEX_INITIALIZER;
StaticMutex _refCountMutex = ICE_STATIC_MUTEX_INITIALIZER;  

const string _catalogName = "__catalog";
const string _catalogIndexListName = "__catalogIndexList";

inline void
checkTypes(const SharedDb& sharedDb, const string& key, const string& value)
{
    if(key != sharedDb.key())
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = sharedDb.dbName() + "'s key type is " + sharedDb.key() + ", not " + key;
        throw ex;
    }
    if(value != sharedDb.value())
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = sharedDb.dbName() + "'s value type is " + sharedDb.value() + ", not " + value;
        throw ex;
    }
}
}

extern "C" 
{
static int customCompare(DB* db, const DBT* dbt1, const DBT* dbt2)
{
    SharedDb* me = static_cast<SharedDb*>(db->app_private);
    Ice::Byte* first = static_cast<Ice::Byte*>(dbt1->data);
    Key k1(first, first + dbt1->size);
    first = static_cast<Ice::Byte*>(dbt2->data);
    Key k2(first, first + dbt2->size);

    return me->getKeyCompare()->compare(k1, k2);
}
}

Freeze::SharedDb::SharedDbMap* Freeze::SharedDb::sharedDbMap = 0;

const string&
Freeze::catalogName()
{
    return _catalogName;
}

const string&
Freeze::catalogIndexListName()
{
    return _catalogIndexListName;
}

SharedDbPtr 
Freeze::SharedDb::get(const ConnectionIPtr& connection, 
                      const string& dbName,
                      const string& key,
                      const string& value,
                      const KeyCompareBasePtr& keyCompare,
                      const vector<MapIndexBasePtr>& indices,
                      bool createDb)
{
    if(dbName == _catalogName)
    {
        //
        // We don't want to lock the _mapMutex to retrieve the catalog
        //
        
        SharedDbPtr result = connection->dbEnv()->getCatalog();
        checkTypes(*result, key, value);
        return result;
    }
    else if(dbName == _catalogIndexListName)
    {
        SharedDbPtr result = connection->dbEnv()->getCatalogIndexList();
        checkTypes(*result, key, value);
        return result;
    }
    
    StaticMutex::Lock lock(_mapMutex);

    if(sharedDbMap == 0)
    {
        sharedDbMap = new SharedDbMap;
    }

    MapKey mapKey;
    mapKey.envName = connection->envName();
    mapKey.communicator = connection->communicator();
    mapKey.dbName = dbName;

    {
        SharedDbMap::iterator p = sharedDbMap->find(mapKey);
        if(p != sharedDbMap->end())
        {
            checkTypes(*(p->second), key, value);
            p->second->connectIndices(indices);
            return p->second;
        }
    }

    //
    // MapKey not found, let's create and open a new Db
    //

    //
    // Since we're going to put this SharedDb in the map no matter
    // what, we use our own transaction and connection to do so
    //
    
    ConnectionIPtr insertConnection = new ConnectionI(SharedDbEnv::get(mapKey.communicator, mapKey.envName));
    

    auto_ptr<SharedDb> result(new SharedDb(mapKey, key, value, insertConnection, 
                                           keyCompare, indices, createDb));
    
    //
    // Insert it into the map
    //
    pair<SharedDbMap::iterator, bool> insertResult;
    insertResult= sharedDbMap->insert(SharedDbMap::value_type(mapKey, result.get()));
    assert(insertResult.second);
    result->_inMap = true;
    
    return result.release();
}

SharedDbPtr 
Freeze::SharedDb::create(const ConnectionIPtr& connection, 
                      const string& dbName,
                      const string& key,
                      const string& value,
                      const KeyCompareBasePtr& keyCompare,
                      const vector<MapIndexBasePtr>& indices)
{
    MapKey mapKey;
    mapKey.envName = connection->envName();
    mapKey.communicator = connection->communicator();
    mapKey.dbName = dbName;

    return new SharedDb(mapKey, key, value, connection, keyCompare, indices, true);
}

void
Freeze::SharedDb::openCatalogs(SharedDbEnv& dbEnv, SharedDbPtr& catalog, SharedDbPtr& catalogIndexList)
{
    StaticMutex::Lock lock(_mapMutex);

    if(sharedDbMap == 0)
    {
        sharedDbMap = new SharedDbMap;
    }

    MapKey mapKey;
    mapKey.envName = dbEnv.getEnvName();
    mapKey.communicator = dbEnv.getCommunicator();
    mapKey.dbName = _catalogName;


    auto_ptr<SharedDb> newCatalog(new SharedDb(mapKey, CatalogKeyCodec::typeId(),
                                               CatalogValueCodec::typeId(), dbEnv.getEnv()));

    //
    // Insert it into the map
    //
    pair<SharedDbMap::iterator, bool> insertResult 
        = sharedDbMap->insert(SharedDbMap::value_type(mapKey, newCatalog.get()));
    
    if(!insertResult.second)
    {
        //
        // That's very wrong
        //
        assert(0);
        throw DatabaseException(__FILE__, __LINE__, "Catalog already opened");
    }
    newCatalog->_inMap = true;

    mapKey.dbName = _catalogIndexListName;

    auto_ptr<SharedDb> newCatalogIndexList(new SharedDb(mapKey, CatalogIndexListKeyCodec::typeId(),
                                                        CatalogIndexListValueCodec::typeId(), dbEnv.getEnv()));

    insertResult 
        = sharedDbMap->insert(SharedDbMap::value_type(mapKey, newCatalogIndexList.get()));
    
    if(!insertResult.second)
    {
        assert(0);
        throw DatabaseException(__FILE__, __LINE__, "CatalogIndexList already opened");
    }
    newCatalogIndexList->_inMap = true;
    

    catalog = newCatalog.release();
    catalogIndexList = newCatalogIndexList.release();
}


Freeze::SharedDb::~SharedDb()
{
    if(_trace >= 1)
    {
        Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
        out << "closing Db \"" << _mapKey.dbName << "\"";
    }

    cleanup(false);
}

void Freeze::SharedDb::__incRef()
{
    if(_trace >= 2)
    {
        Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
        out << "incrementing reference count for Db \"" << _mapKey.dbName << "\"";
    }

    IceUtil::StaticMutex::Lock lock(_refCountMutex);
    _refCount++;
}

void Freeze::SharedDb::__decRef()
{
    if(_trace >= 2)
    {
        Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
        out << "removing reference count for Db \"" << _mapKey.dbName << "\"";
    }

    IceUtil::StaticMutex::Lock lock(_refCountMutex);
    if(--_refCount == 0)
    {
        IceUtil::StaticMutex::TryLock mapLock(_mapMutex);
        if(!mapLock.acquired())
        {
            MapKey mapKey = _mapKey;

            //
            // Reacquire mutex in proper order
            //
            lock.release();
            mapLock.acquire();
            lock.acquire();

            if(_inMap)
            {
                //
                // Now, maybe another thread has deleted 'this'; let's check
                // we're still in the map
                //
                if(sharedDbMap == 0)
                {
                    return;
                }
                
                SharedDbMap::iterator p = sharedDbMap->find(mapKey);
                
                if(p == sharedDbMap->end() || p->second != this)
                {
                    //
                    // 'this' has been deleted by another thread
                    //
                    return;
                }
            }
                
            if(_refCount > 0)
            {
                return;
            }
        }

        if(_inMap)
        {
            //
            // Remove from map
            //

            size_t one;
            one = sharedDbMap->erase(_mapKey);
            assert(one == 1);

            if(sharedDbMap->size() == 0)
            {
                delete sharedDbMap;
                sharedDbMap = 0;
            }
        }

        //
        // Keep lock to prevent somebody else to re-open this Db
        // before it's closed.
        //
        delete this;
    }
}

  
Freeze::SharedDb::SharedDb(const MapKey& mapKey, 
                           const string& key,
                           const string& value,
                           const ConnectionIPtr& connection, 
                           const KeyCompareBasePtr& keyCompare,
                           const vector<MapIndexBasePtr>& indices,
                           bool createDb) :
    Db(connection->dbEnv()->getEnv(), 0),
    _mapKey(mapKey),
    _refCount(0),
    _trace(connection->trace()),
    _keyCompare(keyCompare),
    _inMap(false)
{
    if(_trace >= 1)
    {
        Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
        out << "opening Db \"" << _mapKey.dbName << "\"";
    }

    Catalog catalog(connection, _catalogName);
   
    TransactionPtr tx = connection->currentTransaction();
    bool ownTx = (tx == 0);

    for(;;)
    {
        try
        {
            if(ownTx)
            {
                tx = 0;
                tx = connection->beginTransaction();
            }

            Catalog::iterator ci = catalog.find(_mapKey.dbName);
        
            if(ci != catalog.end())
            {
                if(ci->second.evictor)
                {
                    throw DatabaseException(__FILE__, __LINE__, _mapKey.dbName + " is an evictor database");
                }
                
                _key = ci->second.key;
                _value = ci->second.value;
                checkTypes(*this, key, value);
            }
            else
            {
                _key = key;
                _value = value;
            }
    
            set_app_private(this);
            if(_keyCompare->compareEnabled())
            {
                set_bt_compare(&customCompare);
            }
            
            Ice::PropertiesPtr properties = _mapKey.communicator->getProperties();
            string propPrefix = "Freeze.Map." + _mapKey.dbName + ".";
            
            int btreeMinKey = properties->getPropertyAsInt(propPrefix + "BtreeMinKey");
            if(btreeMinKey > 2)
            {
                if(_trace >= 1)
                {
                    Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                    out << "Setting \"" << _mapKey.dbName << "\"'s btree minkey to " << btreeMinKey;
                }
                set_bt_minkey(btreeMinKey);
            }
            
            bool checksum = properties->getPropertyAsInt(propPrefix + "Checksum") > 0;
            if(checksum)
            {
                if(_trace >= 1)
                {
                    Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                    out << "Turning checksum on for \"" << _mapKey.dbName << "\"";
                }
                
                set_flags(DB_CHKSUM);
            }
            
            int pageSize = properties->getPropertyAsInt(propPrefix + "PageSize");
            if(pageSize > 0)
            {
                if(_trace >= 1)
                {
                    Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                    out << "Setting \"" << _mapKey.dbName << "\"'s pagesize to " << pageSize;
                }
                set_pagesize(pageSize);
            }
            

            DbTxn* txn = getTxn(tx);
            
            u_int32_t flags = DB_THREAD;
            if(createDb)
            {
                flags |= DB_CREATE;
            }
            open(txn, _mapKey.dbName.c_str(), 0, DB_BTREE, flags, FREEZE_DB_MODE);
            
            
            Ice::StringSeq oldIndices;
            Ice::StringSeq newIndices;
            size_t oldSize = 0;
            CatalogIndexList catalogIndexList(connection, _catalogIndexListName);
            
            if(createDb)
            {
                CatalogIndexList::iterator cil = catalogIndexList.find(_mapKey.dbName);
                if(cil != catalogIndexList.end())
                {
                    oldIndices = cil->second;
                    oldSize = oldIndices.size();
                }
            } 
            
            for(vector<MapIndexBasePtr>::const_iterator p = indices.begin();
                p != indices.end(); ++p)
            {
                const MapIndexBasePtr& indexBase = *p; 
                assert(indexBase->_impl == 0);
                assert(indexBase->_communicator == 0);
                indexBase->_communicator = connection->communicator();
                
                auto_ptr<MapIndexI> indexI;

                try
                {
                    indexI.reset(new MapIndexI(connection, *this, txn, createDb, indexBase));
                }
                catch(const DbDeadlockException&)
                {
                    throw;
                }
                catch(const DbException& dx)
                {
                    string message = "Error while opening index \"" + _mapKey.dbName +
                        "." + indexBase->name() + "\": " + dx.what();

                    throw DatabaseException(__FILE__, __LINE__, message);
                }
                
#ifndef NDEBUG
                bool inserted = 
#endif
                    _indices.insert(IndexMap::value_type(indexBase->name(), indexI.get())).second;
                assert(inserted);
                
                indexBase->_impl = indexI.release();
                
                if(createDb)
                {
                    newIndices.push_back(indexBase->name());
                    oldIndices.erase(std::remove(oldIndices.begin(), oldIndices.end(), indexBase->name()), oldIndices.end());
                }
            }
            
            if(ci == catalog.end())
            {
                CatalogData catalogData;
                catalogData.evictor = false;
                catalogData.key = key;
                catalogData.value = value;
                catalog.put(Catalog::value_type(_mapKey.dbName, catalogData));
            }
            
            if(createDb)
            {
                //
                // Remove old indices and write the new ones
                //
                bool indexRemoved = false;

                for(Ice::StringSeq::const_iterator q = oldIndices.begin(); q != oldIndices.end(); ++q)
                {
                    const string& index = *q;
                    
                    if(_trace >= 1)
                    {
                        Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                        out << "removing old index \"" << index << "\" on Db \"" << _mapKey.dbName << "\"";
                    }
                    
                    try
                    {
                        connection->removeMapIndex(_mapKey.dbName, *q);
                        indexRemoved = true;
                    }
                    catch(const IndexNotFoundException&)
                    {
                        // Ignored
                        
                        if(_trace >= 1)
                        {
                            Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                            out << "index \"" << index << "\" on Db \"" << _mapKey.dbName << "\" does not exist";
                        }
                    }
                }
                
                if(indexRemoved || oldSize != newIndices.size())
                {
                    if(newIndices.size() == 0)
                    {
                        catalogIndexList.erase(_mapKey.dbName);
                        if(_trace >= 1)
                        {
                            Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                            out << "Removed catalogIndexList entry for Db \"" << _mapKey.dbName << "\"";
                        }
                        
                    }
                    else
                    {
                        catalogIndexList.put(CatalogIndexList::value_type(_mapKey.dbName, newIndices));
                        if(_trace >= 1)
                        {
                            Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                            out << "Updated catalogIndexList entry for Db \"" << _mapKey.dbName << "\"";
                        }
                    }
                }
            }

            if(ownTx)
            {
                tx->commit();
            }
            break; // for(;;)
        }
        catch(const DbDeadlockException& dx)
        {
            if(ownTx)
            {
                if(connection->deadlockWarning())
                {
                    Warning out(connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::SharedDb::SharedDb on Map \"" 
                        << _mapKey.dbName << "\"; retrying ...";
                }

                //
                // Ignored, try again
                //
            }
            else
            {
                throw DeadlockException(__FILE__, __LINE__, dx.what(), tx);
            }
        }
        catch(const DbException& dx)
        {
            if(ownTx)
            {
                try
                {
                    tx->rollback();
                }
                catch(...)
                {
                }
            }
                
            string message = "Error while opening Db \"" + _mapKey.dbName +
                "\": " + dx.what();

            throw DatabaseException(__FILE__, __LINE__, message);
        }
        catch(...)
        {
            if(ownTx && tx != 0)
            {   
                try
                {
                    tx->rollback();
                }
                catch(...)
                {
                }
            }
            throw;
        }
    }
}

Freeze::SharedDb::SharedDb(const MapKey& mapKey, const string& keyTypeId, const string& valueTypeId, DbEnv* env) :
    Db(env, 0),
    _mapKey(mapKey),
    _key(keyTypeId),
    _value(valueTypeId),
    _refCount(0),
    _inMap(false)
{
    _trace = _mapKey.communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Map");

    if(_trace >= 1)
    {
        Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
        out << "opening Db \"" << _mapKey.dbName << "\"";
    }

    try
    {
        Ice::PropertiesPtr properties = _mapKey.communicator->getProperties();
        string propPrefix = "Freeze.Map." + _mapKey.dbName + ".";
        
        int btreeMinKey = properties->getPropertyAsInt(propPrefix + "BtreeMinKey");
        if(btreeMinKey > 2)
        {
            if(_trace >= 1)
            {
                Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                out << "Setting \"" << _mapKey.dbName << "\"'s btree minkey to " << btreeMinKey;
            }
            set_bt_minkey(btreeMinKey);
        }
        
        bool checksum = properties->getPropertyAsInt(propPrefix + "Checksum") > 0;
        if(checksum)
        {
            if(_trace >= 1)
            {
                Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                out << "Turning checksum on for \"" << _mapKey.dbName << "\"";
            }

            set_flags(DB_CHKSUM);
        }
        
        int pageSize = properties->getPropertyAsInt(propPrefix + "PageSize");
        if(pageSize > 0)
        {
            if(_trace >= 1)
            {
                Trace out(_mapKey.communicator->getLogger(), "Freeze.Map");
                out << "Setting \"" << _mapKey.dbName << "\"'s pagesize to " << pageSize;
            }
            set_pagesize(pageSize);
        }

        u_int32_t flags = DB_THREAD | DB_CREATE | DB_AUTO_COMMIT;
        open(0, _mapKey.dbName.c_str(), 0, DB_BTREE, flags, FREEZE_DB_MODE);
    }
    catch(const ::DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
}

void
Freeze::SharedDb::connectIndices(const vector<MapIndexBasePtr>& indices) const
{
    for(vector<MapIndexBasePtr>::const_iterator p = indices.begin();
        p != indices.end(); ++p)
    {
        const MapIndexBasePtr& indexBase = *p; 
        assert(indexBase->_impl == 0);

        IndexMap::const_iterator q = _indices.find(indexBase->name());
        assert(q != _indices.end());
        indexBase->_impl = q->second;
        indexBase->_communicator = _mapKey.communicator;
    }
}

void
Freeze::SharedDb::cleanup(bool noThrow)
{
    try
    {
        for(IndexMap::iterator p = _indices.begin(); p != _indices.end(); ++p)
        {
            delete p->second;
        }
        _indices.clear();
        
        close(0);
    }
    catch(const ::DbException& dx)
    {
        if(!noThrow)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }
}
