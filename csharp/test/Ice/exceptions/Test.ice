//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

[["cs:typeid-namespace:Ice.exceptions.TypeId"]]

["cs:namespace:Ice.exceptions"]
module Test
{

interface Empty
{
}

interface Thrower;

exception A
{
    int aMem;
}

exception B extends A
{
    int bMem;
}

exception C extends B
{
    int cMem;
}

exception D
{
    int dMem;
}

interface Thrower
{
    void shutdown();
    bool supportsUndeclaredExceptions();
    bool supportsAssertException();

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
    void throwAssertException();
    Ice::ByteSeq throwMemoryLimitException(Ice::ByteSeq seq);

    idempotent void throwLocalExceptionIdempotent();

    void throwAfterResponse();
    void throwAfterException() throws A;
}

interface WrongOperation
{
    void noSuchOperation();
}

}
