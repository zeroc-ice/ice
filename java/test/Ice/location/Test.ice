// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef TEST_ICE
#define TEST_ICE

interface ServerManager
{
    void startServer();
    void shutdown();
};

interface Hello
{
    void sayHello();
};

interface Test
{
    void shutdown();

    Hello* getHello();
};

#endif
