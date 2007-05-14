// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

interface ServerManager
{
    void startServer();
    void shutdown();
};

interface Hello
{
    idempotent void sayHello();
};

interface TestIntf
{
    void shutdown();

    idempotent Hello* getHello();

    idempotent Hello* getReplicatedHello();

    void migrateHello();
};

};

#endif
