// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_ICE
#define HELLO_ICE

module Demo
{

exception RequestCanceledException
{
};

interface Hello
{
    ["ami", "amd"] idempotent void sayHello(int delay)
        throws RequestCanceledException;

    void shutdown();
};

};

#endif
