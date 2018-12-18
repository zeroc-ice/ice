// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceDB/IceDB.h>
#include <Ice/Initialize.h>

#include <lmdb.h>

using namespace IceDB;
using namespace std;

LMDBException::LMDBException(const char* file, int line, int err) :
    IceUtil::Exception(file, line),
    _error(err)
{
}

#ifndef ICE_CPP11_COMPILER
LMDBException::~LMDBException() throw()
{
}
#endif

string
LMDBException::ice_id() const
{
    return "::IceDB::LMDBException";
}

void
LMDBException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
    out << ": " << mdb_strerror(_error);
}

#ifndef ICE_CPP11_MAPPING
LMDBException*
LMDBException::ice_clone() const
{
    return new LMDBException(*this);
}
#endif

void
LMDBException::ice_throw() const
{
    throw *this;
}

int
LMDBException::error() const
{
    return _error;
}

KeyTooLongException::KeyTooLongException(const char* file, int line, size_t size) :
    IceUtil::Exception(file, line),
    _size(size)
{
}

#ifndef ICE_CPP11_COMPILER
KeyTooLongException::~KeyTooLongException() throw()
{
}
#endif

string
KeyTooLongException::ice_id() const
{
    return "::IceDB::KeyTooLongException";
}

void
KeyTooLongException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
    out << ": ";
    if(_size > 0)
    {
        out << "Key size = " << _size << ", ";
    }
    out << "Max size = " << maxKeySize;
}

#ifndef ICE_CPP11_MAPPING
KeyTooLongException*
KeyTooLongException::ice_clone() const
{
    return new KeyTooLongException(*this);
}
#endif

void
KeyTooLongException::ice_throw() const
{
    throw *this;
}

BadEnvException::BadEnvException(const char* file, int line, size_t size) :
    IceUtil::Exception(file, line),
    _size(size)
{
}

#ifndef ICE_CPP11_COMPILER
BadEnvException::~BadEnvException() throw()
{
}
#endif

string
BadEnvException::ice_id() const
{
    return "::IceDB::BadEnvException";
}

void
BadEnvException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
    out << ": LMDB env max key size = " << _size;
    out << ", IceDB max key size = " << maxKeySize;
}

#ifndef ICE_CPP11_MAPPING
BadEnvException*
BadEnvException::ice_clone() const
{
    return new BadEnvException(*this);
}
#endif

void
BadEnvException::ice_throw() const
{
    throw *this;
}

Env::Env(const string& path, MDB_dbi maxDbs, size_t mapSize, unsigned int maxReaders)
{
    int rc = mdb_env_create(&_menv);
    if(rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }

    if(maxDbs != 0)
    {
        rc = mdb_env_set_maxdbs(_menv, maxDbs);
        if(rc != MDB_SUCCESS)
        {
            throw LMDBException(__FILE__, __LINE__, rc);
        }
    }

    if(mapSize != 0)
    {
        // Make sure the map size is a multiple of the page size
        size_t pageSize;
#ifdef _WIN32
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        pageSize = si.dwPageSize;
#else
        pageSize = sysconf(_SC_PAGESIZE);
#endif
        size_t remainder = mapSize % pageSize;
        if(remainder != 0)
        {
            mapSize = mapSize + pageSize - remainder;
        }
        rc = mdb_env_set_mapsize(_menv, mapSize);
        if(rc != MDB_SUCCESS)
        {
            throw LMDBException(__FILE__, __LINE__, rc);
        }
    }

    if(maxReaders != 0)
    {
        rc = mdb_env_set_maxreaders(_menv, maxReaders);
        if(rc != MDB_SUCCESS)
        {
            throw LMDBException(__FILE__, __LINE__, rc);
        }
    }

    rc = mdb_env_open(_menv, path.c_str(), 0, 0644);
    if(rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }

    size_t envMaxKeySize = mdb_env_get_maxkeysize(_menv);
    if(maxKeySize > envMaxKeySize)
    {
        throw BadEnvException(__FILE__, __LINE__, envMaxKeySize);
    }
}

