//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

// 'Test.ice' and 'TestAMD.ice' need to generate code into separate namespaces, but with identical type-ids. So we use
// the 'cs:namespace' metadata here to place the AMD code into a separate AMD namespace, instead of another module;
// Which would result in differing type-ids.
[cs:namespace:ZeroC.Ice.Test.AMD]
module Metrics
{

exception UserEx
{
}

sequence<byte> ByteSeq;

interface Metrics
{
    [amd] void op();

    [amd] idempotent void fail();

    [amd] void opWithUserException()
        throws UserEx;

    [amd] void opWithRequestFailedException();

    [amd] void opWithLocalException();

    [amd] void opWithUnknownException();

    [amd] void opByteS(ByteSeq bs);

    Object* getAdmin();

    void shutdown();
}

interface Controller
{
    void hold();

    void resume();
}

}
