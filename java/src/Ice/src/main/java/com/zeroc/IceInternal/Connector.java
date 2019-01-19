//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

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
