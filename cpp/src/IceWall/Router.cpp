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

IceWall::Router::Router()
{
}

IceWall::Router::~Router()
{
}

ObjectPtr
IceWall::Router::locate(const ObjectAdapterPtr& adapter, const Current& current, LocalObjectPtr&)
{
    return 0;
}

void
IceWall::Router::finished(const ObjectAdapterPtr&, const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
IceWall::Router::deactivate()
{
}
