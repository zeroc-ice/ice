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

interface Test
{
    void shutdown();
    void abort();
    nonmutating void nonmutatingAbort();
    int pid();
};

#endif
