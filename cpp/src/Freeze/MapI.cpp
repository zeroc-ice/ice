// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/MapI.h>
#include <Freeze/MapDb.h>
#include <Freeze/Exception.h>
#include <Freeze/Util.h>
#include <Freeze/TransactionHolder.h>
#include <Freeze/Catalog.h>
#include <Freeze/CatalogIndexList.h>
#include <IceUtil/UUID.h>
#include <IceUtil/StringConverter.h>
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

Freeze::MapIndexBase::MapIndexBase(const string& name, bool enabled) :
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
Freeze::MapIndexBase::begin(bool ro) const
{
    return _impl->begin(ro, *_map);
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

/*static*/ Freeze::MapHelper*
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

/*static*/ void
Freeze::MapHelper::recreate(const Freeze::ConnectionPtr& connection,
                            const string& dbName,
                            const string& key,
                            const string& value,
                            const Freeze::KeyCompareBasePtr& keyCompare,
                            const std::vector<MapIndexBasePtr>& indices)
{
    Freeze::ConnectionIPtr connectionI = Freeze::ConnectionIPtr::dynamicCast(connection.get());
    if(connectionI == 0)
    {
        throw DatabaseException(__FILE__, __LINE__, "Invalid connection");
    }

    if(dbName == catalogName() || dbName == catalogIndexListName())
    {
        throw DatabaseException(__FILE__, __LINE__,
                                "You cannot destroy recreate the \"" + dbName + "\" database");
    }

    if(connectionI->trace() >= 1)
    {
        Trace out(connectionI->communicator()->getLogger(), "Freeze.Map");
        out << "Recreating \"" << dbName << "\"";
    }

    TransactionPtr tx = connectionI->currentTransaction();
    bool ownTx = (tx == 0);

    Dbt keyDbt;
    keyDbt.set_flags(DB_DBT_REALLOC);
    Dbt valueDbt;
    valueDbt.set_flags(DB_DBT_REALLOC);

    try
    {
        for(;;)
        {
            try
            {
                if(ownTx)
                {
                    tx = 0;
                    tx = connectionI->beginTransaction();
                }

                DbTxn* txn = connectionI->dbTxn();

                if(connectionI->trace() >= 2)
                {
                    Trace out(connectionI->communicator()->getLogger(), "Freeze.Map");
                    out << "Removing all existing indices for \"" << dbName << "\"";
                }
                CatalogIndexList catalogIndexList(connection, catalogIndexListName());
                CatalogIndexList::iterator p = catalogIndexList.find(dbName);
                if(p != catalogIndexList.end())
                {
                    const StringSeq& indices = p->second;

                    for(size_t i = 0; i < indices.size(); ++i)
                    {
                        try
                        {
                            connection->removeMapIndex(dbName, indices[i]);
                        }
                        catch(const IndexNotFoundException&)
                        {
                            //
                            // Ignored
                            //
                        }
                    }
                    catalogIndexList.erase(p);
                }

                //
                // Rename existing database
                //
                string oldDbName = dbName + ".old-" + IceUtil::generateUUID();

                if(connectionI->trace() >= 2)
                {
                    Trace out(connectionI->communicator()->getLogger(), "Freeze.Map");
                    out << "Renaming \"" << dbName << "\" to \"" << oldDbName << "\"";
                }

                connectionI->dbEnv()->getEnv()->dbrename(txn, dbName.c_str(), 0, oldDbName.c_str(), 0);

                //
                // Fortunately, DB closes oldDb automatically when it goes out of scope
                //
                Db oldDb(connectionI->dbEnv()->getEnv(), 0);

                //
                // Berkeley DB expects file paths to be UTF8 encoded.
                //
                oldDb.open(txn, IceUtil::nativeToUTF8(oldDbName, IceUtil::getProcessStringConverter()).c_str(),
                           0, DB_BTREE, DB_THREAD, FREEZE_DB_MODE);

                IceUtil::UniquePtr<MapDb> newDb(new MapDb(connectionI, dbName, key, value, keyCompare, indices, true));

                if(connectionI->trace() >= 2)
                {
                    Trace out(connectionI->communicator()->getLogger(), "Freeze.Map");
                    out << "Writing contents of \"" << oldDbName << "\" to fresh \"" << dbName << "\"";
                }

                //
                // Now simply write all of oldDb into newDb
                //
                Dbc* dbc = 0;
                oldDb.cursor(txn, &dbc, 0);

                try
                {
                    while(dbc->get(&keyDbt, &valueDbt, DB_NEXT) == 0)
                    {
                        newDb->put(txn, &keyDbt, &valueDbt, 0);
                    }
                }
                catch(...)
                {
                    dbc->close();
                    throw;
                }
                dbc->close();

                if(connectionI->trace() >= 2)
                {
                    Trace out(connectionI->communicator()->getLogger(), "Freeze.Map");
                    out << "Transfer complete; removing \"" << oldDbName << "\"";
                }
                connectionI->dbEnv()->getEnv()->dbremove(txn, oldDbName.c_str(), 0, 0);

                if(ownTx)
                {
                    tx->commit();
                }

                break; // for (;;)
            }
            catch(const DbDeadlockException& dx)
            {
                if(ownTx)
                {
                    if(connectionI->deadlockWarning())
                    {
                        Warning out(connectionI->communicator()->getLogger());
                        out << "Deadlock in Freeze::MapHelperI::recreate on Db \""
                            << dbName << "\"; retrying ...";
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

                throw DatabaseException(__FILE__, __LINE__, dx.what());
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
        free(keyDbt.get_data());
        free(valueDbt.get_data());
    }
    catch(...)
    {
        free(keyDbt.get_data());
        free(valueDbt.get_data());

        throw;
    }
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

    IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(actualMap, readOnly, 0, false));
    if(r->next())
    {
        return r.release();
    }
    else
    {
        return 0;
    }
}

Freeze::IteratorHelper::~IteratorHelper() ICE_NOEXCEPT_FALSE
{
}

//
// MapCodecBase (from Map.h)
//
Freeze::MapCodecBase::MapCodecBase(const Ice::CommunicatorPtr& communicator, const Ice::EncodingVersion& encoding) :
    _stream(IceInternal::getInstance(communicator).get(), encoding),
    _dbt(0)
{
}

Freeze::MapCodecBase::~MapCodecBase()
{
    delete _dbt;
}

void
Freeze::MapCodecBase::init()
{
    _dbt = new Dbt;
    initializeInDbt(_stream, *_dbt);
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
#ifdef ICE_CPP11
        _tx.reset(new Tx(_map));
#else
        _tx = new Tx(_map);
#endif
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

Freeze::IteratorHelperI::~IteratorHelperI() ICE_NOEXCEPT_FALSE
{
    close();
}

bool
Freeze::IteratorHelperI::find(const Key& key) const
{
    Dbt dbKey;
    initializeInDbt(key, dbKey);
    return find(dbKey);
}

bool
Freeze::IteratorHelperI::find(const Dbt& key) const
{
    assert((key.get_flags() & DB_DBT_USERMEM) != 0);
    Dbt dbKey(key);

#if (DB_VERSION_MAJOR <= 4) || (DB_VERSION_MAJOR == 5 && DB_VERSION_MINOR <= 1)
    //
    // When we have a custom-comparison function, Berkeley DB returns
    // the key on-disk (when it finds one). We disable this behavior:
    // (ref Oracle SR 5925672.992)
    //
    dbKey.set_flags(dbKey.get_flags() | DB_DBT_PARTIAL);
#else
    //
    // In DB > 5.1 we can not set DB_DBT_PARTIAL in the key Dbt,
    // when using DB_SET, we must resize the Dbt key param to hold enough
    // space or Dbc::get fails with DB_BUFFER_SMALL.
    //
    dbKey.set_ulen(dbKey.get_size());
#endif

    //
    // Keep 0 length since we're not interested in the data.
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    for(;;)
    {
        try
        {
            return _dbc->get(&dbKey, &dbValue, DB_SET) == 0;
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

    size_t keySize = _key.size();
    if(keySize < 1024)
    {
        keySize = 1024;
    }
    _key.resize(keySize);

    Dbt dbKey;
    initializeOutDbt(_key, dbKey);

    size_t valueSize = _value.size();
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
            handleDbException(dx, _key, dbKey, _value, dbValue, __FILE__, __LINE__);
        }
    }
}

const Freeze::Key*
Freeze::IteratorHelperI::get() const
{
    size_t keySize = _key.size();
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
    Dbt dbValue;
    initializeInDbt(value, dbValue);
    set(dbValue);
}

void
Freeze::IteratorHelperI::set(const Dbt& value)
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

    Dbt dbValue(value);

    if(_tx != 0)
    {
        _map.closeAllIteratorsExcept(_tx);
    }

    try
    {
#ifndef NDEBUG
        int err = _dbc->put(&dbKey, &dbValue, DB_CURRENT);
        assert(err == 0);
#else
        _dbc->put(&dbKey, &dbValue, DB_CURRENT);
#endif
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
        catch(const ::DbDeadlockException&)
        {
            // Ignored - it's unclear if this can ever occur
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

    // this can raise an exception when committing the transaction
    // (only for read/write iterators)
#ifdef ICE_CPP11
    _tx.reset();
#else
    _tx = 0;
#endif
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

Freeze::IteratorHelperI::Tx::~Tx() ICE_NOEXCEPT_FALSE
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
    _db(connection->dbEnv()->getSharedMapDb(dbName, key, value, keyCompare, indices, createDb)),
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

#ifdef NDEBUG
        _indices.insert(IndexMap::value_type(indexBase->name(), indexBase));
#else
        bool inserted = _indices.insert(IndexMap::value_type(indexBase->name(), indexBase)).second;
        assert(inserted);
#endif

        indexBase->_map = this;
    }

    _connection->registerMap(this);
}

Freeze::MapHelperI::~MapHelperI()
{
    try
    {
        close();
    }
    catch(const DatabaseException& ex)
    {
        Ice::Error error(_connection->getCommunicator()->getLogger());
        error << "Freeze: closing map " << _dbName << " raised: " << ex;
    }
}

Freeze::IteratorHelper*
Freeze::MapHelperI::find(const Key& k, bool readOnly) const
{
    for(;;)
    {
        try
        {
            IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(*this, readOnly, 0, false));
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
Freeze::MapHelperI::find(const Dbt& k, bool readOnly) const
{
    for(;;)
    {
        try
        {
            IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(*this, readOnly, 0, false));
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
            IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(*this, readOnly, 0, false));
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
            IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(*this, readOnly, 0, false));
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
    put(dbKey, dbValue);
}

void
Freeze::MapHelperI::put(const Dbt& key, const Dbt& value)
{
    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
        closeAllIterators();
    }

    Dbt dbKey(key);
    Dbt dbValue(value);

    for(;;)
    {
        try
        {
            int err = _db->put(txn, &dbKey, &dbValue, txn != 0 ? 0 : DB_AUTO_COMMIT);

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
    return erase(dbKey);
}

size_t
Freeze::MapHelperI::erase(const Dbt& key)
{
    DbTxn* txn = _connection->dbTxn();
    if(txn == 0)
    {
        closeAllIterators();
    }

    Dbt dbKey(key);

    for(;;)
    {
        try
        {
            int err = _db->del(txn, &dbKey, txn != 0 ? 0 : DB_AUTO_COMMIT);

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
    return count(dbKey);
}

size_t
Freeze::MapHelperI::count(const Dbt& key) const
{
    Dbt dbKey(key);

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

    Dbt dbKey;
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    try
    {
        for(;;)
        {
            Dbc* dbc = 0;

            try
            {
                IteratorHelperI::TxPtr tx;
                if(txn == 0)
                {
#ifdef ICE_CPP11
                    tx.reset(new IteratorHelperI::Tx(*this));
#else
                    tx = new IteratorHelperI::Tx(*this);
#endif
                    txn = tx->getTxn();
                }

                _db->cursor(txn, &dbc, 0);
                while(dbc->get(&dbKey, &dbValue, DB_NEXT | DB_RMW) == 0)
                {
                    dbc->del(0);
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
                        if(txn != 0)
                        {
                            throw;
                        }
                        else
                        {
                            //
                            // Ignored
                            //
                        }
                    }
                }

                if(_connection->deadlockWarning())
                {
                    Warning out(_connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapHelperI::clear on Map \""
                        << _dbName << "\"; retrying ...";
                }

                if(txn != 0)
                {
                    throw;
                }
                //
                // Otherwise retry
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
                        if(txn != 0)
                        {
                            throw;
                        }
                        else
                        {
                            //
                            // Ignored
                            //
                        }
                    }
                }
                throw;
            }
        }
    }
    catch(const DbDeadlockException& dx)
    {
        throw DeadlockException(__FILE__, __LINE__, dx.what(), _connection->currentTransaction());
    }
    catch(const DbException& dx)
    {
        throw DatabaseException(__FILE__, __LINE__, dx.what());
    }
}

void
Freeze::MapHelperI::destroy()
{
    if(_dbName == catalogName() || _dbName == catalogIndexListName())
    {
        throw DatabaseException(__FILE__, __LINE__, "You cannot destroy the \"" + _dbName + "\" database");
    }

    if(_db == 0)
    {
        //
        // We need an opened map to gather the index names
        //
        throw DatabaseException(__FILE__, __LINE__, "This map is closed");
    }

    if(_connection->currentTransaction())
    {
        throw DatabaseException(__FILE__, __LINE__, "Cannot destroy map within transaction");
    }

    if(_trace >= 1)
    {
        Trace out(_connection->communicator()->getLogger(), "Freeze.Map");
        out << "Destroying \"" << _dbName << "\"";
    }

    vector<string> indexNames;
    for(IndexMap::iterator p = _indices.begin(); p != _indices.end(); ++p)
    {
        indexNames.push_back(p->second->name());
    }

    closeDb();

    for(;;)
    {
        try
        {
            TransactionHolder tx(_connection);
            DbTxn* txn = _connection->dbTxn();

            Catalog catalog(_connection, catalogName());
            catalog.erase(_dbName);

            CatalogIndexList catalogIndexList(_connection, catalogIndexListName());
            catalogIndexList.erase(_dbName);

            _connection->dbEnv()->getEnv()->dbremove(txn, _dbName.c_str(), 0, 0);

            //
            // Remove all indices
            //
            for(vector<string>::iterator q = indexNames.begin(); q != indexNames.end(); ++q)
            {
                _connection->removeMapIndex(_dbName, *q);
            }

            tx.commit();

            break; // for(;;)
        }
        catch(const DbDeadlockException&)
        {
            if(_connection->deadlockWarning())
            {
                Warning out(_connection->communicator()->getLogger());
                out << "Deadlock in Freeze::MapHelperI::destroy on Map \""
                    << _dbName << "\"; retrying ...";
            }

            //
            // Ignored, try again
            //
        }
        catch(const DbException& dx)
        {
            throw DatabaseException(__FILE__, __LINE__, dx.what());
        }
    }
}

size_t
Freeze::MapHelperI::size() const
{
    DB_BTREE_STAT* s;

    try
    {
#if DB_VERSION_MAJOR < 4
#error Freeze requires DB 4.x or greater
#endif
#if (DB_VERSION_MAJOR == 4) && (DB_VERSION_MINOR < 3)
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
Freeze::MapHelperI::closeDb()
{
    close();
    _connection->dbEnv()->removeSharedMapDb(_dbName);
}

Freeze::ConnectionPtr
Freeze::MapHelperI::getConnection() const
{
    return _connection;
}

void
Freeze::MapHelperI::close()
{
    if(_db != 0)
    {
        closeAllIterators();
        _connection->unregisterMap(this);
    }
    _db = 0;

    //
    // We can't clear the indexBase as MapIndexI is using
    // the first map's indexBase objects
    //
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
#if (DB_VERSION_MAJOR <= 5)
    static int customIndexCompare(DB* db, const DBT* dbt1, const DBT* dbt2)
#else
    static int customIndexCompare(DB* db, const DBT* dbt1, const DBT* dbt2, size_t*)
#endif
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

Freeze::MapIndexI::MapIndexI(const ConnectionIPtr& connection, MapDb& db,
                             DbTxn* txn, bool createDb, const MapIndexBasePtr& index) :
    _index(index)
{
    assert(txn != 0);

    _db.reset(new Db(connection->dbEnv()->getEnv(), 0));
    _db->set_flags(DB_DUP | DB_DUPSORT);

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

    Ice::PropertiesPtr properties = connection->communicator()->getProperties();
    string propPrefix = "Freeze.Map." + _dbName + ".";
    int btreeMinKey = properties->getPropertyAsInt(propPrefix + "BtreeMinKey");
    if(btreeMinKey > 2)
    {
        if(connection->trace() >= 1)
        {
            Trace out(connection->communicator()->getLogger(), "Freeze.Map");
            out << "Setting \"" << _dbName << "\"'s btree minkey to " << btreeMinKey;
        }

        _db->set_bt_minkey(btreeMinKey);
    }

    bool checksum = properties->getPropertyAsInt(propPrefix + "Checksum") > 0;
    if(checksum)
    {
        if(connection->trace() >= 1)
        {
            Trace out(connection->communicator()->getLogger(), "Freeze.Map");
            out << "Turning checksum on for \"" << _dbName << "\"";
        }

        _db->set_flags(DB_CHKSUM);
    }

    int pageSize = properties->getPropertyAsInt(propPrefix + "PageSize");
    if(pageSize > 0)
    {
        if(connection->trace() >= 1)
        {
            Trace out(connection->communicator()->getLogger(), "Freeze.Map");
            out << "Setting \"" << _dbName << "\"'s pagesize to " << pageSize;
        }

        _db->set_pagesize(pageSize);
    }

    if(connection->trace() >= 1)
    {
        Trace out(connection->communicator()->getLogger(), "Freeze.Map");
        out << "Opening index \"" << _dbName << "\"";
    }

    //
    // Berkeley DB expects file paths to be UTF8 encoded.
    //
    _db->open(txn, IceUtil::nativeToUTF8(_dbName, IceUtil::getProcessStringConverter()).c_str(), 0, DB_BTREE, flags,
              FREEZE_DB_MODE);

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
    try
    {
        _db->close(0);
    }
    catch(const DbException&)
    {
        // Ignored
    }
}

IteratorHelper*
Freeze::MapIndexI::begin(bool ro, const MapHelperI& m) const
{
    IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(m, ro, _index, false));

    if(r->next())
    {
        return r.release();
    }
    else
    {
        return 0;
    }
}

IteratorHelper*
Freeze::MapIndexI::untypedFind(const Key& k, bool ro, const MapHelperI& m, bool onlyDups) const
{
    IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(m, ro, _index, onlyDups));

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
    IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(m, ro, _index, false));

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
    IceUtil::UniquePtr<IteratorHelperI> r(new IteratorHelperI(m, ro, _index, false));

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
#if (DB_VERSION_MAJOR <= 4)
    //
    // When we have a custom-comparison function, Berkeley DB returns
    // the key on-disk (when it finds one). We disable this behavior:
    // (ref Oracle SR 5925672.992)
    //
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
#else
    //
    // In DB 5.x we can not set DB_DBT_PARTIAL in the key Dbt,
    // when using DB_SET, we must resize the Dbt key param to hold enought
    // space or Dbc::get fails with DB_BUFFER_SMALL.
    //
    dbKey.set_flags(DB_DBT_USERMEM);
    dbKey.set_ulen(static_cast<u_int32_t>(k.size()));
#endif

    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    int result = 0;

    DbTxn* txn = connection->dbTxn();

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
                _db->cursor(txn, &dbc, 0);
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
                        if(txn != 0)
                        {
                            throw;
                        }
                        else
                        {
                            //
                            // Ignored
                            //
                        }
                    }
                }

                if(connection->deadlockWarning())
                {
                    Warning out(connection->communicator()->getLogger());
                    out << "Deadlock in Freeze::MapIndexI::untypedCount while searching \""
                        << _dbName << "\"";
                }

                if(txn != 0)
                {
                    throw;
                }
                //
                // Otherwise retry
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
                        if(txn != 0)
                        {
                            throw;
                        }
                        else
                        {
                            //
                            // Ignored
                            //
                        }
                    }
                }
                throw;
            }
        }
    }
    catch(const DbDeadlockException& dx)
    {
        throw DeadlockException(__FILE__, __LINE__, dx.what(), connection->currentTransaction());
    }
    catch(const DbException& dx)
    {
        throw DatabaseException(__FILE__, __LINE__, dx.what());
    }

    return result;
}

int
Freeze::MapIndexI::secondaryKeyCreate(Db* /*secondary*/, const Dbt* /*dbKey*/, const Dbt* dbValue, Dbt* result)
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
Freeze::DeadlockException::ice_print(std::ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ndatabase deadlock:\n" << message;
}

void
Freeze::NotFoundException::ice_print(std::ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ndatabase record not found:\n" << message;
}

void
Freeze::DatabaseException::ice_print(std::ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << message;
}

void
Freeze::IndexNotFoundException::ice_print(std::ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ncould not find index \"" << indexName << "\" on map \"" << mapName << "\"";
}
