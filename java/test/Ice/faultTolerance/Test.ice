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

interface Test
{
    void shutdown();
    void abort();
    idempotent void idempotentAbort();
    nonmutating void nonmutatingAbort();
    idempotent int pid();
};

#endif
