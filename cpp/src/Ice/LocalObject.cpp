// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/LocalObject.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(LocalObject* p) { p->__incRef(); }
void IceInternal::decRef(LocalObject* p) { p->__decRef(); }

bool
Ice::LocalObject::operator==(const LocalObject& r) const
{
    return this == &r;
}

bool
Ice::LocalObject::operator!=(const LocalObject& r) const
{
    return this != &r;
}

bool
Ice::LocalObject::operator<(const LocalObject& r) const
{
    return this < &r;
}

Int
Ice::LocalObject::ice_hash() const
{
    return reinterpret_cast<Int>(this) >> 4;
}
