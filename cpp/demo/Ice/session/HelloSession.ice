// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_SESSION_ICE
#define HELLO_SESSION_ICE

#include <Session.ice>

module Demo
{

interface HelloSession extends Session
{
    nonmutating void sayHello();
};

};

#endif
