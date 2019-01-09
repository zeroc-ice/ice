// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public interface ACMMonitor
{
    void add(Ice.ConnectionI con);
    void remove(Ice.ConnectionI con);
    void reap(Ice.ConnectionI con);

    ACMMonitor acm(Ice.IntOptional timeout, Ice.Optional<Ice.ACMClose> close, Ice.Optional<Ice.ACMHeartbeat> heartbeat);
    Ice.ACM getACM();
}
