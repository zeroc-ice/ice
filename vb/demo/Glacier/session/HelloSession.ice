// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_ICE
#define HELLO_ICE

#include <Glacier/Session.ice>

module Demo
{

interface HelloSession extends Glacier::Session
{
    void hello();
};

};

#endif
