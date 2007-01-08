// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class ChecksumI extends Test._ChecksumDisp
{
    public
    ChecksumI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public java.util.Map
    getSliceChecksums(Ice.Current __current)
    {
        return SliceChecksums.checksums;
    }

    public void
    shutdown(Ice.Current __current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
}
