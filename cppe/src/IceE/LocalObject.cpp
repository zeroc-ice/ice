// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LocalObject.h>

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
    return static_cast<Int>(reinterpret_cast<Long>(this) >> 4);
}
