// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
