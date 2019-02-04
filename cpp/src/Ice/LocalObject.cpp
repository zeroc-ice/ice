//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/LocalObject.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* Ice::upCast(LocalObject* obj) { return obj; }

bool
Ice::LocalObject::operator==(const LocalObject& r) const
{
    return this == &r;
}

bool
Ice::LocalObject::operator<(const LocalObject& r) const
{
    return this < &r;
}
