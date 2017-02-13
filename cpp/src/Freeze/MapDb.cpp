// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/MapDb.h>
#include <Freeze/Exception.h>
#include <Freeze/Util.h>
#include <Freeze/Catalog.h>
#include <Freeze/CatalogIndexList.h>
#include <algorithm>

#include <IceUtil/StringConverter.h>

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace Freeze;

namespace
{

const string _catalogName = "__catalog";
const string _catalogIndexListName = "__catalogIndexList";

}

extern "C" 
{
#if (DB_VERSION_MAJOR <= 5)
    static int customCompare(DB* db, const DBT* dbt1, const DBT* dbt2)
#else
    static int customCompare(DB* db, const DBT* dbt1, const DBT* dbt2, size_t*)
#endif
    {
	MapDb* me = static_cast<MapDb*>(db->app_private);
	Ice::Byte* first = static_cast<Ice::Byte*>(dbt1->data);
	Key k1(first, first + dbt1->size);
	first = static_cast<Ice::Byte*>(dbt2->data);
	Key k2(first, first + dbt2->size);
	
	return me->getKeyCompare()->compare(k1, k2);
    }
}

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

Freeze::MapDb::~MapDb()
{
    if(_trace >= 1)
    {
        Trace out(_communicator->getLogger(), "Freeze.Map");
        out << "closing Db \"" << _dbName << "\"";
    }

    clearIndices();

    if(get_DB() != 0)
    {
        try
        {
            close(0);
        }
        catch(const ::DbException& dx)
        {
            Ice::Error error(_communicator->getLogger());
            error << "Freeze.Map: closing Db " << _dbName << " raised DbException: " << dx.what();
        }
    }
}
  
