// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/MapI.h>
#include <Freeze/Exception.h>
#include <Freeze/SharedDb.h>
#include <Freeze/Util.h>
#include <Freeze/Catalog.h>
#include <stdlib.h>

using namespace std;
using namespace Ice;
using namespace Freeze;


//
// MapIndexBase (from Map.h)
//

Freeze::MapIndexBase::~MapIndexBase()
{
}

Freeze::MapIndexBase::MapIndexBase(
    const string& name,
    bool enabled) :
    KeyCompareBase(enabled),
    _name(name),
    _impl(0),
    _map(0)
{
}

const string&
MapIndexBase::name() const
{
    return _name;
}

IteratorHelper*
Freeze::MapIndexBase::untypedFind(const Key& k, bool ro, bool onlyDups) const
{
    return _impl->untypedFind(k, ro, *_map, onlyDups);
}

IteratorHelper*
Freeze::MapIndexBase::untypedLowerBound(const Key& k, bool ro) const
{
    return _impl->untypedLowerBound(k, ro, *_map);
}

IteratorHelper*
Freeze::MapIndexBase::untypedUpperBound(const Key& k, bool ro) const
{
    return _impl->untypedUpperBound(k, ro, *_map);
}

int 
Freeze::MapIndexBase::untypedCount(const Key& k) const
{
    return _impl->untypedCount(k, _map->connection());
}

//
// KeyCompareBase
//
Freeze::KeyCompareBase::KeyCompareBase(bool enabled) :
    _enabled(enabled)
{}

bool
Freeze::KeyCompareBase::compareEnabled() const
{
    return _enabled;
}

//
// MapHelper (from Map.h)
//

Freeze::MapHelper*
Freeze::MapHelper::create(const Freeze::ConnectionPtr& connection, 
                          const string& dbName,
                          const string& key,
                          const string& value,
                          const Freeze::KeyCompareBasePtr& keyCompare,
                          const std::vector<MapIndexBasePtr>& indices,
                          bool createDb)
{
    Freeze::ConnectionIPtr connectionI = Freeze::ConnectionIPtr::dynamicCast(connection.get());
    return new MapHelperI(connectionI, dbName, key, value, keyCompare, indices, createDb);
}

Freeze::MapHelper::~MapHelper()
{
}


//
// IteratorHelper (from Map.h)
//

Freeze::IteratorHelper* 
Freeze::IteratorHelper::create(const MapHelper& m, bool readOnly)
{
    const MapHelperI& actualMap = dynamic_cast<const MapHelperI&>(m);

    auto_ptr<IteratorHelperI> r(new IteratorHelperI(actualMap, readOnly, 
                                                    0, false));
    if(r->next())
    {
        return r.release();
    }
    else
    {
        return 0;
    }
}


Freeze::IteratorHelper::~IteratorHelper()
{
}


//
// IteratorHelperI
//

Freeze::IteratorHelperI::IteratorHelperI(const MapHelperI& m, bool readOnly, 
                                         const MapIndexBasePtr& index,
                                         bool onlyDups) :
    _map(m),
    _dbc(0),
    _indexed(index != 0),
    _onlyDups(onlyDups),
    _tx(0)
{
    if(_map._trace >= 2)
    {
        Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
        out << "opening iterator on Db \"" << _map._dbName << "\"";
        if(index != 0)
        {
            out << " with index \"" << index->name() << "\"";
        }
    }

    DbTxn* txn = _map._connection->dbTxn();
    
    if(txn == 0 && !readOnly)
    {
        //
        // Need to start a transaction
        //
        _tx = new Tx(_map);
        txn = _tx->getTxn();
    }

    try
    {
        if(index != 0)
        {
            index->_impl->db()->cursor(txn, &_dbc, 0);
        }
        else
        {
            _map._db->cursor(txn, &_dbc, 0);
        }
    }
    catch(const ::DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }

    _map._iteratorList.push_back(this);
}


