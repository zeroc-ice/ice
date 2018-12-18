// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

public interface ACMMonitor
{
    void add(com.zeroc.Ice.ConnectionI con);
    void remove(com.zeroc.Ice.ConnectionI con);
    void reap(com.zeroc.Ice.ConnectionI con);

    ACMMonitor acm(java.util.OptionalInt timeout, java.util.Optional<com.zeroc.Ice.ACMClose> close,
                   java.util.Optional<com.zeroc.Ice.ACMHeartbeat> heartbeat);
    com.zeroc.Ice.ACM getACM();
}
