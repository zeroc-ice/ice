// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

interface Thrower;

exception A
{
    int a;
    Thrower* thrower;
};

exception B extends A
{
    int b;
};

exception C extends B
{
    int c;
};

interface Thrower
{
    void shutdown();
    void throwAasA(int a) throws A;
    void throwBasA(int a, int b) throws A;
    void throwCasA(int a, int b, int c) throws A;
    void throwBasB(int a, int b) throws B;
    void throwCasB(int a, int b, int c) throws B;
    void throwCasC(int a, int b, int c) throws C;
};

#endif
