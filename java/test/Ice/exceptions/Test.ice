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

interface Thrower;

exception A
{
    int a;
};

exception B extends A
{
    int b;
};

exception C extends B
{
    int c;
};

exception D
{
    int d;
};

interface Thrower
{
    void shutdown();
    void throwAasA(int a) throws A;
    void throwAorDasAorD(int a) throws A, D;
    void throwBasA(int a, int b) throws A;
    void throwCasA(int a, int b, int c) throws A;
    void throwBasB(int a, int b) throws B;
    void throwCasB(int a, int b, int c) throws B;
    void throwCasC(int a, int b, int c) throws C;
    bool supportsUndeclaredExceptions();
    void throwUndeclaredA(int a) throws B, D;
    void throwUndeclaredB(int a, int b);
    void throwUndeclaredC(int a, int b, int c) throws D;
    void throwLocalException();
    void throwNonIceException();
};

interface WrongOperation
{
    void noSuchOperation();
};

#endif
