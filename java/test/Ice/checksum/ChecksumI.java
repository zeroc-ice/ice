// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.checksum;

import test.Ice.checksum.server.Test.*;

public final class ChecksumI extends _ChecksumDisp
{
    public
    ChecksumI()
    {
    }

    public java.util.Map<String, String>
    getSliceChecksums(Ice.Current current)
    {
        return SliceChecksums.checksums;
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
