// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
Freeze::handleMemoryException(const DbMemoryException& dx, Key& key, Dbt& dbKey)
{
    if(dbKey.get_size() > dbKey.get_ulen())
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
	//
	// Real problem
	//
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

void
Freeze::handleMemoryException(const DbMemoryException& dx, Key& key, Dbt& dbKey, 
			      Value& value, Dbt& dbValue)
{
    bool resized = false;
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
    
    if(!resized)
    {
	//
	// Real problem
	//
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}
