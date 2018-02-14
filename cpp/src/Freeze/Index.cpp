// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

Freeze::Index::Index(const string& name, const string& facet) :
    _name(name),
    _facet(facet),
    _impl(new IndexI(*this))
{
}

const string&
Freeze::Index::name() const
{
    return _name;
}

const string&
Freeze::Index::facet() const
{
    return _facet;
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
