//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>
#include <Ice/Exceptions.ice>

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Exceptions
{

interface Empty
{
}

interface Thrower;

exception A
{
    int aMem;
}

exception B : A
{
    int bMem;
}

exception C : B
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
    bool supportsAssertException();

    void throwAasA(int a);
    void throwAorDasAorD(int a);
    void throwBasA(int a, int b);
    void throwCasA(int a, int b, int c);
    void throwBasB(int a, int b);
    void throwCasB(int a, int b, int c);
    void throwCasC(int a, int b, int c);
    void throwLocalException();
    void throwNonIceException();
    void throwAssertException();
    Ice::ByteSeq sendAndReceive(Ice::ByteSeq seq);

    idempotent void throwLocalExceptionIdempotent();

    void throwAfterResponse();
    void throwAfterException();

    void throwAConvertedToUnhandled();
}

interface WrongOperation
{
    void noSuchOperation();
}

}
