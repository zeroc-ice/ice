// **********************************************************************
//
// Copyright (c) 2003
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

#include <Transform/TransformUtil.h>

using namespace std;

string
Transform::typeName(const Slice::TypePtr& type)
{
    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    if(b)
    {
        return b->kindAsString();
    }
    else
    {
        Slice::ContainedPtr c = Slice::ContainedPtr::dynamicCast(type);
        assert(c);
        return c->scoped();
    }
}
