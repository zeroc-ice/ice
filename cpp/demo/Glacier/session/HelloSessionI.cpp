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

#include <Ice/Ice.h>
#include <HelloSessionI.h>

using namespace std;

HelloSessionManagerI::HelloSessionManagerI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

Glacier::SessionPrx
HelloSessionManagerI::create(const string& userId, const Ice::Current&)
{
    Glacier::SessionPtr session = new HelloSessionI(userId, this);
    Ice::Identity ident;
    ident.category = userId;
    ident.name = "session";

    _adapter->add(session, ident);
    return Glacier::SessionPrx::uncheckedCast(_adapter->createProxy(ident));
}

void
HelloSessionManagerI::remove(const Ice::Identity& ident)
{
    _adapter->remove(ident);
}

HelloSessionI::HelloSessionI(const string& userId, const HelloSessionManagerIPtr& manager) :
    _userId(userId),
    _manager(manager)
{
}

void
HelloSessionI::destroy(const Ice::Current& current)
{
    _manager->remove(current.id);
}

void
HelloSessionI::hello(const Ice::Current&)
{
    cout << "Hello " << _userId << endl;
}
