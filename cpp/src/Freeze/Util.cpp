// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/Util.h>
#include <Freeze/Exception.h>

using namespace Freeze;
using namespace Ice;
using namespace std;

void
Freeze::handleDbException(const DbException& dx, 
                          const char* file, int line)
{
    throw DatabaseException(file, line, dx.what());
}

void
Freeze::handleDbException(const DbException& dx, 
                          Key& key, Dbt& dbKey, 
                          const char* file, int line)
{
    bool bufferSmallException =
#if (DB_VERSION_MAJOR == 4) && (DB_VERSION_MINOR == 2)
        (dx.get_errno() == ENOMEM);
#else
        (dx.get_errno() == DB_BUFFER_SMALL || dx.get_errno() == ENOMEM);
#endif  
        
    if(bufferSmallException && (dbKey.get_size() > dbKey.get_ulen()))
    {
        //
        // Keep the old key size in case it's used as input
        //
        size_t oldKeySize = key.size();

        key.resize(dbKey.get_size());
        initializeOutDbt(key, dbKey);
        dbKey.set_size(static_cast<u_int32_t>(oldKeySize));
    }
    else
    {
        handleDbException(dx, file, line);
    }
}

void
Freeze::handleDbException(const DbException& dx, 
                          Key& key, Dbt& dbKey, 
                          Value& value, Dbt& dbValue,
                          const char* file, int line)
{
    bool bufferSmallException =
#if (DB_VERSION_MAJOR == 4) && (DB_VERSION_MINOR == 2)
        (dx.get_errno() == ENOMEM);
#else
        (dx.get_errno() == DB_BUFFER_SMALL || dx.get_errno() == ENOMEM);
#endif  

    bool resized = false;       
    if(bufferSmallException)
    {
        if(dbKey.get_size() > dbKey.get_ulen())
        {
            size_t oldKeySize = key.size();
            key.resize(dbKey.get_size());
            initializeOutDbt(key, dbKey);
            dbKey.set_size(static_cast<u_int32_t>(oldKeySize));
            resized = true;
        }
        
        if(dbValue.get_size() > dbValue.get_ulen())
        {
            value.resize(dbValue.get_size());
            initializeOutDbt(value, dbValue);
            resized = true;
        }
    }
    
    if(!resized)
    {
        handleDbException(dx, file, line);
    }
}
