//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Retry
{

exception SystemFailure
{
}

sequence<byte> ByteSeq;

interface Retry
{
    void op(bool kill);

    idempotent int opIdempotent(int c);
    void opNotIdempotent();
    void opSystemException();
    int opAfterDelay(int retries, int delay);

    idempotent void sleep(int delay);

    void opWithData(int retries, int delay, ByteSeq data);

    idempotent void shutdown();
}

interface Replicated
{
    void otherReplica();
}

interface NonReplicated
{
    void otherReplica();
}

}
