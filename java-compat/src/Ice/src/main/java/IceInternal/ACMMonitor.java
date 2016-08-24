// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