Freeze::IteratorHelperI::IteratorHelperI(const IteratorHelperI& it) :
    _map(it._map),
    _dbc(0),
    _indexed(it._indexed),
    _onlyDups(it._onlyDups),
    _tx(0)
{
    if(_map._trace >= 2)
    {
        Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
        out << "duplicating iterator on Db \"" << _map._dbName << "\"";
    }

    try
    {
        it._dbc->dup(&_dbc, DB_POSITION);
    }
    catch(const ::DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
   
    _tx = it._tx;
    _map._iteratorList.push_back(this);
}

Freeze::IteratorHelperI::~IteratorHelperI()
{
    close();
}


bool 
Freeze::IteratorHelperI::find(const Key& key) const
{
    Dbt dbKey;
    initializeInDbt(key, dbKey);
    //
    // When we have a custom-comparison function, Berkeley DB returns
    // the key on-disk (when it finds one). We disable this behavior:
    // (ref Oracle SR 5925672.992)
    //
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
    
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    for(;;)
    {
        try
        {
            if(_dbc->get(&dbKey, &dbValue, DB_SET) == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(_tx != 0)
            {
                _tx->dead();
            }

            DeadlockException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
        catch(const ::DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }
}

bool 
Freeze::IteratorHelperI::lowerBound(const Key& key) const
{
    //
    // We retrieve the actual key for upperBound
    //
    Dbt dbKey;
    _key = key;
    initializeOutDbt(_key, dbKey);
    dbKey.set_size(static_cast<u_int32_t>(_key.size()));

    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    for(;;)
    {
        try
        {
            int err = _dbc->get(&dbKey, &dbValue, DB_SET_RANGE);
            if(err == 0)
            {
                _key.resize(dbKey.get_size());
                return true;
            }
            else if(err == DB_NOTFOUND)
            {
                return false;
            }
            else
            {
                //
                // Bug in Freeze
                //
                assert(0);
                throw DatabaseException(__FILE__, __LINE__);
            }
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(_tx != 0)
            {
                _tx->dead();
            }
            
            DeadlockException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
        catch(const ::DbException& dx)
        {
            handleDbException(dx, _key, dbKey, __FILE__, __LINE__);
        }
    }
}

bool 
Freeze::IteratorHelperI::upperBound(const Key& key) const
{
    if(lowerBound(key))
    {
        if(_key == key)
        {
            return next(true);
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}


Freeze::IteratorHelper*
Freeze::IteratorHelperI::clone() const
{
    return new IteratorHelperI(*this);
}
    
void
Freeze::IteratorHelperI::get(const Key*& key, const Value*& value) const
{
    key = &_key;
    value = &_value;

    size_t keySize = _key.capacity();
    if(keySize < 1024)
    {
        keySize = 1024;
    }
    _key.resize(keySize);

    Dbt dbKey;
    initializeOutDbt(_key, dbKey);
    
    size_t valueSize = _value.capacity();
    if(valueSize < 1024)
    {
        valueSize = 1024;
    }
    _value.resize(valueSize);
    Dbt dbValue;
    initializeOutDbt(_value, dbValue);

    for(;;)
    {
        try
        {
            int err;

            if(_indexed)
            {
                //
                // Not interested in getting the index's key
                //
                Dbt iKey;
                iKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
            
                err = _dbc->pget(&iKey, &dbKey, &dbValue, DB_CURRENT);
            }
            else
            {
                err = _dbc->get(&dbKey, &dbValue, DB_CURRENT);
            }

            if(err == 0)
            {
                _key.resize(dbKey.get_size());
                _value.resize(dbValue.get_size());
                break; // for(;;)
            }
            else if(err == DB_KEYEMPTY)
            {
                throw InvalidPositionException(__FILE__, __LINE__);
            }
            else 
            {
                //
                // Bug in Freeze
                //
                assert(0);
                throw DatabaseException(__FILE__, __LINE__);
            }
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(_tx != 0)
            {
                _tx->dead();
            }

            DeadlockException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
        catch(const ::DbException& dx)
        {
            handleDbException(dx, _key, dbKey, _value, dbValue,
                              __FILE__, __LINE__);
        }
    }
}
    
const Freeze::Key*
Freeze::IteratorHelperI::get() const
{
    size_t keySize = _key.capacity();
    if(keySize < 1024)
    {
        keySize = 1024;
    }
    _key.resize(keySize);

    Dbt dbKey;
    initializeOutDbt(_key, dbKey);
    
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
    
    for(;;)
    {
        try
        {
            int err;
            if(_indexed)
            {
                //
                // Not interested in getting the index's key
                //
                Dbt iKey;
                iKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
            
                err = _dbc->pget(&iKey, &dbKey, &dbValue, DB_CURRENT);
            }
            else
            {
                err = _dbc->get(&dbKey, &dbValue, DB_CURRENT);
            }

            if(err == 0)
            {
                _key.resize(dbKey.get_size());
                return &_key;
            }
            else if(err == DB_KEYEMPTY)
            {
                throw InvalidPositionException(__FILE__, __LINE__);
            }
            else 
            {
                //
                // Bug in Freeze
                //
                assert(0);
                throw DatabaseException(__FILE__, __LINE__);
            }
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(_tx != 0)
            {
                _tx->dead();
            }

            DeadlockException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
        catch(const ::DbException& dx)
        {
            handleDbException(dx, _key, dbKey, __FILE__, __LINE__);
        }
    }
}

void 
Freeze::IteratorHelperI::set(const Value& value)
{
    if(_indexed)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = "Cannot set an iterator retrieved through an index";
        throw ex;
    }
    
    //
    // key ignored
    //
    Dbt dbKey;
    dbKey.set_flags(DB_DBT_USERMEM);

    Dbt dbValue;
    initializeInDbt(value, dbValue);

    if(_tx != 0)
    {
        _map.closeAllIteratorsExcept(_tx);
    }

    try
    {
        int err;
        err = _dbc->put(&dbKey, &dbValue, DB_CURRENT);
        assert(err == 0);
    }
    catch(const ::DbDeadlockException& dx)
    {
        if(_tx != 0)
        {
            _tx->dead();
        }

        DeadlockException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
    catch(const ::DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
}

void
Freeze::IteratorHelperI::erase()
{
    if(_tx != 0)
    {
        _map.closeAllIteratorsExcept(_tx);
    }

    try
    {
        int err = _dbc->del(0);
        if(err == DB_KEYEMPTY)
        {
            throw InvalidPositionException(__FILE__, __LINE__);
        }
        assert(err == 0);
    }
    catch(const ::DbDeadlockException& dx)
    {
        if(_tx != 0)
        {
            _tx->dead();
        }

        DeadlockException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
    catch(const ::DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
}

bool 
Freeze::IteratorHelperI::next() const
{
    return next(false);
}

bool
Freeze::IteratorHelperI::next(bool skipDups) const
{
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbKey;
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    int flags = DB_NEXT;
    if(skipDups)
    {
        flags = DB_NEXT_NODUP;
    }
    else if(_indexed && _onlyDups)
    {
        flags = DB_NEXT_DUP;
    }

    try
    {
        if(_dbc->get(&dbKey, &dbValue, flags) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    catch(const ::DbDeadlockException& dx)
    {
        if(_tx != 0)
        {
            _tx->dead();
        }

        DeadlockException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
    catch(const ::DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
}

void
Freeze::IteratorHelperI::close()
{
    if(_dbc != 0)
    {
        if(_map._trace >= 2)
        {
            Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
            out << "closing iterator on Db \"" << _map._dbName << "\"";
        }
        
        try
        {
            _dbc->close();
        }
        catch(const ::DbDeadlockException& dx)
        {
            bool raiseException = (_tx == 0);
            cleanup();
            if(raiseException)
            {
                DeadlockException ex(__FILE__, __LINE__);
                ex.message = dx.what();
                throw ex;
            }
        }
        catch(const ::DbException& dx)
        {
            cleanup();
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
        cleanup();
    }
}

void
Freeze::IteratorHelperI::cleanup()
{
    _dbc = 0;
    _map._iteratorList.remove(this);
    _tx = 0;
}


//
// IteratorHelperI::Tx
//

Freeze::IteratorHelperI::Tx::Tx(const MapHelperI& m) :
    _map(m),
    _txn(0),
    _dead(false)
{
    if(_map._trace >= 2)
    {
        Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
        out << "starting transaction for Db \"" << _map._dbName << "\"";
    }

    try
    {
        _map._connection->dbEnv()->getEnv()->txn_begin(0, &_txn, 0);
    }
    catch(const ::DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
}
        

Freeze::IteratorHelperI::Tx::~Tx()
{
    if(_dead)
    {
        if(_map._trace >= 2)
        {
            Trace out(_map._connection->communicator()->getLogger(), "");
            out << "aborting transaction for Db \"" << _map._dbName << "\"";
        }

        try
        {
            _txn->abort();
        }
        catch(...)
        {
            //
            // Ignore exceptions to avoid crash during stack unwinding
            //
        }
    }
    else
    {
        if(_map._trace >= 2)
        {
            Trace out(_map._connection->communicator()->getLogger(), "Freeze.Map");
            out << "committing transaction for Db \"" << _map._dbName.c_str() << "\"";
        }

        try
        {
            _txn->commit(0);
        }
        catch(const ::DbDeadlockException& dx)
        {
            DeadlockException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
        catch(const ::DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }
}

void
Freeze::IteratorHelperI::Tx::dead()
{
    _dead = true;
}



//
// MapHelperI
//


Freeze::MapHelperI::MapHelperI(const ConnectionIPtr& connection, 
                               const string& dbName,
                               const string& key,
                               const string& value,
                               const KeyCompareBasePtr& keyCompare,
                               const vector<MapIndexBasePtr>& indices,
                               bool createDb) :
    _connection(connection),
    _db(SharedDb::get(connection, dbName, key, value, keyCompare, indices, createDb)),
    _dbName(dbName),
    _trace(connection->trace())
{ 
    for(vector<MapIndexBasePtr>::const_iterator p = indices.begin(); 
        p != indices.end(); ++p)
    {
        const MapIndexBasePtr& indexBase = *p;
        assert(indexBase->_impl != 0);
        assert(indexBase->_communicator == _connection->communicator());
        assert(indexBase->_map == 0);
        
#ifndef NDEBUG
        bool inserted =
#endif 
            _indices.insert(IndexMap::value_type(indexBase->name(), indexBase)).second;
        assert(inserted);
        indexBase->_map = this;
    }
    
    _connection->registerMap(this);
}

Freeze::MapHelperI::~MapHelperI()
{
    close();
}

Freeze::IteratorHelper*
Freeze::MapHelperI::find(const Key& k, bool readOnly) const
{
    for(;;)
    {
        try
        {  
            auto_ptr<IteratorHelperI> r(new IteratorHelperI(*this, readOnly, 0, false));
            if(r->find(k))
            {
                return r.release();
            }
            else
            {
                return 0;
            }
        }
        catch(const DeadlockException&)
        {
            if(_connection->dbTxn() != 0)
            {
                throw;
            }
            else
            {
                if(_connection->deadlockWarning())
                {
                    Warning out(_connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapHelperI::find on Map \"" 
                        << _dbName << "\"; retrying ...";
                }

                //
                // Ignored, try again
                //
            }
        }
    }
}

Freeze::IteratorHelper*
Freeze::MapHelperI::lowerBound(const Key& k, bool readOnly) const
{
    for(;;)
    {
        try
        {  
            auto_ptr<IteratorHelperI> r(new IteratorHelperI(*this, readOnly, 0, false));
            if(r->lowerBound(k))
            {
                return r.release();
            }
            else
            {
                return 0;
            }
        }
        catch(const DeadlockException&)
        {
            if(_connection->dbTxn() != 0)
            {
                throw;
            }
            else
            {
                if(_connection->deadlockWarning())
                {
                    Warning out(_connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapHelperI::lowerBound on Map \"" 
                        << _dbName << "\"; retrying ...";
                }

                //
                // Ignored, try again
                //
            }
        }
    }
}

Freeze::IteratorHelper*
Freeze::MapHelperI::upperBound(const Key& k, bool readOnly) const
{
    for(;;)
    {
        try
        {  
            auto_ptr<IteratorHelperI> r(new IteratorHelperI(*this, readOnly, 0, false));
            if(r->upperBound(k))
            {
                return r.release();
            }
            else
            {
                return 0;
            }
        }
        catch(const DeadlockException&)
        {
            if(_connection->dbTxn() != 0)
            {
                throw;
            }
            else
            {
                if(_connection->deadlockWarning())
                {
                    Warning out(_connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapHelperI::upperBound on Map \"" 
                        << _dbName << "\"; retrying ...";
                }

                //
                // Ignored, try again
                //
            }
        }
    }
}

void
Freeze::MapHelperI::put(const Key& key, const Value& value)
{
    Dbt dbKey;
    Dbt dbValue;
    initializeInDbt(key, dbKey);
    initializeInDbt(value, dbValue);
 
    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
        closeAllIterators();
    }

    for(;;)
    {
        try
        {
            int err = _db->put(txn, &dbKey, &dbValue, 
                               txn != 0 ? 0 : DB_AUTO_COMMIT);
            
            if(err == 0)
            {
                break;
            }
            else
            {
                //
                // Bug in Freeze
                //
                throw DatabaseException(__FILE__, __LINE__);
            }
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(txn != 0)
            {
                DeadlockException ex(__FILE__, __LINE__);
                ex.message = dx.what();
                throw ex;
            }
            else
            {
                if(_connection->deadlockWarning())
                {
                    Warning out(_connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapHelperI::put on Map \"" 
                        << _dbName << "\"; retrying ...";
                }

                //
                // Ignored, try again
                //
            }
        }
        catch(const ::DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }
}

size_t
Freeze::MapHelperI::erase(const Key& key)
{
    Dbt dbKey;
    initializeInDbt(key, dbKey);

    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
        closeAllIterators();
    }

    for(;;)
    {
        try
        {
            int err = _db->del(txn, &dbKey, txn != 0 ? 0 : DB_AUTO_COMMIT);

            if(err == 0)
            {
                return true;
            }
            else if(err == DB_NOTFOUND)
            {
                return false;
            }
            else
            {
                assert(0);
                throw DatabaseException(__FILE__, __LINE__);
            }
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(txn != 0)
            {
                DeadlockException ex(__FILE__, __LINE__);
                ex.message = dx.what();
                throw ex;
            }
            else
            {
                if(_connection->deadlockWarning())
                {
                    Warning out(_connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapHelperI::erase on Map \"" 
                        << _dbName << "\"; retrying ...";
                }

                //
                // Ignored, try again
                //
            }
        }
        catch(const ::DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }
}

size_t
Freeze::MapHelperI::count(const Key& key) const
{
    Dbt dbKey;
    initializeInDbt(key, dbKey);
    
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
    
    for(;;)
    {
        try
        {
            int err = _db->get(_connection->dbTxn(), &dbKey, &dbValue, 0);
            
            if(err == 0)
            {
                return 1;
            }
            else if(err == DB_NOTFOUND)
            {
                return 0;
            }
            else
            {
                assert(0);
                throw DatabaseException(__FILE__, __LINE__);
            }
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(_connection->dbTxn() != 0)
            {
                DeadlockException ex(__FILE__, __LINE__);
                ex.message = dx.what();
                throw ex;
            }
            else
            {
                if(_connection->deadlockWarning())
                {
                    Warning out(_connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapHelperI::count on Map \"" 
                        << _dbName << "\"; retrying ...";
                }

                //
                // Ignored, try again
                //
            }
        }
        catch(const ::DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }
}
    
void
Freeze::MapHelperI::clear()
{
    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
        closeAllIterators();
    }

    for(;;)
    {
        try
        {
            u_int32_t count;
            int err;
            err = _db->truncate(txn, &count, txn != 0 ? 0 : DB_AUTO_COMMIT);
            assert(err == 0);
            break;
        }
        catch(const ::DbDeadlockException& dx)
        {
            if(txn != 0)
            {
                DeadlockException ex(__FILE__, __LINE__);
                ex.message = dx.what();
                throw ex;
            }
            else
            {
                if(_connection->deadlockWarning())
                {
                    Warning out(_connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapHelperI::clear on Map \"" 
                        << _dbName << "\"; retrying ...";
                }

                //
                // Ignored, try again
                //
            }
        }
        catch(const ::DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }
}

void
Freeze::MapHelperI::destroy()
{
    if(_dbName == catalogName())
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = "You cannot destroy the " + catalogName() + " database";
        throw ex;
    }

    TransactionPtr tx = _connection->currentTransaction();
    bool ownTx = (tx == 0);
    if(ownTx)
    {   
        tx = _connection->beginTransaction();
    }
   
    DbTxn* txn = _connection->dbTxn();

    try
    {
        close();

        Catalog catalog(_connection, catalogName());
        catalog.erase(_dbName);
        _connection->dbEnv()->getEnv()->dbremove(txn, _dbName.c_str(), 0, 0);

        if(ownTx)
        {
            tx->commit();
        }
    }
    catch(const ::DbException& dx)
    {
        if(ownTx)
        {
            tx = _connection->currentTransaction();
            if(tx != 0)
            {
                try
                {
                    tx->rollback();
                }
                catch(...)
                {
                }
            }
        }

        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
    catch(...)
    {
        if(ownTx)
        {
            tx = _connection->currentTransaction();
            if(tx != 0)
            {
                try
                {
                    tx->rollback();
                }
                catch(...)
                {
                }
            }
        }
        throw;
    }
}


size_t
Freeze::MapHelperI::size() const
{
    //
    // TODO: DB_FAST_STAT doesn't seem to do what the documentation says...
    //
    DB_BTREE_STAT* s;

    try
    {
#if DB_VERSION_MAJOR != 4
   #error Freeze requires DB 4.x
#endif
#if DB_VERSION_MINOR < 3
       _db->stat(&s, 0);
#else
       _db->stat(_connection->dbTxn(), &s, 0);
#endif
    }
    catch(const ::DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }

    size_t num = s->bt_ndata;
    free(s);
    return num;
}


void
Freeze::MapHelperI::closeAllIterators()
{
    while(!_iteratorList.empty())
    {
        (*_iteratorList.begin())->close();
    }
}

const MapIndexBasePtr&
Freeze::MapHelperI::index(const string& name) const
{
    IndexMap::const_iterator p = _indices.find(name);
    if(p == _indices.end())
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = "Cannot find index \"" + name + "\"";
        throw ex;
    }
    return p->second;
}

void
Freeze::MapHelperI::close()
{
    if(_db != 0)
    {
        _connection->unregisterMap(this);
    }
    _db = 0;
    
    for(IndexMap::iterator p = _indices.begin(); p != _indices.end(); ++p)
    {
        MapIndexBasePtr& indexBase = p->second;

        indexBase->_impl = 0;
        indexBase->_map = 0;
    }
    _indices.clear();
}

void
Freeze::MapHelperI::closeAllIteratorsExcept(const IteratorHelperI::TxPtr& tx) const
{
    assert(tx != 0);

    list<IteratorHelperI*>::iterator q = _iteratorList.begin();

    while(q != _iteratorList.end())
    {
        if((*q)->tx().get() == tx.get())
        {
            ++q;
        }
        else
        {
            (*q)->close();
            q = _iteratorList.begin();
        }
    }
}


//
// MapIndexI
//

extern "C" 
{
static int customIndexCompare(DB* db, const DBT* dbt1, const DBT* dbt2)
{
    MapIndexI* me = static_cast<MapIndexI*>(db->app_private);
    Byte* first = static_cast<Byte*>(dbt1->data);
    Key k1(first, first + dbt1->size);
    first = static_cast<Byte*>(dbt2->data);
    Key k2(first, first + dbt2->size);

    return me->getKeyCompare()->compare(k1, k2);
}
}

static int 
callback(Db* secondary, const Dbt* key, const Dbt* value, Dbt* result)
{
    void* indexObj = secondary->get_app_private();
    MapIndexI* index = static_cast<MapIndexI*>(indexObj);
    assert(index != 0);
    return index->secondaryKeyCreate(secondary, key, value, result);
}


Freeze::MapIndexI::MapIndexI(const ConnectionIPtr& connection, SharedDb& db,
                             DbTxn* txn, bool createDb, const MapIndexBasePtr& index) :
    _index(index)
{
    assert(txn != 0);
    
    _db.reset(new Db(connection->dbEnv()->getEnv(), 0));
    _db->set_flags(DB_DUP | DB_DUPSORT);
    _db->set_app_private(this);

    u_int32_t flags = 0;
    if(createDb)
    {
        flags = DB_CREATE;
    }

    _dbName = db.dbName() + "." + _index->name();

    _db->set_app_private(this);

    if(index->compareEnabled())
    {
        _db->set_bt_compare(&customIndexCompare);
    }

    _db->open(txn, _dbName.c_str(), 0, DB_BTREE, flags, FREEZE_DB_MODE);

    //
    // To populate empty indices
    //
    flags = DB_CREATE;
    db.associate(txn, _db.get(), callback, flags);

    //
    // Note: caller catch and translates exceptions
    //
}
   
Freeze::MapIndexI::~MapIndexI()
{
    _db->close(0);
}

IteratorHelper* 
Freeze::MapIndexI::untypedFind(const Key& k, bool ro, const MapHelperI& m, 
                               bool onlyDups) const
{
    auto_ptr<IteratorHelperI> r(new IteratorHelperI(m, ro, _index, onlyDups));

    if(r->find(k))
    {
        return r.release();
    }
    else
    {
        return 0;
    }
}

IteratorHelper* 
Freeze::MapIndexI::untypedLowerBound(const Key& k, bool ro, const MapHelperI& m) const
{
    auto_ptr<IteratorHelperI> r(new IteratorHelperI(m, ro, _index, false));

    if(r->lowerBound(k))
    {
        return r.release();
    }
    else
    {
        return 0;
    }
}

IteratorHelper* 
Freeze::MapIndexI::untypedUpperBound(const Key& k, bool ro, const MapHelperI& m) const
{
    auto_ptr<IteratorHelperI> r(new IteratorHelperI(m, ro, _index, false));

    if(r->upperBound(k))
    {
        return r.release();
    }
    else
    {
        return 0;
    }
}

int 
Freeze::MapIndexI::untypedCount(const Key& k, const ConnectionIPtr& connection) const
{
    Dbt dbKey;
    initializeInDbt(k, dbKey);
    //
    // When we have a custom-comparison function, Berkeley DB returns
    // the key on-disk (when it finds one). We disable this behavior:
    // (ref Oracle SR 5925672.992)
    //
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    int result = 0;
    
    try
    {
        for(;;)
        {
            Dbc* dbc = 0;
            
            try
            {
                //
                // Move to the first record
                // 
                _db->cursor(0, &dbc, 0);
                bool found = (dbc->get(&dbKey, &dbValue, DB_SET) == 0);
                
                if(found)
                {
                    db_recno_t count = 0;
                    dbc->count(&count, 0);
                    result = static_cast<int>(count);
                }
                
                Dbc* toClose = dbc;
                dbc = 0;
                toClose->close();
                break; // for (;;)
            }
            catch(const DbDeadlockException&)
            {
                if(dbc != 0)
                {
                    try
                    {
                        dbc->close();
                    }
                    catch(const DbDeadlockException&)
                    {
                        //
                        // Ignored
                        //
                    }
                }

                if(connection->deadlockWarning())
                {
                    Warning out(connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapIndexI::untypedCount while searching \"" 
                        << _dbName << "\"; retrying ...";
                }

                //
                // Retry
                //
            }
            catch(...)
            {
                if(dbc != 0)
                {
                    try
                    {
                        dbc->close();
                    }
                    catch(const DbDeadlockException&)
                    {
                        //
                        // Ignored
                        //
                    }
                }
                throw;
            }
        }
    }
    catch(const DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
    
    return result;
}
    
int
Freeze::MapIndexI::secondaryKeyCreate(Db* secondary, const Dbt* dbKey, 
                                      const Dbt* dbValue, Dbt* result)
{
    Byte* first = static_cast<Byte*>(dbValue->get_data());
    Value value(first, first + dbValue->get_size());
    
    Key bytes;
    _index->marshalKey(value, bytes);

    result->set_flags(DB_DBT_APPMALLOC);
    void* data = malloc(bytes.size());
    memcpy(data, &bytes[0], bytes.size());
    result->set_data(data);
    result->set_size(static_cast<u_int32_t>(bytes.size()));
    return 0;
}

//
// Print for the various exception types.
//
void
Freeze::DeadlockException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ndatabase deadlock:\n" << message;
}

void
Freeze::NotFoundException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ndatabase record not found:\n" << message;
}

void
Freeze::DatabaseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << message;
}


void
Freeze::InvalidPositionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvalid position";
}
