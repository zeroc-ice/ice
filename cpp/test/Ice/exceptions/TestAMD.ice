// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef TEST_AMD_ICE
#define TEST_AMD_ICE

interface Thrower;

exception A
{
    int aMem;
};

exception B extends A
{
    int bMem;
};

exception C extends B
{
    int cMem;
};

exception D
{
    int dMem;
};

["ami", "amd"] interface Thrower
{
    void shutdown();
    bool supportsUndeclaredExceptions();
    bool supportsNonIceExceptions();

    void throwAasA(int a) throws A;
    void throwAorDasAorD(int a) throws A, D;
    void throwBasA(int a, int b) throws A;
    void throwCasA(int a, int b, int c) throws A;
    void throwBasB(int a, int b) throws B;
    void throwCasB(int a, int b, int c) throws B;
    void throwCasC(int a, int b, int c) throws C;
    void throwUndeclaredA(int a);
    void throwUndeclaredB(int a, int b);
    void throwUndeclaredC(int a, int b, int c);
    void throwLocalException();
    void throwNonIceException();
};

["ami", "amd"] interface WrongOperation
{
    void noSuchOperation();
};

#endif
