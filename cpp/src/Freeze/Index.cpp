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

#include <Freeze/Index.h>
#include <Freeze/IndexI.h>

using namespace Freeze;
using namespace Ice;
using namespace std;

Freeze::Index::~Index()
{
    delete _impl;
}

Freeze::Index::Index(const string& name) :
    _impl(new IndexI(*this, name))
{
}
  
vector<Identity>
Freeze::Index::untypedFindFirst(const Key& bytes, Int firstN) const
{
    return _impl->untypedFindFirst(bytes, firstN);
}   

vector<Identity>
Freeze::Index::untypedFind(const Key& bytes) const
{
    return _impl->untypedFind(bytes);
}
    
Int
Freeze::Index::untypedCount(const Key& bytes) const
{
    return _impl->untypedCount(bytes);
}
