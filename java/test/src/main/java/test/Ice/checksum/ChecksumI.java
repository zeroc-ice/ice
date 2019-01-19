//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
