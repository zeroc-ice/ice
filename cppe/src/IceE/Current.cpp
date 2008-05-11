// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Current.h>

bool
Ice::Current::operator==(const Current& __rhs) const
{
    return !operator!=(__rhs);
}

bool
Ice::Current::operator!=(const Current& __rhs) const
{
    if(this == &__rhs)
    {
	return false;
    }
    if(adapter != __rhs.adapter)
    {
	return true;
    }
    if(con != __rhs.con)
    {
	return true;
    }
    if(id != __rhs.id)
    {
	return true;
    }
    if(facet != __rhs.facet)
    {
	return true;
    }
    if(operation != __rhs.operation)
    {
	return true;
    }
    if(mode != __rhs.mode)
    {
	return true;
    }
    if(ctx != __rhs.ctx)
    {
	return true;
    }
    return false;
}

bool
Ice::Current::operator<(const Current& __rhs) const
{
    if(this == &__rhs)
    {
	return false;
    }
    if(adapter < __rhs.adapter)
    {
	return true;
    }
    else if(__rhs.adapter < adapter)
    {
	return false;
    }
    if(con < __rhs.con)
    {
	return true;
    }
    else if(__rhs.con < con)
    {
	return false;
    }
    if(id < __rhs.id)
    {
	return true;
    }
    else if(__rhs.id < id)
    {
	return false;
    }
    if(facet < __rhs.facet)
    {
	return true;
    }
    else if(__rhs.facet < facet)
    {
	return false;
    }
    if(operation < __rhs.operation)
    {
	return true;
    }
    else if(__rhs.operation < operation)
    {
	return false;
    }
    if(mode < __rhs.mode)
    {
	return true;
    }
    else if(__rhs.mode < mode)
    {
	return false;
    }
    if(ctx < __rhs.ctx)
    {
	return true;
    }
    else if(__rhs.ctx < ctx)
    {
	return false;
    }
    return false;
}
