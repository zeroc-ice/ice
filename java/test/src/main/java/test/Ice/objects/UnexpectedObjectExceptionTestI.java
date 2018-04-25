// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.AlsoEmpty;

public final class UnexpectedObjectExceptionTestI implements com.zeroc.Ice.Blobject
{
    @Override
    public com.zeroc.Ice.Object.Ice_invokeResult ice_invoke(byte[] inParams, com.zeroc.Ice.Current current)
    {
        com.zeroc.Ice.Object.Ice_invokeResult r = new com.zeroc.Ice.Object.Ice_invokeResult();
        com.zeroc.Ice.Communicator communicator = current.adapter.getCommunicator();
        com.zeroc.Ice.OutputStream out = new com.zeroc.Ice.OutputStream(communicator);
        out.startEncapsulation(current.encoding, com.zeroc.Ice.FormatType.DefaultFormat);
        out.writeValue(new AlsoEmpty());
        out.writePendingValues();
        out.endEncapsulation();
        r.outParams = out.finished();
        r.returnValue = true;
        return r;
    }
}
