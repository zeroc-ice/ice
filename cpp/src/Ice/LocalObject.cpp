// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
