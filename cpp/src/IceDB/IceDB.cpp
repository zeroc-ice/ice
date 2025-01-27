// Copyright (c) ZeroC, Inc.

#include "IceDB.h"
#include "Ice/Initialize.h"

#include <lmdb.h>
#include <sstream>

#ifndef _WIN32
#    include <unistd.h>
#endif

using namespace IceDB;
using namespace std;

LMDBException::LMDBException(const char* file, int line, int error)
    : Ice::LocalException(file, line, mdb_strerror(error))
{
}

const char*
LMDBException::ice_id() const noexcept
{
    return "::IceDB::LMDBException";
}

namespace
{
    inline string createKeyTooLongMessage(size_t size)
    {
        ostringstream os;
        os << "requested LMDB key size (" << size << ") exceeds max size (" << maxKeySize << ")";
        return os.str();
    }
}

KeyTooLongException::KeyTooLongException(const char* file, int line, size_t size)
    : Ice::LocalException(file, line, createKeyTooLongMessage(size))
{
}

const char*
KeyTooLongException::ice_id() const noexcept
{
    return "::IceDB::KeyTooLongException";
}

namespace
{
    inline string createBadEnvMessage(size_t size)
    {
        ostringstream os;
        os << "the LMDB env max key size (" << size << ") is smaller than IceDB's max size (" << maxKeySize << ")";
        return os.str();
    }
}

BadEnvException::BadEnvException(const char* file, int line, size_t size)
    : Ice::LocalException(file, line, createBadEnvMessage(size))
{
}

const char*
BadEnvException::ice_id() const noexcept
{
    return "::IceDB::BadEnvException";
}

Env::Env(const string& path, MDB_dbi maxDbs, size_t mapSize, unsigned int maxReaders)
{
    int rc = mdb_env_create(&_menv);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }

    if (maxDbs != 0)
    {
        rc = mdb_env_set_maxdbs(_menv, maxDbs);
        if (rc != MDB_SUCCESS)
        {
            throw LMDBException(__FILE__, __LINE__, rc);
        }
    }

    if (mapSize != 0)
    {
        // Make sure the map size is a multiple of the page size
        size_t pageSize;
#ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        pageSize = si.dwPageSize;
#else
        long sz = sysconf(_SC_PAGESIZE);
        pageSize = static_cast<size_t>(sz == -1 ? 4096 : sz);
#endif
        size_t remainder = mapSize % pageSize;
        if (remainder != 0)
        {
            mapSize = mapSize + pageSize - remainder;
        }
        rc = mdb_env_set_mapsize(_menv, mapSize);
        if (rc != MDB_SUCCESS)
        {
            throw LMDBException(__FILE__, __LINE__, rc);
        }
    }

    if (maxReaders != 0)
    {
        rc = mdb_env_set_maxreaders(_menv, maxReaders);
        if (rc != MDB_SUCCESS)
        {
            throw LMDBException(__FILE__, __LINE__, rc);
        }
    }

    rc = mdb_env_open(_menv, path.c_str(), 0, 0644);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }

    auto envMaxKeySize = static_cast<size_t>(mdb_env_get_maxkeysize(_menv));
    if (maxKeySize > envMaxKeySize)
    {
        throw BadEnvException(__FILE__, __LINE__, envMaxKeySize);
    }
}

Env::~Env() { close(); }

void
Env::close()
{
    if (_menv != nullptr)
    {
        mdb_env_close(_menv);
        _menv = nullptr;
    }
}

MDB_env*
Env::menv() const
{
    return _menv;
}

