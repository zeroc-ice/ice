//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

// 'Test.ice' and 'TestAMD.ice' need to generate code into separate namespaces, but with identical type-ids. So we use
// the 'cs:namespace' metadata here to place the AMD code into a separate AMD namespace, instead of another module;
// Which would result in differing type-ids.
[cs:namespace:ZeroC.Ice.Test]
module Metrics
{

exception UserEx
{
}

sequence<byte> ByteSeq;

interface Metrics
{
    void op();

    idempotent void fail();

    void opWithUserException()
        throws UserEx;

    void opWithRequestFailedException();

    void opWithLocalException();

    void opWithUnknownException();

    void opByteS(ByteSeq bs);

    Object* getAdmin();

    void shutdown();
}

interface Controller
{
    void hold();

    void resume();
}

}
