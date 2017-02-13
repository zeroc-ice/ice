// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_UTIL_H
#define FREEZE_UTIL_H

#include <Ice/Ice.h>
#include <Freeze/DB.h>
#include <db_cxx.h>
#include <sys/stat.h>

#ifdef _WIN32
#   define FREEZE_DB_MODE 0
#else
#   define FREEZE_DB_MODE (S_IRUSR | S_IWUSR)
#endif


namespace Freeze
{

inline void
initializeInDbt(const std::vector<Ice::Byte>& v, Dbt& dbt)
{
    dbt.set_data(const_cast<Ice::Byte*>(&v[0]));
    dbt.set_size(static_cast<u_int32_t>(v.size()));
    dbt.set_ulen(0);
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}

inline void
initializeInDbt(IceInternal::BasicStream& s, Dbt& dbt)
{
    dbt.set_data(const_cast<Ice::Byte*>(s.b.begin()));
    dbt.set_size(static_cast<u_int32_t>(s.b.size()));
    dbt.set_ulen(0);
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}

inline void
initializeOutDbt(std::vector<Ice::Byte>& v, Dbt& dbt)
{
    v.resize(v.capacity());
    dbt.set_data(&v[0]);
    dbt.set_size(0);
    dbt.set_ulen(static_cast<u_int32_t>(v.size()));
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}

inline void
initializeOutDbt(IceInternal::BasicStream& s, Dbt& dbt)
{
    dbt.set_data(const_cast<Ice::Byte*>(s.b.begin()));
    dbt.set_size(0);
    dbt.set_ulen(static_cast<u_int32_t>(s.b.size()));
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}


//
// Handles a Berkeley DB DbException by resizing the
// given key/value/dbt (when the exception's errno is
// DB_SMALL_BUFFER) or by throwing a
// Freeze::DatabaseException
//

void
handleDbException(const DbException&, const char*, int);

void
handleDbException(const DbException&, Key&, Dbt&,
                  const char*, int);

void
handleDbException(const DbException&, Key&, Dbt&, Value&, Dbt&,
                  const char*, int);

}


#endif

