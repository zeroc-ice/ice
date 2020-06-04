//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

// 'Test.ice' and 'TestAMD.ice' need to generate code into separate namespaces, but with identical type-ids. So we use
// the 'cs:namespace' metadata here to place the AMD code into a separate AMD namespace, instead of another module;
// Which would result in differing type-ids.
[cs:namespace:ZeroC.Ice.Test.AMD]
module Exceptions
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

[amd] interface Thrower
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
    void throwUndeclaredA(int a);
    void throwUndeclaredB(int a, int b);
    void throwUndeclaredC(int a, int b, int c);
    void throwLocalException();
    void throwNonIceException();
    void throwAssertException();
    Ice::ByteSeq throwMemoryLimitException(Ice::ByteSeq seq);

    idempotent void throwLocalExceptionIdempotent();

    void throwAfterResponse();
    void throwAfterException();

    void throwAConvertedToUnhandled();
}

[amd] interface WrongOperation
{
    void noSuchOperation();
}

}
