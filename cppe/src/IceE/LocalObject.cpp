// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LocalObject.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(LocalObject* p) { p->__incRef(); }
void IceEInternal::decRef(LocalObject* p) { p->__decRef(); }

bool
IceE::LocalObject::operator==(const LocalObject& r) const
{
    return this == &r;
}

bool
IceE::LocalObject::operator!=(const LocalObject& r) const
{
    return this != &r;
}

bool
IceE::LocalObject::operator<(const LocalObject& r) const
{
    return this < &r;
}

Int
IceE::LocalObject::ice_hash() const
{
    return static_cast<Int>(reinterpret_cast<Long>(this) >> 4);
}