Freeze::MapDb::MapDb(const ConnectionIPtr& connection, 
                     const string& dbName,
                     const string& key,
                     const string& value,
                     const KeyCompareBasePtr& keyCompare,
                     const vector<MapIndexBasePtr>& indices,
                     bool createDb) :
    Db(connection->dbEnv()->getEnv(), 0),
    _communicator(connection->communicator()),
    _encoding(connection->encoding()),
    _dbName(dbName),
    _trace(connection->trace()),
    _keyCompare(keyCompare)
{
    if(_trace >= 1)
    {
        Trace out(_communicator->getLogger(), "Freeze.Map");
        out << "opening Db \"" << _dbName << "\"";
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

            Catalog::iterator ci = catalog.find(_dbName);
        
            if(ci != catalog.end())
            {
                if(ci->second.evictor)
                {
                    throw DatabaseException(__FILE__, __LINE__, _dbName + " is an evictor database");
                }
                
                _key = ci->second.key;
                _value = ci->second.value;
                checkTypes(key, value);
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
            
            PropertiesPtr properties = _communicator->getProperties();
            string propPrefix = "Freeze.Map." + _dbName + ".";
            
            int btreeMinKey = properties->getPropertyAsInt(propPrefix + "BtreeMinKey");
            if(btreeMinKey > 2)
            {
                if(_trace >= 1)
                {
                    Trace out(_communicator->getLogger(), "Freeze.Map");
                    out << "Setting \"" << _dbName << "\"'s btree minkey to " << btreeMinKey;
                }
                set_bt_minkey(btreeMinKey);
            }
            
            bool checksum = properties->getPropertyAsInt(propPrefix + "Checksum") > 0;
            if(checksum)
            {
                if(_trace >= 1)
                {
                    Trace out(_communicator->getLogger(), "Freeze.Map");
                    out << "Turning checksum on for \"" << _dbName << "\"";
                }
                
                set_flags(DB_CHKSUM);
            }
            
            int pageSize = properties->getPropertyAsInt(propPrefix + "PageSize");
            if(pageSize > 0)
            {
                if(_trace >= 1)
                {
                    Trace out(_communicator->getLogger(), "Freeze.Map");
                    out << "Setting \"" << _dbName << "\"'s pagesize to " << pageSize;
                }
                set_pagesize(pageSize);
            }
            

            DbTxn* txn = getTxn(tx);
            
            u_int32_t flags = DB_THREAD;
            if(createDb)
            {
                flags |= DB_CREATE;
            }

            //
            // Berkeley DB expects file paths to be UTF8 encoded.
            //
            open(txn, nativeToUTF8(_dbName, getProcessStringConverter()).c_str(), 0, DB_BTREE,
                 flags, FREEZE_DB_MODE);
            
            StringSeq oldIndices;
            StringSeq newIndices;
            size_t oldSize = 0;
            CatalogIndexList catalogIndexList(connection, _catalogIndexListName);
            
            if(createDb)
            {
                CatalogIndexList::iterator cil = catalogIndexList.find(_dbName);
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
                indexBase->_encoding = connection->encoding();
                
                IceUtil::UniquePtr<MapIndexI> indexI;

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
                    string message = "Error while opening index \"" + _dbName +
                        "." + indexBase->name() + "\": " + dx.what();

                    throw DatabaseException(__FILE__, __LINE__, message);
                }
                
#ifdef NDEBUG
                _indices.insert(IndexMap::value_type(indexBase->name(), indexI.get()));
#else
                bool inserted = 
                    _indices.insert(IndexMap::value_type(indexBase->name(), indexI.get())).second;
                assert(inserted);
#endif
                
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
                catalog.put(Catalog::value_type(_dbName, catalogData));
            }
            
            if(createDb)
            {
                //
                // Remove old indices and write the new ones
                //
                bool indexRemoved = false;

                for(StringSeq::const_iterator q = oldIndices.begin(); q != oldIndices.end(); ++q)
                {
                    const string& index = *q;
                    
                    if(_trace >= 1)
                    {
                        Trace out(_communicator->getLogger(), "Freeze.Map");
                        out << "removing old index \"" << index << "\" on Db \"" << _dbName << "\"";
                    }
                    
                    try
                    {
                        connection->removeMapIndex(_dbName, *q);
                        indexRemoved = true;
                    }
                    catch(const IndexNotFoundException&)
                    {
                        // Ignored
                        
                        if(_trace >= 1)
                        {
                            Trace out(_communicator->getLogger(), "Freeze.Map");
                            out << "index \"" << index << "\" on Db \"" << _dbName << "\" does not exist";
                        }
                    }
                }
                
                if(indexRemoved || oldSize != newIndices.size())
                {
                    if(newIndices.size() == 0)
                    {
                        catalogIndexList.erase(_dbName);
                        if(_trace >= 1)
                        {
                            Trace out(_communicator->getLogger(), "Freeze.Map");
                            out << "Removed catalogIndexList entry for Db \"" << _dbName << "\"";
                        }
                        
                    }
                    else
                    {
                        catalogIndexList.put(CatalogIndexList::value_type(_dbName, newIndices));
                        if(_trace >= 1)
                        {
                            Trace out(_communicator->getLogger(), "Freeze.Map");
                            out << "Updated catalogIndexList entry for Db \"" << _dbName << "\"";
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
                    out << "Deadlock in Freeze::MapDb::MapDb on Map \"" 
                        << _dbName << "\"; retrying ...";
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
                
            string message = "Error while opening Db \"" + _dbName +
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

Freeze::MapDb::MapDb(const Ice::CommunicatorPtr& communicator, 
                     const Ice::EncodingVersion& encoding,
                     const string& dbName, 
                     const string& keyTypeId, 
                     const string& valueTypeId, 
                     DbEnv* env) :
    Db(env, 0),
    _communicator(communicator),
    _encoding(encoding),
    _dbName(dbName),
    _key(keyTypeId),
    _value(valueTypeId),
    _trace(communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Map"))
{
    if(_trace >= 1)
    {
        Trace out(_communicator->getLogger(), "Freeze.Map");
        out << "opening Db \"" << _dbName << "\"";
    }

    try
    {
        PropertiesPtr properties = _communicator->getProperties();
        string propPrefix = "Freeze.Map." + _dbName + ".";
        
        int btreeMinKey = properties->getPropertyAsInt(propPrefix + "BtreeMinKey");
        if(btreeMinKey > 2)
        {
            if(_trace >= 1)
            {
                Trace out(_communicator->getLogger(), "Freeze.Map");
                out << "Setting \"" << _dbName << "\"'s btree minkey to " << btreeMinKey;
            }
            set_bt_minkey(btreeMinKey);
        }
        
        bool checksum = properties->getPropertyAsInt(propPrefix + "Checksum") > 0;
        if(checksum)
        {
            if(_trace >= 1)
            {
                Trace out(_communicator->getLogger(), "Freeze.Map");
                out << "Turning checksum on for \"" << _dbName << "\"";
            }

            set_flags(DB_CHKSUM);
        }
        
        int pageSize = properties->getPropertyAsInt(propPrefix + "PageSize");
        if(pageSize > 0)
        {
            if(_trace >= 1)
            {
                Trace out(_communicator->getLogger(), "Freeze.Map");
                out << "Setting \"" << _dbName << "\"'s pagesize to " << pageSize;
            }
            set_pagesize(pageSize);
        }

        u_int32_t flags = DB_THREAD | DB_CREATE | DB_AUTO_COMMIT;

        //
        // Berkeley DB expects file paths to be UTF8 encoded.
        //
        open(0, nativeToUTF8(_dbName, getProcessStringConverter()).c_str(), 0, DB_BTREE, flags,
             FREEZE_DB_MODE);
    }
    catch(const ::DbException& dx)
    {
        throw DatabaseException(__FILE__, __LINE__, dx.what());
    }
}

void
Freeze::MapDb::connectIndices(const vector<MapIndexBasePtr>& indices) const
{
    for(vector<MapIndexBasePtr>::const_iterator p = indices.begin();
        p != indices.end(); ++p)
    {
        const MapIndexBasePtr& indexBase = *p; 
        assert(indexBase->_impl == 0);

        IndexMap::const_iterator q = _indices.find(indexBase->name());

        assert(q != _indices.end());
        indexBase->_impl = q->second;
        indexBase->_communicator = _communicator;
        indexBase->_encoding = _encoding;
    }
}

void
Freeze::MapDb::clearIndices()
{
    try
    {
        for(IndexMap::iterator p = _indices.begin(); p != _indices.end(); ++p)
        {
            delete p->second;
        }
    }
    catch(const ::DbException& dx)
    {
        throw DatabaseException(__FILE__, __LINE__, dx.what());
    }
    _indices.clear();
}

void
Freeze::MapDb::checkTypes(const string& key, const string& value) const
{
    if(key != _key)
    {
        throw DatabaseException(__FILE__, __LINE__,
                                _dbName + "'s key type is " + _key + ", not " + key);
    }
    if(value != _value)
    {
        throw DatabaseException(__FILE__, __LINE__,
                                _dbName + "'s value type is " + _value + ", not " + value);
    }
}
