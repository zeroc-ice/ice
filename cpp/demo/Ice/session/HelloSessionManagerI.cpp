// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloSessionManagerI.h>
#include <HelloSessionI.h>

using namespace std;
using namespace Demo;

SessionPrx
HelloSessionManagerI::create(const Ice::Current& c)
{
    SessionPrx session = SessionPrx::uncheckedCast(c.adapter->addWithUUID(new HelloSessionI(this)));
    add(session);
    return session;
}