Env::~Env()
{
    close();
}

void
Env::close()
{
    if(_menv != 0)
    {
        mdb_env_close(_menv);
        _menv = 0;
    }
}

MDB_env*
Env::menv() const
{
    return _menv;
}

Txn::Txn(const Env& env, unsigned int flags)
{
    const int rc = mdb_txn_begin(env.menv(), 0, flags, &_mtxn);
    if(rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

Txn::~Txn()
{
    rollback();
}

void
Txn::commit()
{
    const int rc = mdb_txn_commit(_mtxn);
    _mtxn = 0;
    if(rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

void
Txn::rollback()
{
    if(_mtxn != 0)
    {
        mdb_txn_abort(_mtxn);
        _mtxn = 0;
    }
}

MDB_txn*
Txn::mtxn() const
{
    return _mtxn;
}

ReadOnlyTxn::~ReadOnlyTxn()
{
    // Out of line to avoid weak vtable
}

ReadOnlyTxn::ReadOnlyTxn(const Env& env) :
    Txn(env, MDB_RDONLY)
{
}

void
ReadOnlyTxn::reset()
{
    mdb_txn_reset(_mtxn);
}

void
ReadOnlyTxn::renew()
{
    const int rc = mdb_txn_renew(_mtxn);
    if(rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

ReadWriteTxn::~ReadWriteTxn()
{
    // Out of line to avoid weak vtable
}

ReadWriteTxn::ReadWriteTxn(const Env& env) :
    Txn(env, 0)
{
}

DbiBase::DbiBase(const Txn& txn, const std::string& name, unsigned int flags, MDB_cmp_func* cmp)
{
    int rc = mdb_dbi_open(txn.mtxn(), name.c_str(), flags, &_mdbi);
    if(rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
    if(cmp != 0)
    {
        rc = mdb_set_compare(txn.mtxn(), _mdbi, cmp);
        if(rc != MDB_SUCCESS)
        {
            throw LMDBException(__FILE__, __LINE__, rc);
        }
    }
}

DbiBase::DbiBase() :
    _mdbi(0)
{
}

DbiBase::~DbiBase()
{
}

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
    if(rc != MDB_SUCCESS && rc != MDB_NOTFOUND)
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
    if(rc != MDB_SUCCESS)
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

CursorBase::CursorBase(MDB_dbi dbi, const Txn& txn, bool readOnly) :
    _readOnly(readOnly)
{
    const int rc = mdb_cursor_open(txn.mtxn(), dbi, &_mcursor);
    if(rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

CursorBase::~CursorBase()
{
    if(_readOnly)
    {
        close();
    }
}

void
CursorBase::close()
{
    if(_mcursor != 0)
    {
        mdb_cursor_close(_mcursor);
        _mcursor = 0;
    }
}

MDB_cursor*
CursorBase::mcursor() const
{
    return _mcursor;
}

bool
CursorBase::get(MDB_val* key, MDB_val* data, MDB_cursor_op op)
{
    const int rc = mdb_cursor_get(_mcursor, key, data, op);
    if(rc != MDB_SUCCESS && rc != MDB_NOTFOUND)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
    return rc == MDB_SUCCESS;
}

void
CursorBase::put(MDB_val* key, MDB_val* data, unsigned int flags)
{
    assert(key->mv_size <= maxKeySize);
    assert(!_readOnly);

    const int rc = mdb_cursor_put(_mcursor, key, data, flags);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
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

void
CursorBase::del(unsigned int flags)
{
    assert(!_readOnly);

    const int rc = mdb_cursor_del(_mcursor, flags);
    if (rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
}

void
CursorBase::renew(const ReadOnlyTxn& txn)
{
    assert(_readOnly);
    const int rc = mdb_cursor_renew(txn.mtxn(), CursorBase::_mcursor);
    if(rc != MDB_SUCCESS)
    {
        throw LMDBException(__FILE__, __LINE__, rc);
    }
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

   return ((configValue <= 0) ? defaultMapSize : configValue) * 1024 * 1024;
}
