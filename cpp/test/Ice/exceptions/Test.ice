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

class A
{
    int a;
};

class B extends A
{
    int b;
};

class C extends B
{
    int c;
};

class Thrower
{
    void shutdown();
    void throwAasA(int a) throws A;
    void throwAasAproxy() throws A*;
    void throwBasA(int a, int b) throws A;
    void throwBasAproxy() throws A*;
    void throwCasA(int a, int b, int c) throws A;
    void throwCasAproxy() throws A*;
    void throwBasB(int a, int b) throws B;
    void throwBasBproxy() throws B*;
    void throwCasB(int a, int b, int c) throws B;
    void throwCasBproxy() throws B*;
    void throwCasC(int a, int b, int c) throws C;
    void throwCasCproxy() throws C*;
};

#endif
