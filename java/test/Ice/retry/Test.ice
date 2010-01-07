// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

[["java:package:test.Ice.retry"]]
module Test
{

interface Retry
{
    void op(bool kill);
    void shutdown();
};

};

#endif