Txn::Txn(const Env& env, unsigned int flags) : _readOnly(flags == MDB_RDONLY)
{
    const int rc = mdb_txn_begin(env.menv(), nullptr, flags, &_mtxn);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

Txn::~Txn() { rollback(); }

void
Txn::commit()
{
    const int rc = mdb_txn_commit(_mtxn);
    _mtxn = nullptr;
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

void
Txn::rollback()
{
    if (_mtxn != nullptr)
    {
        mdb_txn_abort(_mtxn);
        _mtxn = nullptr;
    }
}

MDB_txn*
Txn::mtxn() const
{
    return _mtxn;
}

ReadOnlyTxn::~ReadOnlyTxn() = default;

ReadOnlyTxn::ReadOnlyTxn(const Env& env) : Txn(env, MDB_RDONLY) {}

ReadWriteTxn::~ReadWriteTxn() = default;

ReadWriteTxn::ReadWriteTxn(const Env& env) : Txn(env, 0) {}

DbiBase::DbiBase(const Txn& txn, const std::string& name, unsigned int flags, MDB_cmp_func* cmp)
{
    int rc = mdb_dbi_open(txn.mtxn(), name.c_str(), flags, &_mdbi);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
    if (cmp != nullptr)
    {
        rc = mdb_set_compare(txn.mtxn(), _mdbi, cmp);
        if (rc != MDB_SUCCESS)
        {
            throw LMDBException(__FILE__, __LINE__, rc);
        }
    }
}

DbiBase::DbiBase() : _mdbi(0) {}

void
DbiBase::clear(const ReadWriteTxn& txn)
{
    const int rc = mdb_drop(txn.mtxn(), _mdbi, 0);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

MDB_dbi
DbiBase::mdbi() const
{
    return _mdbi;
}

bool
DbiBase::get(const Txn& txn, MDB_val* key, MDB_val* data) const
{
    assert(key->mv_size <= maxKeySize);

    const int rc = mdb_get(txn.mtxn(), _mdbi, key, data);
    if (rc != MDB_SUCCESS && rc != MDB_NOTFOUND)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
    return rc == MDB_SUCCESS;
}

void
DbiBase::put(const ReadWriteTxn& txn, MDB_val* key, MDB_val* data, unsigned int flags)
{
    assert(key->mv_size <= maxKeySize);

    const int rc = mdb_put(txn.mtxn(), _mdbi, key, data, flags);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

bool
DbiBase::find(const Txn& txn, MDB_val* key) const
{
    MDB_val data;
    return get(txn, key, &data);
}

bool
DbiBase::del(const ReadWriteTxn& txn, MDB_val* key, MDB_val* data)
{
    assert(key->mv_size <= maxKeySize);

    const int rc = mdb_del(txn.mtxn(), _mdbi, key, data);
    if (rc != MDB_SUCCESS && rc != MDB_NOTFOUND)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
    return rc == MDB_SUCCESS;
}

CursorBase::CursorBase(MDB_dbi dbi, const Txn& txn) : _readOnly(txn.isReadOnly())
{
    const int rc = mdb_cursor_open(txn.mtxn(), dbi, &_mcursor);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

CursorBase::~CursorBase()
{
    if (_readOnly)
    {
        close();
    }
}

void
CursorBase::close()
{
    if (_mcursor != nullptr)
    {
        mdb_cursor_close(_mcursor);
        _mcursor = nullptr;
    }
}

bool
CursorBase::get(MDB_val* key, MDB_val* data, MDB_cursor_op op)
{
    const int rc = mdb_cursor_get(_mcursor, key, data, op);
    if (rc != MDB_SUCCESS && rc != MDB_NOTFOUND)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
    return rc == MDB_SUCCESS;
}

bool
CursorBase::find(MDB_val* key)
{
    MDB_val data;
    return get(key, &data, MDB_SET);
}

bool
CursorBase::find(MDB_val* key, MDB_val* data)
{
    return get(key, data, MDB_SET);
}

//
// On Windows, we use a default LMDB map size of 10MB, whereas on other platforms
// (Linux, macOS), we use a default of 100MB.
//
// On Windows, LMDB does not use sparse files and allocates immediately the file
// with the given (max) size. This is why we need a fairly small default map size
// on Windows, and a larger value on other platforms.

size_t
IceDB::getMapSize(int configValue)
{
#ifdef _WIN32
    const size_t defaultMapSize = 10;
#else
    const size_t defaultMapSize = 100;
#endif

    return ((configValue <= 0) ? defaultMapSize : static_cast<size_t>(configValue)) * 1024 * 1024;
}
