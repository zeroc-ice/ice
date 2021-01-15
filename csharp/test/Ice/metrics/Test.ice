//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Metrics
{

exception UserEx
{
}

sequence<byte> ByteSeq;

interface Metrics
{
    void op();

    idempotent void fail();

    void opWithUserException();

    void opWithRequestFailedException();

    void opWithLocalException();

    void opWithUnknownException();

    void opByteS(ByteSeq bs);

    Object? getAdmin();

    void shutdown();
}

interface Controller
{
    void hold();

    void resume();
}

}
