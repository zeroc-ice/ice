// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

