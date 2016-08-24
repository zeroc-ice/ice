// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.checksum;

import test.Ice.checksum.server.Test.*;

public final class ChecksumI implements Checksum
{
    public ChecksumI()
    {
    }

    @Override
    public java.util.Map<String, String> getSliceChecksums(com.zeroc.Ice.Current current)
    {
        return SliceChecksums.checksums;
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
