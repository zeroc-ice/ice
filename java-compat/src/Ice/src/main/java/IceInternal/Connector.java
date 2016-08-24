// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface Connector
{
    Transceiver connect();

    short type();
    @Override
    String toString();

    //
    // Compare connectors for sorting process.
    //
    @Override
    boolean equals(java.lang.Object obj);
}
