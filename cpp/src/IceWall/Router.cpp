// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceWall/Router.h>

using namespace std;
using namespace Ice;
using namespace IceWall;

void
IceWall::Blobject::ice_invoke(const std::vector<Byte>& inParams, std::vector<Byte>& outParams, const Current& current)
{
    cout << current.identity << endl;
    cout << current.facet << endl;
    cout << current.operation << endl;
    throw UnknownLocalException(__FILE__, __LINE__);
}

IceWall::Router::Router() :
    _blobject(new IceWall::Blobject)
{
}

ObjectPtr
IceWall::Router::locate(const ObjectAdapterPtr&, const Current&, LocalObjectPtr&)
{
    return _blobject;
}

void
IceWall::Router::finished(const ObjectAdapterPtr&, const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
IceWall::Router::deactivate()
{
    _blobject = 0;
}
